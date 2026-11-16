#include "sfg_fastdds_bridge/detail/participant_handle.hpp"

#include <stdexcept>

#include <rcpputils/find_and_replace.hpp>
#include <rmw_fastrtps_shared_cpp/names.hpp>
#include <rmw_fastrtps_shared_cpp/namespace_prefix.hpp>
#include <rmw_fastrtps_shared_cpp/qos.hpp>
#include <rosidl_runtime_c/type_hash.h>

#include "sfg_fastdds_bridge/detail/generic_type_support.hpp"

namespace sfg_fastdds_bridge::detail
{
    using namespace eprosima::fastdds::dds;

    namespace
    {
        /// @brief Converts a fully qualified ROS topic name (e.g. "/cmd_vel") into its DDS topic name (e.g. "rt/cmd_vel").
        /// @param topic_name The fully qualified ROS topic name.
        /// @param qos The QoS profile. The prefix is omitted if it avoids the ROS namespace conventions.
        /// @return The DDS topic name.
        /// @throws std::invalid_argument If the topic name is not fully qualified.
        std::string to_dds_topic_name(const std::string &topic_name, const rclcpp::QoS &qos)
        {
            if (topic_name.size() < 2 || topic_name.front() != '/')
            {
                throw std::invalid_argument("Invalid ROS topic name '" + topic_name + "', expected a fully qualified name.");
            }

            const auto rmw_qos = qos.get_rmw_qos_profile();
            return _create_topic_name(&rmw_qos, ros_topic_prefix, topic_name.c_str()).to_string();
        }

        /// @brief Converts a ROS topic type (e.g. "sensor_msgs/msg/Imu") into its DDS topic type (e.g. "sensor_msgs::msg::dds_::Imu_").
        /// @param topic_type The ROS topic type.
        /// @return The DDS topic type.
        /// @throws std::invalid_argument If the topic type is malformed.
        std::string to_dds_topic_type(const std::string &topic_type)
        {
            const auto separator = topic_type.rfind('/');

            if (separator == std::string::npos || separator == 0 || separator + 1 == topic_type.size())
            {
                throw std::invalid_argument("Invalid ROS topic type '" + topic_type + "'.");
            }

            const auto type_namespace = rcpputils::find_and_replace(topic_type.substr(0, separator), "/", "::");
            return type_namespace + "::dds_::" + topic_type.substr(separator + 1) + "_";
        }

        /// @brief Converts a ROS QoS profile into Fast DDS data writer QoS.
        /// @param qos The QoS profile.
        /// @return The data writer QoS.
        /// @throws std::runtime_error If the profile cannot be converted.
        DataWriterQos to_writer_qos(const rclcpp::QoS &qos)
        {
            auto writer_qos = DATAWRITER_QOS_DEFAULT;

            // rmw_fastrtps announces the ROS type hash in the endpoint's USER_DATA ("typehash=...;"). The generic layer only
            // knows the type as a string, so the hash would have to be obtained by dynamically loading the type support
            // library. We pass an unset (zero) hash instead, which leaves USER_DATA empty. ROS 2 treats a missing hash as
            // unknown and still matches the endpoints, and the Create 3 firmware does not use type hashes.
            if (!get_datawriter_qos(qos.get_rmw_qos_profile(), rosidl_get_zero_initialized_type_hash(), writer_qos))
            {
                throw std::runtime_error("Failed to convert ROS QoS to Fast DDS data writer QoS.");
            }
            // Required for variable-size messages.
            writer_qos.endpoint().history_memory_policy = eprosima::fastdds::rtps::PREALLOCATED_WITH_REALLOC_MEMORY_MODE;
            return writer_qos;
        }

        /// @brief Converts a ROS QoS profile into Fast DDS data reader QoS.
        /// @param qos The QoS profile.
        /// @return The data reader QoS.
        /// @throws std::runtime_error If the profile cannot be converted.
        DataReaderQos to_reader_qos(const rclcpp::QoS &qos)
        {
            auto reader_qos = DATAREADER_QOS_DEFAULT;

            if (!get_datareader_qos(qos.get_rmw_qos_profile(), rosidl_get_zero_initialized_type_hash(), reader_qos))
            {
                throw std::runtime_error("Failed to convert ROS QoS to Fast DDS data reader QoS.");
            }
            // Required for variable-size messages.
            reader_qos.endpoint().history_memory_policy = eprosima::fastdds::rtps::PREALLOCATED_WITH_REALLOC_MEMORY_MODE;
            return reader_qos;
        }
    }

    ParticipantHandle::ParticipantHandle(std::uint32_t domain_id) : m_factory(DomainParticipantFactory::get_shared_instance())
    {
        // PARTICIPANT_QOS_DEFAULT picks up the default profile of FASTDDS_DEFAULT_PROFILES_FILE, if set.
        m_participant = m_factory->create_participant(domain_id, PARTICIPANT_QOS_DEFAULT);

        if (!m_participant)
        {
            throw std::runtime_error("Failed to create Fast DDS participant for domain " + std::to_string(domain_id) + ".");
        }
        m_publisher = m_participant->create_publisher(PUBLISHER_QOS_DEFAULT);
        m_subscriber = m_participant->create_subscriber(SUBSCRIBER_QOS_DEFAULT);

        if (!m_publisher || !m_subscriber)
        {
            m_participant->delete_contained_entities();
            m_factory->delete_participant(m_participant);
            throw std::runtime_error("Failed to create Fast DDS publisher/subscriber containers for domain " + std::to_string(domain_id) + ".");
        }
    }

    ParticipantHandle::~ParticipantHandle()
    {
        // All writers, readers and topics hold a reference to this handle, so only the containers are left.
        m_participant->delete_subscriber(m_subscriber);
        m_participant->delete_publisher(m_publisher);
        m_factory->delete_participant(m_participant);
    }

    std::shared_ptr<Topic> ParticipantHandle::acquire_topic(
        const std::string &topic_name,
        const std::string &topic_type,
        const rclcpp::QoS &qos)
    {
        const auto dds_topic_name = to_dds_topic_name(topic_name, qos);
        const auto dds_topic_type = to_dds_topic_type(topic_type);
        std::lock_guard lock(m_mutex);

        if (auto topic = m_weak_topics[dds_topic_name].lock())
        {
            if (topic->get_type_name() != dds_topic_type)
            {
                throw std::invalid_argument("Topic '" + dds_topic_name + "' already exists with type '" + topic->get_type_name() + "', requested type '" + dds_topic_type + "'.");
            }
            return topic;
        }

        // The last reference may have just been released while its deleter still waits for the mutex. In that case the
        // DDS topic still exists and is reused; the pending deleter then sees the live cache entry and skips the deletion.
        auto raw_topic = dynamic_cast<Topic *>(m_participant->lookup_topicdescription(dds_topic_name));

        if (raw_topic && raw_topic->get_type_name() != dds_topic_type)
        {
            throw std::invalid_argument("Topic '" + dds_topic_name + "' already exists with type '" + raw_topic->get_type_name() + "', requested type '" + dds_topic_type + "'.");
        }

        if (!raw_topic)
        {
            if (m_participant->find_type(dds_topic_type).empty())
            {
                TypeSupport type_support(new GenericTypeSupport(dds_topic_type));

                if (type_support.register_type(m_participant) != RETCODE_OK)
                {
                    throw std::runtime_error("Failed to register type '" + dds_topic_type + "'.");
                }
            }
            raw_topic = m_participant->create_topic(dds_topic_name, dds_topic_type, TOPIC_QOS_DEFAULT);

            if (!raw_topic)
            {
                throw std::runtime_error("Failed to create topic '" + dds_topic_name + "'.");
            }
        }

        std::shared_ptr<Topic> topic(raw_topic, [self = shared_from_this(), dds_topic_name](Topic *released_topic)
                                     { self->release_topic(dds_topic_name, released_topic); });
        m_weak_topics[dds_topic_name] = topic;
        return topic;
    }

    void ParticipantHandle::release_topic(const std::string &dds_topic_name, Topic *topic)
    {
        std::lock_guard lock(m_mutex);
        auto weak_topic = m_weak_topics.find(dds_topic_name);

        if (weak_topic == m_weak_topics.end() || !weak_topic->second.expired())
        {
            // Acquired again, or already deleted by another pending deleter.
            return;
        }
        m_weak_topics.erase(weak_topic);
        m_participant->delete_topic(topic);
    }

    DataWriterHandle ParticipantHandle::create_writer(Topic *topic, const rclcpp::QoS &qos)
    {
        auto writer = m_publisher->create_datawriter(topic, to_writer_qos(qos));

        if (!writer)
        {
            throw std::runtime_error("Failed to create data writer for topic '" + topic->get_name() + "'.");
        }
        return DataWriterHandle(
            writer,
            [self = shared_from_this()](DataWriter *released_writer)
            {
                self->m_publisher->delete_datawriter(released_writer);
            });
    }

    DataReaderHandle ParticipantHandle::create_reader(Topic *topic, const rclcpp::QoS &qos, DataReaderListener *listener)
    {
        auto reader = m_subscriber->create_datareader(topic, to_reader_qos(qos), listener, StatusMask::data_available());

        if (!reader)
        {
            throw std::runtime_error("Failed to create data reader for topic '" + topic->get_name() + "'.");
        }
        return DataReaderHandle(
            reader,
            [self = shared_from_this()](DataReader *released_reader)
            {
                self->m_subscriber->delete_datareader(released_reader);
            });
    }
}

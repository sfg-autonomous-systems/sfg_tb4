#pragma once

#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/DataReaderListener.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/topic/Topic.hpp>
#include <rclcpp/qos.hpp>

#include "sfg_fastdds_bridge/detail/data_reader_handle.hpp"
#include "sfg_fastdds_bridge/detail/data_writer_handle.hpp"

namespace sfg_fastdds_bridge::detail
{
    /// @brief Owns the DomainParticipant of one domain, one DDS Publisher and one DDS Subscriber container, and the topics
    /// created in that participant.
    ///
    /// Obtain instances through ParticipantRegistry::instance().get(). The deleters of all returned entities keep this handle alive,
    /// so the participant is only deleted after all its entities.
    class ParticipantHandle : public std::enable_shared_from_this<ParticipantHandle>
    {
    public:
        /// @param domain_id The DDS domain ID.
        /// @throws std::runtime_error If the participant or its containers cannot be created.
        explicit ParticipantHandle(uint32_t domain_id);
        ParticipantHandle(const ParticipantHandle &) = delete;
        ParticipantHandle(ParticipantHandle &&) = delete;
        ParticipantHandle &operator=(const ParticipantHandle &) = delete;
        ParticipantHandle &operator=(ParticipantHandle &&) = delete;
        ~ParticipantHandle();

        /// @brief Returns the topic for the given ROS topic, creating it (and registering its type) if it does not exist
        /// yet. The topic is deleted once the last returned shared_ptr is released.
        /// @param topic_name The fully qualified ROS topic name.
        /// @param topic_type The ROS topic type.
        /// @param qos The QoS profile, which determines whether the ROS topic prefix is applied.
        /// @return The shared topic.
        /// @throws std::invalid_argument If the topic name or type is malformed, or the topic already exists with a different type.
        /// @throws std::runtime_error If the type cannot be registered or the topic cannot be created.
        std::shared_ptr<eprosima::fastdds::dds::Topic> acquire_topic(
            const std::string &topic_name,
            const std::string &topic_type,
            const rclcpp::QoS &qos);

        /// @brief Creates a data writer in the internal DDS Publisher container.
        /// @param topic The topic to write to.
        /// @param qos The QoS profile of the writer.
        /// @return The writer, which is deleted again when released.
        /// @throws std::runtime_error If the QoS cannot be converted or the writer cannot be created.
        DataWriterHandle create_writer(eprosima::fastdds::dds::Topic *topic, const rclcpp::QoS &qos);

        /// @brief Creates a data reader in the internal DDS Subscriber container.
        /// @param topic The topic to read from.
        /// @param qos The QoS profile of the reader.
        /// @param listener The listener notified about incoming data, which must outlive the returned reader.
        /// @return The reader, which is deleted again when released.
        /// @throws std::runtime_error If the QoS cannot be converted or the reader cannot be created.
        DataReaderHandle create_reader(
            eprosima::fastdds::dds::Topic *topic,
            const rclcpp::QoS &qos,
            eprosima::fastdds::dds::DataReaderListener *listener);

    private:
        void release_topic(const std::string &dds_topic_name, eprosima::fastdds::dds::Topic *topic);

        // We keep a reference to the factory to ensure it stays alive as long as this instance does.
        // This matters if the last reference to this instance is released during static teardown,
        // e.g. by a publisher with static storage duration. Since the destruction order depends on
        // construction order across libraries, the factory may be destroyed before this instance.
        // Calling DomainParticipantFactory::get_instance() or DomainParticipantFactory::get_shared_instance()
        // in the destructor would not solve this either (see https://eel.is/c++draft/basic.start.term#5).
        std::shared_ptr<eprosima::fastdds::dds::DomainParticipantFactory> m_factory;
        eprosima::fastdds::dds::DomainParticipant *m_participant{nullptr};
        eprosima::fastdds::dds::Publisher *m_publisher{nullptr};
        eprosima::fastdds::dds::Subscriber *m_subscriber{nullptr};

        std::mutex m_mutex;
        // Topics keyed by their DDS topic name.
        std::unordered_map<std::string, std::weak_ptr<eprosima::fastdds::dds::Topic>> m_weak_topics;
    };
}

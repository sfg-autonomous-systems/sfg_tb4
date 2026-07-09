#include "sfg_fastdds_utils/fastdds_publisher.hpp"

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <rmw_fastrtps_cpp/MessageTypeSupport.hpp>
#include <rmw_fastrtps_shared_cpp/qos.hpp>

namespace sfg_fastdds_utils
{
    FastDDSPublisher::FastDDSPublisher(
        const std::string &topic_fqn,
        const rosidl_message_type_support_t *type_support_handle,
        const rclcpp::QoS &qos,
        uint32_t domain_id)
        : m_type_support_handle(type_support_handle)
    {
        using namespace eprosima::fastdds::dds;

        // Setup type support.
        auto callbacks = static_cast<const message_type_support_callbacks_t *>(m_type_support_handle->data);
        m_type_support.reset(new rmw_fastrtps_cpp::MessageTypeSupport(callbacks, m_type_support_handle));

        // Create domain participant.
        // ToDo: Share the participant across publishers and subscribers, instead of creating a new one for each publisher.
        auto factory = DomainParticipantFactory::get_instance();
        auto participant_qos = PARTICIPANT_QOS_DEFAULT;
        m_participant = factory->create_participant(domain_id, participant_qos);

        if (!m_participant)
        {
            throw std::runtime_error("Failed to create FastDDS Participant");
        }

        // Register type and create publisher and topic.
        m_type_support.register_type(m_participant);
        m_publisher = m_participant->create_publisher(PUBLISHER_QOS_DEFAULT);
        m_topic = m_participant->create_topic("rt" + topic_fqn, m_type_support.get_type_name(), TOPIC_QOS_DEFAULT);

        // Create data writer.
        if (m_type_support_handle->get_type_hash_func == nullptr)
        {
            throw std::runtime_error("Type support handle does not provide a type hash function.");
        }
        auto type_hash = m_type_support_handle->get_type_hash_func(m_type_support_handle);

        if (type_hash == nullptr)
        {
            throw std::runtime_error("Failed to retrieve type hash from the type support handle.");
        }
        auto writer_qos = DATAWRITER_QOS_DEFAULT;
        auto rmw_qos = qos.get_rmw_qos_profile();

        if (!get_datawriter_qos(rmw_qos, *type_hash, writer_qos))
        {
            throw std::runtime_error("Failed to convert ROS QoS to FastDDS QoS");
        }
        m_writer = m_publisher->create_datawriter(m_topic, writer_qos);
    }

    FastDDSPublisher::~FastDDSPublisher()
    {
        if (m_publisher && m_writer)
        {
            m_publisher->delete_datawriter(m_writer);
        }

        if (m_participant && m_topic)
        {
            m_participant->delete_topic(m_topic);
        }

        if (m_participant && m_publisher)
        {
            m_participant->delete_publisher(m_publisher);
        }

        if (m_participant)
        {
            eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->delete_participant(m_participant);
        }
    }

    bool FastDDSPublisher::publish(const void *msg)
    {
        auto callbacks = static_cast<const message_type_support_callbacks_t *>(m_type_support_handle->data);
        rmw_fastrtps_shared_cpp::SerializedData serialized_data;
        serialized_data.type = rmw_fastrtps_shared_cpp::FASTDDS_SERIALIZED_DATA_TYPE_ROS_MESSAGE;
        serialized_data.data = const_cast<void *>(msg);
        serialized_data.impl = callbacks;
        return m_writer->write(&serialized_data);
    }
}
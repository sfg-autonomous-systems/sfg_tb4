#include "sfg_fastdds_bridge/generic_publisher.hpp"

#include "sfg_fastdds_bridge/detail/participant_registry.hpp"

namespace sfg_fastdds_bridge
{
    GenericPublisher::GenericPublisher(
        const std::string &topic_name,
        const std::string &topic_type,
        const rclcpp::QoS &qos,
        uint32_t domain_id)
        : m_participant(detail::ParticipantRegistry::instance().get(domain_id)),
          m_topic(m_participant->acquire_topic(topic_name, topic_type, qos)),
          m_writer(m_participant->create_writer(m_topic.get(), qos))
    {
    }

    bool GenericPublisher::publish(const rclcpp::SerializedMessage &message)
    {
        return m_writer->write(&message) == eprosima::fastdds::dds::RETCODE_OK;
    }
}

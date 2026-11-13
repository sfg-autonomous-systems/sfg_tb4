#pragma once

#include <rosidl_typesupport_cpp/message_type_support.hpp>

#include "sfg_fastdds_bridge/fastdds_publisher.hpp"

namespace sfg_fastdds_bridge
{
    template <typename MessageType>
    FastDDSPublisher<MessageType>::FastDDSPublisher(
        const std::string &topic_fqn,
        const rclcpp::QoS &qos,
        uint32_t domain_id)
        : m_generic_publisher(topic_fqn, rosidl_typesupport_cpp::get_message_type_support_handle<MessageType>(), qos, domain_id)
    {
    }

    template <typename MessageType>
    bool FastDDSPublisher<MessageType>::publish(const MessageType &msg)
    {
        return m_generic_publisher.publish(&msg);
    }
}

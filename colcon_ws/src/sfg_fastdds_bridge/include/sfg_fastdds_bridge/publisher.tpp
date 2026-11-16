#pragma once

#include <rosidl_runtime_cpp/traits.hpp>

#include "sfg_fastdds_bridge/publisher.hpp"

namespace sfg_fastdds_bridge
{
    template <typename MessageType>
    Publisher<MessageType>::Publisher(const std::string &topic_name, const rclcpp::QoS &qos, std::optional<std::uint32_t> domain_id)
        : m_generic_publisher(topic_name, rosidl_generator_traits::name<MessageType>(), qos, domain_id)
    {
    }

    template <typename MessageType>
    bool Publisher<MessageType>::publish(const MessageType &msg)
    {
        rclcpp::SerializedMessage serialized_msg;
        m_serialization.serialize_message(&msg, &serialized_msg);
        return m_generic_publisher.publish(serialized_msg);
    }
}

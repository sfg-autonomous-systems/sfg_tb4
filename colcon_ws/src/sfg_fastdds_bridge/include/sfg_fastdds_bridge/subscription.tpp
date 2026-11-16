#pragma once

#include <functional>
#include <stdexcept>

#include <rosidl_runtime_cpp/traits.hpp>

#include "sfg_fastdds_bridge/subscription.hpp"

namespace sfg_fastdds_bridge
{
    template <typename MessageType>
    Subscription<MessageType>::Subscription(
        const std::string &topic_name,
        const rclcpp::QoS &qos,
        Callback callback,
        std::optional<std::uint32_t> domain_id)
        : m_callback(callback ? std::move(callback) : throw std::invalid_argument("Subscription callback must not be empty.")),
          m_generic_subscription(
              topic_name,
              rosidl_generator_traits::name<MessageType>(),
              qos,
              std::bind(&Subscription::on_message, this, std::placeholders::_1),
              domain_id)
    {
    }

    template <typename MessageType>
    void Subscription<MessageType>::on_message(std::shared_ptr<const rclcpp::SerializedMessage> serialized_msg)
    {
        auto msg = std::make_shared<MessageType>();
        m_serialization.deserialize_message(serialized_msg.get(), msg.get());
        m_callback(std::move(msg));
    }
}

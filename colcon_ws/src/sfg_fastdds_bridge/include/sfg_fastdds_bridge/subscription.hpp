#pragma once

#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <string>

#include <rclcpp/qos.hpp>
#include <rclcpp/serialization.hpp>

#include "sfg_fastdds_bridge/generic_subscription.hpp"

namespace sfg_fastdds_bridge
{
    /// @brief Typed Fast DDS subscription, independent of the active RMW. Receives through a GenericSubscription and
    /// deserializes with rclcpp::Serialization<MessageType>.
    ///
    /// The callback is invoked directly from the Fast DDS listener thread, not from an rclcpp executor. Messages that fail
    /// to deserialize and exceptions thrown by the callback are logged and dropped.
    template <typename MessageType>
    class Subscription
    {
    public:
        using Callback = std::function<void(std::shared_ptr<const MessageType>)>;

        /// @param topic_name The fully qualified ROS topic name.
        /// @param qos The QoS profile.
        /// @param callback The callback called with each received message.
        /// @param domain_id The DDS domain ID. Defaults to the value of ROS_DOMAIN_ID, or 0 if that is unset or empty.
        /// @throws See GenericSubscription.
        Subscription(
            const std::string &topic_name,
            const rclcpp::QoS &qos,
            Callback callback,
            std::optional<std::uint32_t> domain_id = std::nullopt);
        Subscription(const Subscription &) = delete;
        Subscription(Subscription &&) = delete;
        Subscription &operator=(const Subscription &) = delete;
        Subscription &operator=(Subscription &&) = delete;

    private:
        /// @brief Deserializes a received message and passes it to the user callback.
        /// @param serialized_msg The received message, including the encapsulation header.
        /// @throws rclcpp::exceptions::RCLError If the message cannot be deserialized.
        void on_message(std::shared_ptr<const rclcpp::SerializedMessage> serialized_msg);

        Callback m_callback;
        rclcpp::Serialization<MessageType> m_serialization;
        // Declared last, so it is destroyed first and no message arrives while the members above are destroyed.
        GenericSubscription m_generic_subscription;
    };
}

#include "sfg_fastdds_bridge/subscription.tpp"

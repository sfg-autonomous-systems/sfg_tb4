#pragma once

#include <cstdint>
#include <optional>
#include <string>

#include <rclcpp/qos.hpp>
#include <rclcpp/serialization.hpp>

#include "sfg_fastdds_bridge/generic_publisher.hpp"

namespace sfg_fastdds_bridge
{
    /// @brief Typed Fast DDS publisher, independent of the active RMW. Serializes with rclcpp::Serialization<MessageType>
    /// and writes through a GenericPublisher.
    template <typename MessageType>
    class Publisher
    {
    public:
        /// @param topic_name The fully qualified ROS topic name.
        /// @param qos The QoS profile.
        /// @param domain_id The DDS domain ID. Defaults to the value of ROS_DOMAIN_ID, or 0 if that is unset or empty.
        /// @throws See GenericPublisher.
        explicit Publisher(
            const std::string &topic_name,
            const rclcpp::QoS &qos = rclcpp::SystemDefaultsQoS(),
            std::optional<std::uint32_t> domain_id = std::nullopt);

        /// @brief Serializes and writes a message.
        /// @param msg The message to publish.
        /// @return Whether the message was written.
        /// @throws rclcpp::exceptions::RCLError If the message cannot be serialized.
        bool publish(const MessageType &msg);

    private:
        rclcpp::Serialization<MessageType> m_serialization;
        GenericPublisher m_generic_publisher;
    };
}

#include "sfg_fastdds_bridge/publisher.tpp"

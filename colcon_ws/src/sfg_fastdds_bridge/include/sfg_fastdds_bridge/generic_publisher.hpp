#pragma once

#include <cstdint>
#include <memory>
#include <string>

#include <rclcpp/qos.hpp>
#include <rclcpp/serialized_message.hpp>

#include "sfg_fastdds_bridge/detail/data_writer_handle.hpp"
#include "sfg_fastdds_bridge/detail/participant_handle.hpp"

namespace sfg_fastdds_bridge
{
    /// @brief Fast DDS publisher that writes already serialized ROS messages, independent of the active RMW.
    ///
    /// There is no type checking: publishing bytes that do not match the given type only shows up as garbage on the
    /// receiving side.
    class GenericPublisher
    {
    public:
        /// @param topic_name The fully qualified ROS topic name.
        /// @param topic_type The ROS topic type.
        /// @param qos The QoS profile.
        /// @param domain_id The DDS domain ID.
        /// @throws std::invalid_argument If the topic name or type is invalid, or the topic already exists with another type.
        /// @throws std::runtime_error If a DDS entity cannot be created.
        GenericPublisher(
            const std::string &topic_name,
            const std::string &topic_type,
            const rclcpp::QoS &qos = rclcpp::SystemDefaultsQoS(),
            uint32_t domain_id = 0);

        /// @brief Writes a serialized message.
        /// @param message The serialized message, including the encapsulation header, as produced by rclcpp::Serialization.
        /// @return Whether the message was written.
        bool publish(const rclcpp::SerializedMessage &message);

    private:
        // Declaration order matters for correct destruction.
        std::shared_ptr<detail::ParticipantHandle> m_participant;
        std::shared_ptr<eprosima::fastdds::dds::Topic> m_topic;
        detail::DataWriterHandle m_writer;
    };
}

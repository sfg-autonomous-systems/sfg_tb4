#pragma once

#include <string>

#include <rclcpp/qos.hpp>

#include "sfg_fastdds_utils/generic_fastdds_publisher.hpp"

namespace sfg_fastdds_utils
{
    /// Typed FastDDS publisher for ROS 2 messages, e.g. FastDDSPublisher<geometry_msgs::msg::Twist>.
    template <typename MessageType>
    class FastDDSPublisher
    {
    public:
        explicit FastDDSPublisher(
            const std::string &topic_fqn,
            const rclcpp::QoS &qos = rclcpp::SystemDefaultsQoS(),
            uint32_t domain_id = 0);

        bool publish(const MessageType &msg);

    private:
        GenericFastDDSPublisher m_generic_publisher;
    };
}

#include "sfg_fastdds_utils/fastdds_publisher.tpp"

#pragma once

#include <geometry_msgs/msg/transform_stamped.hpp>
#include <nav_msgs/msg/odometry.hpp>
#include <rclcpp/rclcpp.hpp>
#include <tf2_ros/buffer.h>
#include <tf2_ros/transform_broadcaster.hpp>

#include "sfg_fastdds_bridge/publisher.hpp"
#include "sfg_fastdds_bridge/subscription.hpp"
#include "sfg_hardware_interface/locomotion_controller_base.hpp"

namespace sfg_tb4_hardware_interface
{
    class LocomotionController : public sfg_hardware_interface::LocomotionControllerBase
    {
    public:
        LocomotionController(const rclcpp::NodeOptions &options);

    protected:
        void apply_cmd(const geometry_msgs::msg::TwistStamped &cmd) override;

    private:
        void odometry_callback(std::shared_ptr<const nav_msgs::msg::Odometry> msg);

        std::unique_ptr<sfg_fastdds_bridge::Subscription<nav_msgs::msg::Odometry>> m_odom_subscriber;
        std::unique_ptr<sfg_fastdds_bridge::Publisher<geometry_msgs::msg::Twist>> m_cmd_vel_publisher;
    };
}
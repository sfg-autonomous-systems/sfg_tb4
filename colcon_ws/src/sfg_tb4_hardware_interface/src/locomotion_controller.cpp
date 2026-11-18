#include "sfg_tb4_hardware_interface/locomotion_controller.hpp"

#include <functional>

#include "sfg_utils/fqn/ros_fqn_builder.hpp"

namespace sfg_tb4_hardware_interface
{
    LocomotionController::LocomotionController(const rclcpp::NodeOptions &options) : LocomotionControllerBase("locomotion_controller", options)
    {
        using namespace sfg_utils::fqn;

        m_cmd_vel_publisher = std::make_unique<sfg_fastdds_bridge::Publisher<geometry_msgs::msg::Twist>>(
            get_node_base_interface()->resolve_topic_or_service_name(RosFqnBuilder().component(Component::Create3).resource(Resource::CmdVel).build(RosFqnSegment::Component, RosFqnSegment::Resource), false),
            rclcpp::SensorDataQoS());

        m_odom_subscriber = std::make_unique<sfg_fastdds_bridge::Subscription<nav_msgs::msg::Odometry>>(
            get_node_base_interface()->resolve_topic_or_service_name(RosFqnBuilder().component(Component::Create3).resource(Resource::Custom, "odom").build(RosFqnSegment::Component, RosFqnSegment::Resource), false),
            rclcpp::SensorDataQoS(),
            std::bind(&LocomotionController::odometry_callback, this, std::placeholders::_1));

        RCLCPP_INFO(get_logger(), "Started locomotion controller.");
    }

    void LocomotionController::apply_cmd(const geometry_msgs::msg::TwistStamped &cmd)
    {
        geometry_msgs::msg::Twist msg;
        msg.linear = cmd.twist.linear;
        msg.angular = cmd.twist.angular;
        m_cmd_vel_publisher->publish(msg);
    }

    void LocomotionController::odometry_callback(std::shared_ptr<const nav_msgs::msg::Odometry> msg)
    {
        try_publish_odometry(now(), msg->pose, msg->twist);
    }
}
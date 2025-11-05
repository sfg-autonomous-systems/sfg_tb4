#include "sfg_tb4_hardware_interface/locomotion_controller.hpp"

#include "sfg_utils/fqn/ros_fqn_builder.hpp"

namespace sfg_tb4_hardware_interface
{
    LocomotionController::LocomotionController(const rclcpp::NodeOptions &options) : LocomotionControllerBase("locomotion_controller", options)
    {
        using namespace sfg_utils::fqn;

        // Set up interfaces.
        m_cmd_vel_publisher = create_publisher<geometry_msgs::msg::Twist>(
            RosFqnBuilder().component(Component::Create3).resource(Resource::CmdVel).build(RosFqnSegment::Component, RosFqnSegment::Resource),
            10);

        RCLCPP_INFO(get_logger(), "Started locomotion controller.");
    }

    void LocomotionController::apply_cmd(const geometry_msgs::msg::TwistStamped &cmd)
    {
        geometry_msgs::msg::Twist msg;
        msg.linear = cmd.twist.linear;
        msg.angular = cmd.twist.angular;
        m_cmd_vel_publisher->publish(msg);
    }
}
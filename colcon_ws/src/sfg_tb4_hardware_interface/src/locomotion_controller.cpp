#include "sfg_tb4_hardware_interface/locomotion_controller.hpp"

#include <geometry_msgs/msg/detail/twist__rosidl_typesupport_fastrtps_cpp.hpp>

#include "sfg_utils/fqn/ros_fqn_builder.hpp"

namespace sfg_tb4_hardware_interface
{
    LocomotionController::LocomotionController(const rclcpp::NodeOptions &options) : LocomotionControllerBase("locomotion_controller", options)
    {
        using namespace sfg_utils::fqn;

        auto ros_domain_id = std::getenv("ROS_DOMAIN_ID");
        m_cmd_vel_publisher = std::make_unique<sfg_fastdds_utils::FastDDSPublisher>(
            get_node_base_interface()->resolve_topic_or_service_name(RosFqnBuilder().component(Component::Create3).resource(Resource::CmdVel).build(RosFqnSegment::Component, RosFqnSegment::Resource), false),
            rosidl_typesupport_fastrtps_cpp__get_message_type_support_handle__geometry_msgs__msg__Twist(),
            rclcpp::SensorDataQoS(),
            ros_domain_id ? std::stoul(ros_domain_id) : 0);

        RCLCPP_INFO(get_logger(), "Started locomotion controller.");
    }

    void LocomotionController::apply_cmd(const geometry_msgs::msg::TwistStamped &cmd)
    {
        geometry_msgs::msg::Twist msg;
        msg.linear = cmd.twist.linear;
        msg.angular = cmd.twist.angular;
        m_cmd_vel_publisher->publish(&msg);
    }
}
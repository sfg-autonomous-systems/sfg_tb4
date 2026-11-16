#pragma once

#include <geometry_msgs/msg/twist.hpp>

#include "sfg_fastdds_bridge/publisher.hpp"
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
        std::unique_ptr<sfg_fastdds_bridge::Publisher<geometry_msgs::msg::Twist>> m_cmd_vel_publisher;
    };
}
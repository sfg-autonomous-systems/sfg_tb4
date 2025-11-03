#include "sfg_tb4_hardware_interface/locomotion_controller.hpp"

namespace sfg_tb4_hardware_interface
{
    LocomotionController::LocomotionController(const rclcpp::NodeOptions &options) : Node("locomotion_controller", options)
    {
        RCLCPP_INFO(get_logger(), "Started locomotion controller.");
    }
}
#pragma once

#include "rclcpp/rclcpp.hpp"

namespace sfg_tb4_hardware_interface
{
    class LocomotionController : public rclcpp::Node
    {
    public:
        LocomotionController(const rclcpp::NodeOptions &options);
    };
}
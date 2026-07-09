#pragma once

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>

#include "sfg_hardware_interface/locomotion_controller_base.hpp"

namespace sfg_tb4_hardware_interface
{
    class LocomotionController : public sfg_hardware_interface::LocomotionControllerBase
    {
    public:
        LocomotionController(const rclcpp::NodeOptions &options);
        LocomotionController(const LocomotionController &) = delete;
        LocomotionController &operator=(const LocomotionController &) = delete;
        LocomotionController(LocomotionController &&) = delete;
        LocomotionController &operator=(LocomotionController &&) = delete;
        ~LocomotionController() override;

    protected:
        void apply_cmd(const geometry_msgs::msg::TwistStamped &cmd) override;

    private:
        eprosima::fastdds::dds::DomainParticipant *m_participant = nullptr;
        eprosima::fastdds::dds::Publisher *m_cmd_vel_publisher = nullptr;
        eprosima::fastdds::dds::Topic *m_cmd_vel_topic = nullptr;
        eprosima::fastdds::dds::TypeSupport m_cmd_vel_type;
        eprosima::fastdds::dds::DataWriter *m_cmd_vel_writer = nullptr;
    };
}
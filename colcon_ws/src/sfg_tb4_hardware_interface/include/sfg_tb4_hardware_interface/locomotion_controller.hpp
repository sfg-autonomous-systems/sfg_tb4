#pragma once

#include <fastcdr/Cdr.h>
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

    protected:
        void apply_cmd(const geometry_msgs::msg::TwistStamped &cmd) override;

    private:
        class CmdVelType : public eprosima::fastdds::dds::TopicDataType
        {
        public:
            CmdVelType();

            bool serialize(
                const void *const data,
                eprosima::fastdds::rtps::SerializedPayload_t &payload,
                eprosima::fastdds::dds::DataRepresentationId_t data_representation) override;

            bool deserialize(
                eprosima::fastdds::rtps::SerializedPayload_t &payload,
                void *data) override;

            uint32_t calculate_serialized_size(
                const void *const data,
                eprosima::fastdds::dds::DataRepresentationId_t data_representation) override;

            bool compute_key(
                eprosima::fastdds::rtps::SerializedPayload_t &payload,
                eprosima::fastdds::rtps::InstanceHandle_t &handle,
                bool force_md5 = false) override;

            bool compute_key(
                const void *const data,
                eprosima::fastdds::rtps::InstanceHandle_t &handle,
                bool force_md5 = false) override;

            void *create_data() override;

            void delete_data(void *data) override;

            void register_type_object_representation() override;
        };

        eprosima::fastdds::dds::DomainParticipant *m_participant = nullptr;
        eprosima::fastdds::dds::Publisher *m_cmd_vel_publisher = nullptr;
        eprosima::fastdds::dds::Topic *m_cmd_vel_topic = nullptr;
        eprosima::fastdds::dds::TypeSupport m_cmd_vel_type{new CmdVelType()};
        eprosima::fastdds::dds::DataWriter *m_cmd_vel_writer = nullptr;
    };
}
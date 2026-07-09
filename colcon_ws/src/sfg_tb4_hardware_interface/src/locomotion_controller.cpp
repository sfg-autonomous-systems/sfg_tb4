#include "sfg_tb4_hardware_interface/locomotion_controller.hpp"

#include <fastcdr/Cdr.h>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/rtps/common/CdrSerialization.hpp>
#include <geometry_msgs/msg/detail/twist__rosidl_typesupport_fastrtps_cpp.hpp>
#include <rmw_fastrtps_cpp/rmw_fastrtps_cpp/MessageTypeSupport.hpp>

#include "sfg_utils/fqn/ros_fqn_builder.hpp"

namespace sfg_tb4_hardware_interface
{
    LocomotionController::LocomotionController(const rclcpp::NodeOptions &options) : LocomotionControllerBase("locomotion_controller", options)
    {
        using namespace sfg_utils::fqn;
        using namespace eprosima::fastdds::dds;

        auto type_support_handle = rosidl_typesupport_fastrtps_cpp__get_message_type_support_handle__geometry_msgs__msg__Twist();
        auto callbacks = static_cast<const message_type_support_callbacks_t *>(type_support_handle->data);
        m_cmd_vel_type.reset(new rmw_fastrtps_cpp::MessageTypeSupport(callbacks, type_support_handle));

        DomainParticipantQos participant_qos = PARTICIPANT_QOS_DEFAULT;
        auto ros_domain_id = std::getenv("ROS_DOMAIN_ID");
        m_participant = DomainParticipantFactory::get_instance()->create_participant(ros_domain_id ? std::stoul(ros_domain_id) : 0, participant_qos);

        if (m_participant == nullptr)
        {
            RCLCPP_ERROR(get_logger(), "Failed to create Fast DDS participant.");
            return;
        }

        // Register the type.
        m_cmd_vel_type.register_type(m_participant);

        // Create the publisher.
        PublisherQos publisher_qos = PUBLISHER_QOS_DEFAULT;
        m_participant->get_default_publisher_qos(publisher_qos);
        m_cmd_vel_publisher = m_participant->create_publisher(publisher_qos);

        if (m_cmd_vel_publisher == nullptr)
        {
            RCLCPP_ERROR(get_logger(), "Failed to create Fast DDS publisher.");
            return;
        }

        // Create the topic.
        auto topic_name = "rt" + (get_namespace() + ("/" + RosFqnBuilder().component(Component::Create3).resource(Resource::CmdVel).build(RosFqnSegment::Component, RosFqnSegment::Resource)));
        TopicQos topic_qos = TOPIC_QOS_DEFAULT;
        m_participant->get_default_topic_qos(topic_qos);
        m_cmd_vel_topic = m_participant->create_topic(
            topic_name,
            m_cmd_vel_type.get_type_name(),
            topic_qos);

        RCLCPP_INFO(get_logger(), "Created Fast DDS topic: %s", topic_name.c_str());

        if (m_cmd_vel_topic == nullptr)
        {
            RCLCPP_ERROR(get_logger(), "Failed to create Fast DDS topic.");
            return;
        }

        // Create the data writer.
        DataWriterQos writer_qos = DATAWRITER_QOS_DEFAULT;
        m_cmd_vel_publisher->get_default_datawriter_qos(writer_qos);
        writer_qos.reliability().kind = BEST_EFFORT_RELIABILITY_QOS;
        writer_qos.durability().kind = VOLATILE_DURABILITY_QOS;
        writer_qos.history().kind = KEEP_LAST_HISTORY_QOS;
        writer_qos.history().depth = 5;
        m_cmd_vel_writer = m_cmd_vel_publisher->create_datawriter(m_cmd_vel_topic, writer_qos);

        if (m_cmd_vel_writer == nullptr)
        {
            RCLCPP_ERROR(get_logger(), "Failed to create Fast DDS data writer.");
            return;
        }

        RCLCPP_INFO(get_logger(), "Started locomotion controller.");
    }

    LocomotionController::~LocomotionController()
    {
        using namespace eprosima::fastdds::dds;

        if (m_cmd_vel_publisher && m_cmd_vel_writer)
        {
            m_cmd_vel_publisher->delete_datawriter(m_cmd_vel_writer);
        }

        if (m_participant && m_cmd_vel_topic)
        {
            m_participant->delete_topic(m_cmd_vel_topic);
        }

        if (m_participant && m_cmd_vel_publisher)
        {
            m_participant->delete_publisher(m_cmd_vel_publisher);
        }

        if (m_participant)
        {
            DomainParticipantFactory::get_instance()->delete_participant(m_participant);
        }
    }

    void LocomotionController::apply_cmd(const geometry_msgs::msg::TwistStamped &cmd)
    {
        if (!m_cmd_vel_writer)
        {
            RCLCPP_ERROR(get_logger(), "Fast DDS data writer is not initialized.");
            return;
        }

        RCLCPP_INFO(get_logger(), "Publishing cmd_vel: linear=(%f, %f, %f), angular=(%f, %f, %f)",
                    cmd.twist.linear.x, cmd.twist.linear.y, cmd.twist.linear.z,
                    cmd.twist.angular.x, cmd.twist.angular.y, cmd.twist.angular.z);

        geometry_msgs::msg::Twist msg;
        msg.linear = cmd.twist.linear;
        msg.angular = cmd.twist.angular;

        auto type_support_handle = rosidl_typesupport_fastrtps_cpp__get_message_type_support_handle__geometry_msgs__msg__Twist();
        auto callbacks = static_cast<const message_type_support_callbacks_t *>(type_support_handle->data);

        rmw_fastrtps_shared_cpp::SerializedData serialized_data;
        serialized_data.type = rmw_fastrtps_shared_cpp::FASTDDS_SERIALIZED_DATA_TYPE_ROS_MESSAGE;
        serialized_data.data = &msg;
        serialized_data.impl = callbacks;

        m_cmd_vel_writer->write(&serialized_data);
    }
}
#include "sfg_tb4_hardware_interface/locomotion_controller.hpp"

#include <fastcdr/Cdr.h>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/rtps/common/CdrSerialization.hpp>

#include "sfg_utils/fqn/ros_fqn_builder.hpp"

namespace sfg_tb4_hardware_interface
{
    LocomotionController::CmdVelType::CmdVelType()
    {
        set_name("geometry_msgs::msg::dds_::Twist_");
        // Encapsulation header length (4 bytes) + plain CDR data data length (48 bytes).
        std::uint32_t type_size = 48;
        type_size += static_cast<std::uint32_t>(eprosima::fastcdr::Cdr::alignment(type_size, 4));
        max_serialized_type_size = type_size + 4;
        is_compute_key_provided = false;
    }

    bool LocomotionController::CmdVelType::serialize(
        const void *const data,
        eprosima::fastdds::rtps::SerializedPayload_t &payload,
        eprosima::fastdds::dds::DataRepresentationId_t data_representation)
    {
        using namespace eprosima::fastdds::dds;
        using namespace eprosima::fastcdr;

        if (data == nullptr)
        {
            return false;
        }

        const auto *msg = static_cast<const geometry_msgs::msg::Twist *>(data);

        FastBuffer buffer(reinterpret_cast<char *>(payload.data), payload.max_size);
        Cdr serializer(
            buffer,
            Cdr::DEFAULT_ENDIAN,
            data_representation == DataRepresentationId_t::XCDR_DATA_REPRESENTATION ? CdrVersion::XCDRv1 : CdrVersion::XCDRv2);

        payload.encapsulation = serializer.endianness() == Cdr::BIG_ENDIANNESS ? CDR_BE : CDR_LE;
        serializer.set_encoding_flag(
            data_representation == DataRepresentationId_t::XCDR_DATA_REPRESENTATION ? EncodingAlgorithmFlag::PLAIN_CDR : EncodingAlgorithmFlag::DELIMIT_CDR2);

        try
        {
            serializer.serialize_encapsulation();
            serializer << msg->linear.x << msg->linear.y << msg->linear.z;
            serializer << msg->angular.x << msg->angular.y << msg->angular.z;
            serializer.set_dds_cdr_options({0, 0});
        }
        catch (eprosima::fastcdr::exception::Exception &)
        {
            return false;
        }

        payload.length = static_cast<uint32_t>(serializer.get_serialized_data_length());
        return true;
    }

    bool LocomotionController::CmdVelType::deserialize(eprosima::fastdds::rtps::SerializedPayload_t &payload, void *data)
    {
        using namespace eprosima::fastdds::dds;
        using namespace eprosima::fastcdr;

        if (data == nullptr)
        {
            return false;
        }

        auto *msg = static_cast<geometry_msgs::msg::Twist *>(data);

        try
        {
            FastBuffer buffer(reinterpret_cast<char *>(payload.data), payload.length);
            Cdr deserializer(buffer, Cdr::DEFAULT_ENDIAN);

            deserializer.read_encapsulation();
            payload.encapsulation = deserializer.endianness() == Cdr::BIG_ENDIANNESS ? CDR_BE : CDR_LE;
            deserializer >> msg->linear.x >> msg->linear.y >> msg->linear.z;
            deserializer >> msg->angular.x >> msg->angular.y >> msg->angular.z;
        }
        catch (exception::Exception &)
        {
            return false;
        }

        return true;
    }

    uint32_t LocomotionController::CmdVelType::calculate_serialized_size(
        const void *const,
        eprosima::fastdds::dds::DataRepresentationId_t)
    {
        return max_serialized_type_size;
    }

    bool LocomotionController::CmdVelType::compute_key(
        eprosima::fastdds::rtps::SerializedPayload_t &,
        eprosima::fastdds::rtps::InstanceHandle_t &,
        bool)
    {
        return false;
    }

    bool LocomotionController::CmdVelType::compute_key(
        const void *const,
        eprosima::fastdds::rtps::InstanceHandle_t &,
        bool)
    {
        return false;
    }

    void *LocomotionController::CmdVelType::create_data()
    {
        return reinterpret_cast<void *>(new geometry_msgs::msg::Twist());
    }

    void LocomotionController::CmdVelType::delete_data(void *data)
    {
        delete reinterpret_cast<geometry_msgs::msg::Twist *>(data);
    }

    void LocomotionController::CmdVelType::register_type_object_representation()
    {
    }

    LocomotionController::LocomotionController(const rclcpp::NodeOptions &options) : LocomotionControllerBase("locomotion_controller", options)
    {
        using namespace sfg_utils::fqn;
        using namespace eprosima::fastdds::dds;

        auto factory = DomainParticipantFactory::get_instance();
        DomainParticipantQos participant_qos = PARTICIPANT_QOS_DEFAULT;
        auto ros_domain_id = std::getenv("ROS_DOMAIN_ID");
        m_participant = factory->create_participant(ros_domain_id ? std::stoul(ros_domain_id) : 0, participant_qos);

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
        m_cmd_vel_writer->write(&msg);
    }
}
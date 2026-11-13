#pragma once

#include <memory>
#include <string>

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/topic/Topic.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>
#include <rclcpp/qos.hpp>
#include <rosidl_runtime_c/message_type_support_struct.h>

namespace sfg_fastdds_utils
{
    /// Type-erased FastDDS publisher for ROS 2 messages.
    /**
     * Prefer the typed FastDDSPublisher<MessageT>. This class is useful when the message type
     * is only known at runtime (e.g. a type support handle loaded via rclcpp::get_message_typesupport_handle).
     *
     * \param type_support_handle Either a rosidl_typesupport_cpp handle (as returned by
     *        rosidl_typesupport_cpp::get_message_type_support_handle<MessageT>()) or a
     *        rosidl_typesupport_fastrtps_cpp handle. The FastRTPS handle is resolved internally.
     */
    class GenericFastDDSPublisher
    {
    public:
        GenericFastDDSPublisher(
            const std::string &topic_fqn,
            const rosidl_message_type_support_t *type_support_handle,
            const rclcpp::QoS &qos = rclcpp::SystemDefaultsQoS(),
            uint32_t domain_id = 0);
        GenericFastDDSPublisher(const GenericFastDDSPublisher &) = delete;
        GenericFastDDSPublisher(GenericFastDDSPublisher &&) = delete;
        GenericFastDDSPublisher &operator=(const GenericFastDDSPublisher &) = delete;
        GenericFastDDSPublisher &operator=(GenericFastDDSPublisher &&) = delete;
        ~GenericFastDDSPublisher();

        /// Publish a message. \p msg must point to an instance of the C++ message type of this publisher.
        bool publish(const void *msg);

    private:
        eprosima::fastdds::dds::DomainParticipant *m_participant{nullptr};
        eprosima::fastdds::dds::Publisher *m_publisher{nullptr};
        eprosima::fastdds::dds::Topic *m_topic{nullptr};
        eprosima::fastdds::dds::DataWriter *m_writer{nullptr};
        eprosima::fastdds::dds::TypeSupport m_type_support;

        const rosidl_message_type_support_t *m_type_support_handle{nullptr};
    };
}

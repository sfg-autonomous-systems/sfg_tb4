#include "sfg_fastdds_bridge/detail/generic_type_support.hpp"

#include <cstring>
#include <limits>

#include <rclcpp/serialized_message.hpp>

namespace sfg_fastdds_bridge::detail
{
    GenericTypeSupport::GenericTypeSupport(const std::string &dds_topic_type)
    {
        set_name(dds_topic_type);
        // The type is unbounded. Fast DDS 3.2 crashes while creating the writer's payload pool if this is 0, so we use the
        // smallest possible payload (the 4 byte encapsulation header) as the initial size. Payloads grow as needed thanks to
        // PREALLOCATED_WITH_REALLOC_MEMORY_MODE.
        max_serialized_type_size = 4;
        // ROS 2 messages are keyless.
        is_compute_key_provided = false;
    }

    bool GenericTypeSupport::serialize(
        const void *const data,
        eprosima::fastdds::rtps::SerializedPayload_t &payload,
        eprosima::fastdds::dds::DataRepresentationId_t)
    {
        const auto &message = static_cast<const rclcpp::SerializedMessage *>(data)->get_rcl_serialized_message();

        if (message.buffer_length > payload.max_size)
        {
            return false;
        }
        std::memcpy(payload.data, message.buffer, message.buffer_length);
        payload.length = static_cast<uint32_t>(message.buffer_length);
        return true;
    }

    bool GenericTypeSupport::deserialize(eprosima::fastdds::rtps::SerializedPayload_t &payload, void *data)
    {
        auto message = static_cast<rclcpp::SerializedMessage *>(data);

        if (message->capacity() < payload.length)
        {
            message->reserve(payload.length);
        }
        auto &rcl_message = message->get_rcl_serialized_message();
        std::memcpy(rcl_message.buffer, payload.data, payload.length);
        rcl_message.buffer_length = payload.length;
        return true;
    }

    uint32_t GenericTypeSupport::calculate_serialized_size(const void *const data, eprosima::fastdds::dds::DataRepresentationId_t)
    {
        const auto size = static_cast<const rclcpp::SerializedMessage *>(data)->size();

        if (size > std::numeric_limits<uint32_t>::max())
        {
            return 0;
        }
        return static_cast<uint32_t>(size);
    }

    void *GenericTypeSupport::create_data()
    {
        return new rclcpp::SerializedMessage();
    }

    void GenericTypeSupport::delete_data(void *data)
    {
        delete static_cast<rclcpp::SerializedMessage *>(data);
    }

    bool GenericTypeSupport::compute_key(eprosima::fastdds::rtps::SerializedPayload_t &, eprosima::fastdds::rtps::InstanceHandle_t &, bool)
    {
        return false;
    }

    bool GenericTypeSupport::compute_key(const void *const, eprosima::fastdds::rtps::InstanceHandle_t &, bool)
    {
        return false;
    }
}

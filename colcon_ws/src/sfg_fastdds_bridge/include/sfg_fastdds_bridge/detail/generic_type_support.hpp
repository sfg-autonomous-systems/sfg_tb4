#pragma once

#include <cstdint>
#include <string>

#include <fastdds/dds/topic/TopicDataType.hpp>

namespace sfg_fastdds_bridge::detail
{
    /// @brief Byte-passthrough Fast DDS type, whose samples are rclcpp::SerializedMessage instances.
    ///
    /// The serialized message already contains the encapsulation header followed by the CDR payload, which is exactly the
    /// RTPS payload format ROS 2 uses. Hence, bytes are copied unchanged in both directions.
    class GenericTypeSupport : public eprosima::fastdds::dds::TopicDataType
    {
    public:
        /// @param dds_topic_type The DDS topic type.
        explicit GenericTypeSupport(const std::string &dds_topic_type);

        bool serialize(
            const void *const data,
            eprosima::fastdds::rtps::SerializedPayload_t &payload,
            eprosima::fastdds::dds::DataRepresentationId_t data_representation) override;

        bool deserialize(eprosima::fastdds::rtps::SerializedPayload_t &payload, void *data) override;
        std::uint32_t calculate_serialized_size(const void *const data, eprosima::fastdds::dds::DataRepresentationId_t data_representation) override;
        void *create_data() override;
        void delete_data(void *data) override;
        bool compute_key(eprosima::fastdds::rtps::SerializedPayload_t &payload, eprosima::fastdds::rtps::InstanceHandle_t &ihandle, bool force_md5 = false) override;
        bool compute_key(const void *const data, eprosima::fastdds::rtps::InstanceHandle_t &ihandle, bool force_md5 = false) override;
    };
}

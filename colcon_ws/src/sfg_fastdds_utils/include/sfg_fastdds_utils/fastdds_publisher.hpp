#pragma once

#include <memory>
#include <string>

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/topic/Topic.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>

#include <rosidl_runtime_c/message_type_support_struct.h>

namespace sfg_fastdds_utils
{
    class FastDDSPublisher
    {
    public:
        FastDDSPublisher(
            const std::string &topic_fqn,
            const rosidl_message_type_support_t *type_support_handle,
            uint32_t domain_id = 0);
        FastDDSPublisher(const FastDDSPublisher &) = delete;
        FastDDSPublisher(FastDDSPublisher &&) = delete;
        FastDDSPublisher &operator=(const FastDDSPublisher &) = delete;
        FastDDSPublisher &operator=(FastDDSPublisher &&) = delete;
        ~FastDDSPublisher();

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
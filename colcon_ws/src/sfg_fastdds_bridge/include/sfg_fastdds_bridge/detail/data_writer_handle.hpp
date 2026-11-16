#pragma once

#include <functional>
#include <memory>

#include <fastdds/dds/publisher/DataWriter.hpp>

namespace sfg_fastdds_bridge::detail
{
    using DataWriterHandle = std::unique_ptr<eprosima::fastdds::dds::DataWriter, std::function<void(eprosima::fastdds::dds::DataWriter *)>>;
}

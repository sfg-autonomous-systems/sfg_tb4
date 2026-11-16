#pragma once

#include <functional>
#include <memory>

#include <fastdds/dds/subscriber/DataReader.hpp>

namespace sfg_fastdds_bridge::detail
{
    using DataReaderHandle = std::unique_ptr<eprosima::fastdds::dds::DataReader, std::function<void(eprosima::fastdds::dds::DataReader *)>>;
}

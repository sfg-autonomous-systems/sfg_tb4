#include <rmw_fastrtps_cpp/ServiceTypeSupport.hpp>

#include "sfg_fastdds_utils/type_support_utils.hpp"

namespace rmw_fastrtps_cpp
{
    ServiceTypeSupport::ServiceTypeSupport(const rosidl_message_type_support_t *type_supports) : TypeSupport(type_supports)
    {
    }
}
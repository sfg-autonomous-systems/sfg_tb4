#include <rmw_fastrtps_cpp/ServiceTypeSupport.hpp>

#include "sfg_fastdds_utils/type_support_utils.hpp"

namespace rmw_fastrtps_cpp
{
    RequestTypeSupport::RequestTypeSupport(const service_type_support_callbacks_t *members, const rosidl_message_type_support_t *type_supports) : ServiceTypeSupport(type_supports)
    {
        assert(members);
        auto msg = static_cast<const message_type_support_callbacks_t *>(members->request_members_->data);
        std::string name = _create_type_name(msg);
        set_name(name.c_str());
        set_members(msg);
    }
}
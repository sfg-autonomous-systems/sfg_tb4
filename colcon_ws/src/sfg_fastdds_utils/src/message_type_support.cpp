#include "sfg_fastdds_utils/type_support_utils.hpp"

namespace rmw_fastrtps_cpp
{
    MessageTypeSupport::MessageTypeSupport(const message_type_support_callbacks_t *members, const rosidl_message_type_support_t *type_supports) : TypeSupport(type_supports)
    {
        assert(members);
        std::string name = _create_type_name(members);
        set_name(name.c_str());
        set_members(members);
    }
}
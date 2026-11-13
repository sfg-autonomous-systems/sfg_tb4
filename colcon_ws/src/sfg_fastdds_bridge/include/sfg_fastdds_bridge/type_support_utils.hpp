#include <rcpputils/find_and_replace.hpp>
#include <rmw/error_handling.h>
#include <rmw_fastrtps_cpp/MessageTypeSupport.hpp>

namespace rmw_fastrtps_cpp
{
    inline std::string _create_type_name(std::string message_namespace, std::string message_name)
    {
        std::ostringstream ss;

        if (!message_namespace.empty())
        {
            // Find and replace C namespace separator with C++, in case this is using C type support.
            std::string message_namespace_new = rcpputils::find_and_replace(message_namespace, "__", "::");
            ss << message_namespace_new << "::";
        }

        ss << "dds_::" << message_name << "_";
        return ss.str();
    }

    inline std::string _create_type_name(const message_type_support_callbacks_t *members)
    {
        if (!members)
        {
            RMW_SET_ERROR_MSG("members handle is null");
            return "";
        }

        std::string message_namespace(members->message_namespace_);
        std::string message_name(members->message_name_);
        return _create_type_name(message_namespace, message_name);
    }
}
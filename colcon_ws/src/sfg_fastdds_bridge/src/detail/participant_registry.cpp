#include "sfg_fastdds_bridge/detail/participant_registry.hpp"

#include <stdexcept>
#include <string>

#include <rcl/domain_id.h>
#include <rcl/error_handling.h>

namespace sfg_fastdds_bridge::detail
{
    namespace
    {
        /// @brief Returns the domain ID that ROS uses by default.
        /// @return The value of ROS_DOMAIN_ID, or 0 if it is unset or empty.
        /// @throws std::runtime_error If ROS_DOMAIN_ID is invalid.
        std::uint32_t get_default_domain_id()
        {
            // rcl_get_default_domain_id leaves the value untouched if ROS_DOMAIN_ID is unset or empty.
            size_t domain_id = 0;

            if (rcl_get_default_domain_id(&domain_id) != RCL_RET_OK)
            {
                std::string error = rcl_get_error_string().str;
                rcl_reset_error();
                throw std::runtime_error("Failed to get the default domain ID: " + error);
            }
            return static_cast<std::uint32_t>(domain_id);
        }
    }

    std::shared_ptr<ParticipantHandle> ParticipantRegistry::get(std::optional<std::uint32_t> domain_id)
    {

        std::lock_guard lock(m_mutex);
        const auto resolved_domain_id = domain_id ? *domain_id : get_default_domain_id();
        auto &weak_participant = m_weak_participants[resolved_domain_id];
        auto participant = weak_participant.lock();

        if (!participant)
        {
            participant = std::make_shared<ParticipantHandle>(resolved_domain_id);
            weak_participant = participant;
        }
        return participant;
    }

    ParticipantRegistry &ParticipantRegistry::instance()
    {
        static ParticipantRegistry registry;
        return registry;
    }
}

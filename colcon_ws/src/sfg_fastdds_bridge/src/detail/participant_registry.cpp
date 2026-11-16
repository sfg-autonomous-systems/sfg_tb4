#include "sfg_fastdds_bridge/detail/participant_registry.hpp"

namespace sfg_fastdds_bridge::detail
{
    std::shared_ptr<ParticipantHandle> ParticipantRegistry::get(uint32_t domain_id)
    {
        std::lock_guard lock(m_mutex);
        auto &weak_participant = m_weak_participants[domain_id];
        auto participant = weak_participant.lock();

        if (!participant)
        {
            participant = std::make_shared<ParticipantHandle>(domain_id);
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

#pragma once

#include <cstdint>
#include <map>
#include <memory>
#include <mutex>

#include "sfg_fastdds_bridge/detail/participant_handle.hpp"

namespace sfg_fastdds_bridge::detail
{
    /// @brief Process-wide registry that shares one DomainParticipant per domain ID.
    ///
    /// The registry only holds weak references: the participant is created together with the first endpoint of a domain
    /// and deleted together with the last one, not during static teardown. Participant QoS (network interfaces, name, ...)
    /// is configured through a Fast DDS XML profile (FASTDDS_DEFAULT_PROFILES_FILE), not in code.
    class ParticipantRegistry
    {
    public:
        /// @brief Returns the process-wide registry.
        /// @return The registry instance.
        static ParticipantRegistry &instance();

        /// @brief Returns the participant handle of the given domain, creating it if necessary.
        /// @param domain_id The DDS domain ID.
        /// @return The shared participant handle.
        /// @throws std::runtime_error If the participant cannot be created.
        std::shared_ptr<ParticipantHandle> get(uint32_t domain_id);

    private:
        ParticipantRegistry() = default;
        ParticipantRegistry(const ParticipantRegistry &) = delete;
        ParticipantRegistry(ParticipantRegistry &&) = delete;
        ParticipantRegistry &operator=(const ParticipantRegistry &) = delete;
        ParticipantRegistry &operator=(ParticipantRegistry &&) = delete;
        ~ParticipantRegistry() = default;

        std::mutex m_mutex;
        std::map<uint32_t, std::weak_ptr<ParticipantHandle>> m_weak_participants;
    };
}

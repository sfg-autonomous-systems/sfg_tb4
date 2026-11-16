#pragma once

#include <cstdint>
#include <functional>
#include <memory>
#include <string>

#include <fastdds/dds/subscriber/DataReaderListener.hpp>
#include <rclcpp/qos.hpp>
#include <rclcpp/serialized_message.hpp>

#include "sfg_fastdds_bridge/detail/data_reader_handle.hpp"
#include "sfg_fastdds_bridge/detail/participant_handle.hpp"

namespace sfg_fastdds_bridge
{
    /// @brief Fast DDS subscription that receives serialized ROS messages, independent of the active RMW.
    ///
    /// The callback is invoked directly from the Fast DDS listener thread, not from an rclcpp executor. Exceptions thrown
    /// by the callback are caught and logged.
    ///
    /// There is no type checking: a wrong topic type only shows up as garbage when deserializing the received bytes.
    class GenericSubscription
    {
    public:
        using Callback = std::function<void(std::shared_ptr<const rclcpp::SerializedMessage>)>;

        /// @param topic_name The fully qualified ROS topic name.
        /// @param topic_type The ROS topic type.
        /// @param qos The QoS profile.
        /// @param callback The callback called with each received message, including the encapsulation header.
        /// @param domain_id The DDS domain ID.
        /// @throws std::invalid_argument If the callback is empty, the topic or type name is invalid, or the topic already
        /// exists with another type.
        /// @throws std::runtime_error If a DDS entity cannot be created.
        GenericSubscription(
            const std::string &topic_name,
            const std::string &topic_type,
            const rclcpp::QoS &qos,
            Callback callback,
            uint32_t domain_id = 0);
        GenericSubscription(const GenericSubscription &) = delete;
        GenericSubscription(GenericSubscription &&) = delete;
        GenericSubscription &operator=(const GenericSubscription &) = delete;
        GenericSubscription &operator=(GenericSubscription &&) = delete;

    private:
        class Listener : public eprosima::fastdds::dds::DataReaderListener
        {
        public:
            explicit Listener(Callback callback);
            void on_data_available(eprosima::fastdds::dds::DataReader *reader) override;

        private:
            Callback m_callback;
        };

        // Declaration order matters for correct destruction.
        Listener m_listener;
        std::shared_ptr<detail::ParticipantHandle> m_participant;
        std::shared_ptr<eprosima::fastdds::dds::Topic> m_topic;
        detail::DataReaderHandle m_reader;
    };
}

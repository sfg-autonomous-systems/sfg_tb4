#include "sfg_fastdds_bridge/generic_subscription.hpp"

#include <exception>
#include <stdexcept>

#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>
#include <rclcpp/logging.hpp>

#include "sfg_fastdds_bridge/detail/participant_registry.hpp"

#define STRINGIFY(x) #x

namespace sfg_fastdds_bridge
{
    GenericSubscription::GenericSubscription(
        const std::string &topic_name,
        const std::string &topic_type,
        const rclcpp::QoS &qos,
        Callback callback,
        uint32_t domain_id)
        : m_listener(std::move(callback)),
          m_participant(detail::ParticipantRegistry::instance().get(domain_id)),
          m_topic(m_participant->acquire_topic(topic_name, topic_type, qos)),
          m_reader(m_participant->create_reader(m_topic.get(), qos, &m_listener))
    {
    }

    GenericSubscription::Listener::Listener(Callback callback) : m_callback(std::move(callback))
    {
        if (!m_callback)
        {
            throw std::invalid_argument("Subscription callback must not be empty.");
        }
    }

    void GenericSubscription::Listener::on_data_available(eprosima::fastdds::dds::DataReader *reader)
    {
        eprosima::fastdds::dds::SampleInfo info;

        while (true)
        {
            auto message = std::make_shared<rclcpp::SerializedMessage>();

            if (reader->take_next_sample(message.get(), &info) != eprosima::fastdds::dds::RETCODE_OK)
            {
                break;
            }

            if (!info.valid_data)
            {
                continue;
            }

            try
            {
                m_callback(std::move(message));
            }
            catch (const std::exception &exception)
            {
                RCLCPP_ERROR(
                    rclcpp::get_logger(STRINGIFY(sfg_fastdds_bridge)), "Subscription callback for topic '%s' threw: %s",
                    reader->get_topicdescription()->get_name().c_str(), exception.what());
            }
        }
    }
}

#undef STRINGIFY
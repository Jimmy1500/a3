// A3 : Archive of Advanced Addons
#pragma once

#include <napi.h>
#include <sw/redis++/redis++.h>
#include <glog/logging.h>

namespace a3::lib {

class Worker final : public Napi::AsyncWorker {
    public:
        typedef std::pair<std::string, std::string> Item;
        typedef std::vector<Item>                   Items;
        typedef std::pair<std::string, Items>       Event; // <id, items>
        typedef std::vector<Event>                  Events;

        struct Cache {
            std::string id;
            std::string key;
            double val;
        };

        Worker(const std::string_view & home_topic, const std::string_view & health_topic, const Napi::Function & callback) noexcept;
        ~Worker() noexcept;

        void Execute() noexcept final;
        void OnOK()    noexcept final;

        bool connect(
            const std::string &         host            = "localhost",
            int                         port            = 6379,
            int                         db              = 0,
            std::size_t                 wait_ms         = 5000,
            sw::redis::ConnectionType   type = sw::redis::ConnectionType::TCP
        ) noexcept;

        constexpr std::string & homeTopic() noexcept { return home_topic_; }
        constexpr std::string & healthTopic() noexcept { return health_topic_; }
        constexpr Cache & home() noexcept { return home_; }
        constexpr Cache & health() noexcept { return health_; }
    private:
        std::string home_topic_;
        std::string health_topic_;
        Cache home_;
        Cache health_;

        sw::redis::ConnectionOptions connection_;
        std::unique_ptr<sw::redis::Redis> redis_;
};

} // namespace a3::lib
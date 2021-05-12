#include "a3/lib/Worker.hpp"
#include <chrono>
#include <thread>
#include <memory>
#include <vector>

namespace a3::lib {

Worker::Worker(const std::string_view & home_topic, const std::string_view & health_topic, const Napi::Function & callback) noexcept : 
AsyncWorker     ( callback ),
home_topic_     ( home_topic ),
health_topic_   ( health_topic ),
home_           { std::string(), std::string(), 0.0 },
health_         { std::string(), std::string(), 0.0 },
connection_     (),
redis_          { nullptr }
{

};

bool Worker::connect(const std::string & host, int port, int db, std::size_t wait_ms, sw::redis::ConnectionType type) noexcept {
    connection_.host = host;
    connection_.port = port;
    connection_.db   = db;
    connection_.type = type;
    if ( wait_ms ) { connection_.socket_timeout = std::chrono::milliseconds(wait_ms); }
    redis_.reset( new sw::redis::Redis(connection_) );
    return true;
}

Worker::~Worker() noexcept {
    DLOG(INFO) << "worker destroyed";
}

// cannot call javascript facilities here, executed on worker thread
void Worker::Execute() noexcept {
    if ( !home_topic_.size() ) { SetError ("No home topic specified for consumption"); }
    if ( !health_topic_.size() ) { SetError ("No health topic specified for consumption"); }
    Events home_events, health_events;

    redis_->xrange(home_topic_, "-", "+", std::back_inserter(home_events));
    for (const auto & event : home_events) {
        const auto & [ id, items ] { event };
        DLOG(INFO) << home_topic_ << " " << id;

        for (const auto & item : items) {
            const auto & [key, val] { item };
            DLOG(INFO) << " [" << key << ", " << val << "]";
            home_ = { id, key, std::stod(val) };
        }
        redis_->xdel(home_topic_, id);
    }

    redis_->xrange(health_topic_, "-", "+", std::back_inserter(health_events));
    for (const auto & event : health_events) {
        const auto & [ id, items ] { event };
        DLOG(INFO) << health_topic_ << " " << id;

        for (const auto & item : items) {
            const auto & [key, val] { item };
            DLOG(INFO) << " [" << key << ", " << val << "]";
            health_ = { id, key, std::stod(val) };
        }
        redis_->xdel(health_topic_, id);
    }
};

void Worker::OnOK() noexcept {
    Napi::Object home { Napi::Object::New(Env()) };
    home.Set("id", Napi::String::New(Env(),  home_.id) );
    home.Set("key", Napi::String::New(Env(), home_.key) );
    home.Set("val", Napi::Number::New(Env(), home_.val) );

    Napi::Object health { Napi::Object::New(Env()) };
    health.Set("id", Napi::String::New(Env(),  health_.id) );
    health.Set("key", Napi::String::New(Env(), health_.key) );
    health.Set("val", Napi::Number::New(Env(), health_.val) );

    std::string msg { "Worker: consumed topics (" + home_topic_ + "," + health_topic_ + ")" };
    Callback().Call(
        Env().Null(),
        {
            Napi::String::New(Env(), msg),
            Napi::Boolean::New(Env(), true),
            home,
            health,
        } // OnError() passes error as the first parameter if error was set
    );
};

} // namespace a3::lib
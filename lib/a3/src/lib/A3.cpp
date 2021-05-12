#include "a3/lib/Worker.hpp"

namespace a3::lib {

static inline Napi::Object consumeAsync(const Napi::CallbackInfo& info) noexcept {
    const Napi::String message { Napi::String::New(info.Env(), "message") };
    const Napi::String okay    { Napi::String::New(info.Env(), "okay") };

    Napi::Object done { Napi::Object::New(info.Env()) };
    if ( info.Length() != 3 ) {
        done.Set( message, Napi::String::New(info.Env(), "invalid number of parameters, 3 expected(home_topic, health_topic, callback)") );
        done.Set( okay, Napi::Boolean::New(info.Env(), false) );
        return done;
    }

    const Napi::String home_topic { info[0].As<Napi::String>() };
    const Napi::String health_topic { info[1].As<Napi::String>() };
    const Napi::Function & callback { info[2].As<Napi::Function>() };
    Worker * worker { new Worker(home_topic.Utf8Value(), health_topic.Utf8Value(), callback ) }; // not copyable, nodejs framework will delete worker

    if ( !worker->connect() ) {
        done.Set( message, Napi::String::New(info.Env(), "redis connection failed") );
        done.Set( okay, Napi::Boolean::New(info.Env(), false) );
        return done;
    }
    worker->Queue();

    std::string msg { "worker queued(" + worker->homeTopic() + "," + worker->healthTopic() + ")" };
    done.Set( message, Napi::String::New(info.Env(), msg) );
    done.Set( okay, Napi::Boolean::New(info.Env(), true) );
    return done;
};

} // namespace a3::lib

// exports
Napi::Object Init(Napi::Env env, Napi::Object exports) {
    exports.Set(Napi::String::New(env, "consumeAsync"), Napi::Function::New(env, a3::lib::consumeAsync));
    return exports;
}

NODE_API_MODULE(NODE_GYP_MODULE_NAME, Init)
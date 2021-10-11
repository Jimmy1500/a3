#include "a3/xml/Parser.hpp"
// #include <glog/logging.h>

namespace a3 {

template <typename T>
static inline std::string_view view(const Napi::Buffer<T> & buffer) {
    return std::string_view( reinterpret_cast<const char *>(buffer.Data()), buffer.Length() );
}

} // namespace a3

namespace a3::xml {

static inline Napi::Value parse(const Napi::CallbackInfo& info) noexcept {
    // sanity checks
    if ( info.Length() != 2 ) { return Napi::String::New(info.Env(), "invalid inputs, 2 expected(file_${buffer(s),name(s),string(s)}, callback)"); }
    if ( !info[1].IsFunction() ) { return Napi::String::New(info.Env(), "invalid input[1], callback function expected"); }

    const auto & source { info[0] };
    const auto & callback { info[1].As<Napi::Function>() };
    try {
        Parser * parser { nullptr }; // no need to delete, auto destroyed by NodeJS framework OnWorkComplete()

        if ( source.IsBuffer() ) {
            parser = new Parser( a3::view(source.As<Napi::Buffer<std::uint8_t>>()), callback );
        } else if ( source.IsString() ) {
            parser = new Parser( source.As<Napi::String>().Utf8Value(), callback );
        } else if ( source.IsArray() ) {
            const auto & sources { source.As<Napi::Array>() };
            std::vector<std::string_view> files;
            files.reserve( sources.Length() );

            for ( std::size_t i = 0; i < sources.Length(); ++i ) {
                if ( sources[i].IsBuffer() ) {
                    files.push_back( a3::view(sources[i].As<Napi::Buffer<std::uint8_t>>()) );
                } else if ( sources[i].IsString() ) {
                    files.push_back( sources[i].As<Napi::String>().Utf8Value() );
                } else {
                    return Napi::String::New(info.Env(), "invalid input[0][" + std::to_string(i) + "], string or buffer expected");
                }
            }
            parser = new Parser( files, callback );
        } else {
            return Napi::String::New(info.Env(), "invalid input[0], string or array expected");
        }

        parser->Queue();
        return Napi::Boolean::New(info.Env(), false);
    } catch ( const std::exception & e ) {
        return Napi::String::New(info.Env(), "parser cannot be queued(std::exception): " + std::string(e.what()));
    } catch ( ... ) {
        return Napi::String::New(info.Env(), "parser cannot be queued(unknown)");
    }
}

} // namespace a3::xml

// exports
static inline Napi::Object Init(Napi::Env env, Napi::Object exports) {
    exports.Set( Napi::String::New(env, "parse"), Napi::Function::New(env, a3::xml::parse) );
    return exports;
}

NODE_API_MODULE(NODE_GYP_MODULE_NAME, Init)
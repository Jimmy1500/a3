#include "a3/xml/Parser.hpp"

#include <chrono>
#include <thread>
#include <memory>
// #include <glog/logging.h>

namespace a3::xml {

Parser::Parser(const std::string_view& file, const Napi::Function& callback) noexcept :
Napi::AsyncWorker ( callback ),
files_            ( 1, std::string(file) ),
docs_             ( 1 )
{
}

Parser::Parser(const std::vector<std::string_view>& files, const Napi::Function& callback) noexcept :
Napi::AsyncWorker ( callback ),
files_            ( files.begin(), files.end() ),
docs_             ( files_.size() )
{
}

Parser::~Parser() noexcept {
    // DLOG(INFO) << "parser destroyed (" << files_[0] << ") ";
}


Napi::Object Parser::toJson(const pugi::xml_node & node) noexcept {
    Napi::Object obj { Napi::Object::New(Env()) };

    switch( node.type() ) {
        case pugi::xml_node_type::node_element:
            if ( node.first_attribute() ) {
                Napi::Object attr_obj { Napi::Object::New(Env()) };
                for ( auto attr = node.first_attribute(); attr; attr = attr.next_attribute() ) {
                    attr_obj.Set( Napi::String::New(Env(), attr.name()), Napi::String::New(Env(), attr.value()) );
                }
                obj.Set(Napi::String::New(Env(), "@"), attr_obj);
            }
            break;
        case pugi::xml_node_type::node_comment:
            obj.Set( Napi::String::New(Env(), "#"), Napi::String::New(Env(), node.value()) );
            break;
        default:
            break;
    } // switch ( node.type() )

    if ( std::distance(node.children().begin(), node.children().end()) > 0 )  {
        for ( const auto & child : node.children() ) {
            switch ( child.first_child().type() ) {
                case pugi::xml_node_type::node_pcdata:
                case pugi::xml_node_type::node_cdata:
                    obj.Set( Napi::String::New(Env(), child.name()), Napi::String::New(Env(), child.child_value()) );
                    break;
                default:
                    obj.Set( Napi::String::New(Env(), child.name()), toObject(child) );
                    break;
            }
        } // for ( const pugi::xml_node & node : xml.children() )
    }

    return obj;
}

} // namespace a3::xml

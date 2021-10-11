#pragma once

#include <napi.h>
#include <string>
#include <vector>
#include <filesystem>
#include <pugixml.hpp>

namespace a3::xml {

class Parser final : public Napi::AsyncWorker {
    private:
        std::vector<std::string> files_;
        std::vector<pugi::xml_document> docs_;
    public:
        explicit Parser()               noexcept = delete;
        explicit Parser(const Parser &) noexcept = delete;
        explicit Parser(Parser &&)      noexcept = delete;

        Parser(const std::string_view& file, const Napi::Function& callback) noexcept;
        Parser(const std::vector<std::string_view>& files, const Napi::Function& callback) noexcept;
        ~Parser() noexcept;

        /* xml -> json facilities */
        [[nodiscard]] bool         isArray  (const pugi::xml_node& node) noexcept;
        [[nodiscard]] Napi::Array  toArray  (const pugi::xml_node& node) noexcept;
        [[nodiscard]] Napi::Object toJson   (const pugi::xml_node& node) noexcept;
        [[nodiscard]] Napi::Object toObject (const pugi::xml_node& node) noexcept;
        [[nodiscard]] Napi::Object toObject ()                           noexcept;
        [[nodiscard]] bool         parse    (const std::string_view & file, pugi::xml_document& doc) noexcept;

        /* interface override */
        void Execute() noexcept override final;
        void OnOK   () noexcept override final;

        // get / set
        [[nodiscard]] constexpr std::vector<std::string>        & files() noexcept { return files_; }
        [[nodiscard]] constexpr std::vector<pugi::xml_document> & docs () noexcept { return docs_; }
        [[nodiscard]] std::string_view view(const std::string_view & file, std::size_t size = 20) const noexcept;

        // utility
        [[nodiscard]] static constexpr bool isContent(const std::string_view & file) noexcept {
            return file.size() > FILENAME_MAX || (
                file.find('<', 0) != std::string::npos &&
                file.rfind('>', file.size()) != std::string::npos && (
                    file.find("</", 3) != std::string::npos ||
                    file.rfind("/>", file.size()) != std::string::npos
                )
            );
        }
}; // class Parser

inline bool Parser::isArray(const pugi::xml_node& node) noexcept {
    return (
        std::distance(node.children().begin(), node.children().end()) > 1 &&
        !std::strcmp(node.first_child().name(), node.last_child().name())
    );
}

inline Napi::Array Parser::toArray(const pugi::xml_node& node) noexcept {
    Napi::Array array { Napi::Array::New(Env()) };
    for ( auto it = node.children().begin(); it != node.children().end(); ++it ) {
        array[ static_cast<std::size_t>(std::distance(node.children().begin(), it)) ] = toObject(*it);
    }
    return array;
}

inline Napi::Object Parser::toObject(const pugi::xml_node& node) noexcept {
    if ( isArray(node) ) { return toArray(node); }
    return toJson(node);
}

inline Napi::Object Parser::toObject() noexcept {
    if ( docs_.size() == 1 ) { return toObject(docs_[0]); }

    Napi::Array array { Napi::Array::New(Env()) };
    for (std::size_t i = 0; i != docs_.size(); ++i) {
        array[i] = toObject(docs_[i]);
    }
    return array;
}

inline bool Parser::parse(const std::string_view& file, pugi::xml_document& doc) noexcept {
    if ( !file.size() ) {
        SetError("parser aborted, file is empty");
        return false;
    } else if ( isContent(file) ) {
        // parse as string
        const pugi::xml_parse_result parsed { doc.load_buffer(file.data(), file.length()) };
        if ( !parsed ) {
            SetError("parser failed('" + std::string(view(file)) + "' as string), " + std::string(parsed.description()) );
            return false;
        }
    } else if ( std::filesystem::exists(file) ) {
        // parse as file
        const pugi::xml_parse_result parsed { doc.load_file(file.data()) };
        if ( !parsed ) {
            SetError("parser failed('" + std::string(view(file, file.size())) + "' as file), " + std::string(parsed.description()) );
            return false;
        }
    } else {
        SetError("parser failed('" + std::string(view(file, file.size())) + "' as file), file doesn't exist or invalid" );
        return false;
    }
    return true;
}

inline void Parser::Execute() noexcept {
    if ( !files_.size() ) { SetError("parser aborted, no file specified"); return; }
    for ( std::size_t i = 0; i != files_.size(); ++i ) {
        if ( !parse(files_[i], docs_[i]) ) { return; }
    }
}

inline void Parser::OnOK() noexcept {
    Callback().Call(
        Env().Null(),
        {
            Napi::Boolean::New(Env(), false),  // = error_message OnError(), see Napi::AsyncWorker::SetError()
            toObject()
        }
    );
}

inline std::string_view Parser::view(const std::string_view& file, std::size_t size) const noexcept {
    return ( file.size() > size ? std::string(file.substr(0, size)).append("...") : file );
}

} // namespace a3::xml
#pragma once

#include <string>
#include <string_view>
#include <optional>

namespace ekt
{

namespace utils
{
    std::optional<std::size_t> find_template_end(std::string_view str, size_t offset, const std::string& template_start, const std::string& template_end);

    std::optional<std::string> get_parent_key(const std::string& key);
    bool split_key(const std::string& key, std::string& parent, std::string& field);
}

}

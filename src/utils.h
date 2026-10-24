#pragma once

#include <string>
#include <string_view>
#include <optional>

namespace ekt
{

namespace utils
{
    std::string to_upper(const std::string& in);
    std::string to_lower(const std::string& in);
    // Removes leading and trailing whitespace. The result views into the input, so it must outlive it
    std::string_view trim(std::string_view in);
    std::optional<std::size_t> find_template_end(std::string_view str, size_t offset, const std::string& template_start, const std::string& template_end);
    std::optional<int> to_int(std::string_view sv);

    std::optional<std::string> get_parent_key(const std::string& key);
    bool split_key(const std::string& key, std::string& parent, std::string& field);
}

}

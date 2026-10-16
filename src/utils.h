#pragma once

#include <string>
#include <optional>

namespace ekt
{

namespace utils
{
    std::string to_upper(const std::string& in);
    std::optional<std::size_t> find_template_end(std::string_view str, size_t offset, const std::string& template_start, const std::string& template_end);
}

}

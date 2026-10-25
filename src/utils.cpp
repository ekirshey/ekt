#include "utils.h"

#include "ekt_defines.h"
#include <algorithm>
#include <unordered_map>
#include <charconv>
#include <ranges>
#include <vector>

namespace ekt
{

std::string utils::to_upper(const std::string& in)
{
    std::string upper = in;
    std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);
    return upper;
}

std::string utils::to_lower(const std::string& in)
{
    std::string upper = in;
    std::transform(upper.begin(), upper.end(), upper.begin(), ::tolower);
    return upper;
}

std::string_view utils::trim(std::string_view in)
{
    constexpr std::string_view whitespace = " \t\n\r\f\v";

    const auto first = in.find_first_not_of(whitespace);
    if (first == std::string_view::npos)
    {
        return {};
    }
    const auto last = in.find_last_not_of(whitespace);
    return in.substr(first, last - first + 1);
}

std::optional<std::size_t> utils::find_template_end(std::string_view str, std::size_t offset, const std::string& template_start, const std::string& template_end)
{
    int open_count = 0;
    size_t end;

    size_t i = offset;
    while(true)
    {
        const auto next_start = str.find(template_start, i);
        const auto temp_end = str.find(template_end, i);
        if (next_start != std::string_view::npos && next_start < temp_end)
        {
            open_count++;
            i = next_start + template_start.size();
            continue;
        }

        end = temp_end;
        if (end == std::string_view::npos)
        {
            return std::nullopt;
        }

        open_count--;
        if (open_count == 0)
        {
            break;
        }
        // End before open
        if (open_count < 0)
        {
            return std::nullopt;
        }
        i = end + key_template_end.size();
    }

    return end;
}

std::optional<int> utils::to_int(std::string_view sv)
{
    int value = 0;
    auto [ptr, ec] = std::from_chars(sv.data(), sv.data() + sv.size(), value);
    if(ec != std::errc{} || ptr != sv.data() + sv.size())
    {
        return std::nullopt; // not a number, out of range, or trailing characters
    }
    return value;
}

std::optional<std::string> utils::get_parent_key(const std::string& key)
{
    if(!key.contains(ownership_delimiter))
    {
        return std::nullopt;
    }

    auto split = std::views::split(key, std::string_view(ownership_delimiter))
            | std::ranges::to<std::vector<std::string>>();

    if (split.size() != 2)
    {
        return std::nullopt;
    }

    return utils::to_upper(split[0]);
}

bool utils::split_key(const std::string& key, std::string& parent, std::string& field)
{
    if(!key.contains(ownership_delimiter))
    {
        return false;
    }

    auto split = std::views::split(key, std::string_view(ownership_delimiter))
            | std::ranges::to<std::vector<std::string>>();

    if (split.size() != 2)
    {
        return false;
    }

    parent = utils::to_upper(split[0]);
    field = utils::to_upper(split[1]);

    return true;
}

}
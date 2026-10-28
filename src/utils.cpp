#include "utils.h"

#include "ekt_defines.h"
#include "ekcore/strings.h"

namespace ekt
{

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
        i = end + template_end.size();
    }

    return end;
}

std::optional<std::string> utils::get_parent_key(const std::string& key)
{
    if(!key.contains(ownership_delimiter))
    {
        return std::nullopt;
    }

    auto split = ekcore::split(key, ownership_delimiter);

    if (split.size() != 2)
    {
        return std::nullopt;
    }

    return ekcore::to_upper(split[0]);
}

bool utils::split_key(const std::string& key, std::string& parent, std::string& field)
{
    if(!key.contains(ownership_delimiter))
    {
        return false;
    }

    auto split = ekcore::split(key, ownership_delimiter);

    if (split.size() != 2)
    {
        return false;
    }

    parent = ekcore::to_upper(split[0]);
    field = ekcore::to_upper(split[1]);

    return true;
}

}

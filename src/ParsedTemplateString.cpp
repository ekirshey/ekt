#include <expected>
#include <optional>
#include <string>
#include <string_view>
#include <iostream>

#include "ParsedTemplateString.h"
#include "utils.h"

namespace ekt
{
namespace
{
    const std::string template_start = "![[";
    const std::string template_end = "]]";

    std::optional<std::size_t> find_template_end(std::string_view str, std::size_t start)
    {
        const auto end = str.find(template_end, start);
        if (end == std::string_view::npos)
        {
            return std::nullopt;
        }

        const auto nested = str.find(template_start, start);
        if (nested != std::string_view::npos && nested < end)
        {
            return std::nullopt;
        }

        return end;
    }
}

ParsedTemplateString::Result ParsedTemplateString::parse(const std::string& content)
{
    // Empty outputs are fine
    if (content.empty())
    {
        return {};
    }

    ParsedTemplateString res;
    res.m_content = content;

    std::size_t pos = 0;
    while ((pos = content.find(template_start, pos)) != std::string::npos)
    {
        const auto result = find_template_end(content, pos + template_start.size());
        if(!result)
        {
            return std::unexpected(false);
        }

        std::size_t end = *result;

        std::string inner = content.substr(
            pos + template_start.length(),
            end - pos - template_start.length());

        res.m_variableLocations.push_back({
            .start = pos,
            .end = end + template_end.length()
        });
        pos = end + template_end.length();
    }

    return std::move(res);
}

std::string ParsedTemplateString::resolve(const Context& context) const
{
    if(m_variableLocations.size() == 0)
    {
        return m_content;
    }

    std::string result;

    int idx = 0;
    for(const auto& loc : m_variableLocations)
    {
        result += m_content.substr(idx, loc.start - idx);
        auto var = get_variable(loc);
        if (context.contains(var))
        {
            result += context.get(var);
        }

        idx = loc.end;
    }

    if (idx < m_content.size())
    {
        result += m_content.substr(idx, m_content.size() - idx);
    }

    return result;
}

const std::vector<ParsedTemplateString::VariableLocation>& ParsedTemplateString::variables() const
{
    return m_variableLocations;
}

std::string ParsedTemplateString::get_variable(const VariableLocation& location) const
{
    const int start = location.start + template_start.length();
    const int length = location.end - template_end.length() - start;
    return utils::to_upper(std::string(m_content.data() + start, length));
}
}
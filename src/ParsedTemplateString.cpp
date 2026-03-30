#include <expected>
#include <string>

#include "ParsedTemplateString.h"

namespace
{
    const std::string template_start = "![[";
    const std::string template_end = "]]";
}

ParsedTemplateString::Result ParsedTemplateString::parse(const std::string& content)
{
    if (content.empty())
    {
        return std::unexpected(false);
    }
    ParsedTemplateString res;
    res.m_content = content;

    std::size_t pos = 0;
    while ((pos = content.find(template_start, pos)) != std::string::npos)
    {
        std::size_t end = content.find(template_end, pos);
        if (end == std::string::npos)
        {
            break;
        }

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
            result += context.get(std::string(var));
        }

        idx = loc.end;
    }

    if (idx < m_content.size())
    {
        result += m_content.substr(idx, m_content.size()- idx);
    }

    return result;
}

const std::vector<ParsedTemplateString::VariableLocation>& ParsedTemplateString::variables() const
{
    return m_variableLocations;
}

std::string_view ParsedTemplateString::get_variable(const VariableLocation& location) const
{
    const int start = location.start + template_start.length();
    const int length = location.end - template_end.length() - start;
    return std::string_view(m_content.data() + start, length);
}

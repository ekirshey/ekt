#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <expected>

#include "Context.h"

class ParsedTemplateString
{
public:
    struct VariableLocation
    {
        uint64_t start;
        uint64_t end;
    };

    ParsedTemplateString() = default;

    using Result = std::expected<ParsedTemplateString, bool>;
    static Result parse(const std::string& content);
    std::string resolve(const Context& context) const;

    const std::vector<VariableLocation>& variables() const;
    std::string_view get_variable(const VariableLocation& location) const;

private:
    std::string m_content;
    std::vector<VariableLocation> m_variableLocations;
};

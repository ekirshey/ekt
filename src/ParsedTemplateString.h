#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <expected>
#include <memory>

#include "Scope.h"
#include "Context.h"

namespace ekt
{

enum class VariableType
{
    Key,
    Loop,
    Conditional
};

class ParsedTemplateString;
struct TemplateVariable
{
    uint64_t start;
    uint64_t end;
    std::string label;
    VariableType type = VariableType::Key;
    std::shared_ptr<ParsedTemplateString> child = nullptr;
};

class ParsedTemplateString
{
public:
    ParsedTemplateString() = default;

    using Result = std::expected<ParsedTemplateString, bool>;
    static Result parse(const std::string_view& content);
    std::string resolve(const Context& context) const;

    const std::vector<TemplateVariable>& variables() const;
    std::string get_variable(const TemplateVariable& location) const;

private:
    std::string m_content;
    std::vector<TemplateVariable> m_variableLocations;

    std::string resolve_impl(Scope& scope) const;

    static std::optional<TemplateVariable> parse_key_template(const std::string_view& view, size_t start);
    static std::optional<TemplateVariable> parse_label_template(const std::string_view& view, size_t start, VariableType type);

    std::string resolve_key(Scope& scope, const TemplateVariable& var) const;
    std::string resolve_conditional(Scope& scope, const TemplateVariable& var) const;
    std::string resolve_loop(Scope& scope, const TemplateVariable& var) const;
};
}

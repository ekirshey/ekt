#include <expected>
#include <optional>
#include <string>
#include <string_view>
#include <iostream>

#include "ParsedTemplateString.h"
#include "utils.h"
#include "ekt_defines.h"

namespace ekt
{

ParsedTemplateString::Result ParsedTemplateString::parse(const std::string_view& content)
{
    // Empty outputs are fine
    if (content.empty())
    {
        return {};
    }

    ParsedTemplateString res;
    res.m_content = content;

    std::size_t pos = 0;
    while (pos < content.size())
    {
        if (content[pos] == '[')
        {
            std::optional<TemplateVariable> result = std::nullopt;
            const auto key_start = content.substr(pos, key_template_start.size());
            bool key_found = key_start == key_template_start;
            if(key_found)
            {
                result = parse_key_template(content, pos);
            }

            const auto cond_start = content.substr(pos, conditional_template_start.size());
            bool cond_found = cond_start == conditional_template_start;
            if(cond_found)
            {
                result = parse_label_template(content, pos, VariableType::Conditional);
            }

            const auto loop_start = content.substr(pos, loop_template_start.size());
            bool loop_found = loop_start == loop_template_start;
            if(loop_found)
            {
                result = parse_label_template(content, pos,  VariableType::Loop);
            }

            if (key_found || cond_found || loop_found)
            {
                if(!result)
                {
                    return std::unexpected(false);
                }
                pos = result->end;
                res.m_variableLocations.push_back(std::move(*result));
                continue;
            }
        }
        pos++;
    }

    return std::move(res);
}

std::string ParsedTemplateString::resolve(const Context& context) const
{
    Scope scope;
    scope.set_root_context(&context);
    return resolve_impl(scope);
}

const std::vector<TemplateVariable>& ParsedTemplateString::variables() const
{
    return m_variableLocations;
}

std::string ParsedTemplateString::get_variable(const TemplateVariable& location) const
{
    const int start = location.start + key_template_start.length();
    const int length = location.end - key_template_end.length() - start;
    return utils::to_upper(std::string(m_content.data() + start, length));
}

std::string ParsedTemplateString::resolve_impl(Scope& scope) const
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
        switch(loc.type)
        {
            case VariableType::Key:
            {
                result += resolve_key(scope, loc);
                break;
            }
            case VariableType::Conditional:
            {
                result += resolve_conditional(scope, loc);
                break;
            }
            case VariableType::Loop:
            {
                result += resolve_loop(scope, loc);
                break;
            }
        }
        idx = loc.end;
    }

    if (idx < m_content.size())
    {
        result += m_content.substr(idx, m_content.size() - idx);
    }

    return result;
}

std::optional<TemplateVariable> ParsedTemplateString::parse_key_template(const std::string_view& view, size_t start)
{
    const auto result = utils::find_template_end(view, start, key_template_start, key_template_end);
    if(!result)
    {
        return std::nullopt;
    }

    auto inner = view.substr(
        start + key_template_start.length(),
        *result - start - key_template_start.length());

    auto inner_parse = parse(inner);
    if(!inner_parse.has_value())
    {
        return std::nullopt;
    }

    std::size_t end = *result;

    return TemplateVariable{
        .start = start,
        .end = end + key_template_end.length(),
        .type = VariableType::Key,
        .child = std::make_shared<ParsedTemplateString>(*inner_parse)
    };
}

std::optional<TemplateVariable> ParsedTemplateString::parse_label_template(const std::string_view& view, size_t start, VariableType type)
{
    std::string start_template;
    std::string end_template;
    switch(type)
    {
        case VariableType::Key:
            return std::nullopt;
        case VariableType::Conditional:
        {
            start_template = conditional_template_start;
            end_template = conditional_template_end;
            break;
        }
        case VariableType::Loop:
        {
            start_template = loop_template_start;
            end_template = loop_template_end;
            break;
        }
    }
    const auto label_delim = view.find(label_delimiter, start);
    if (label_delim == std::string::npos)
    {
        return std::nullopt;
    }

    auto label = view.substr(
        start + start_template.length(),
        label_delim - start - end_template.length());

    const auto result = utils::find_template_end(view, start, start_template, end_template);
    if(!result)
    {
        return std::nullopt;
    }
    std::size_t end = *result;

    auto inner = view.substr(
        label_delim + label_delimiter.size(),
        *result - label_delim - label_delimiter.size());

    auto inner_parse = parse(inner);
    if(!inner_parse.has_value())
    {
        return std::nullopt;
    }

    return TemplateVariable{
        .start = start,
        .end = end + end_template.length(),
        .label = std::string(label),
        .type = type,
        .child = std::make_shared<ParsedTemplateString>(*inner_parse)
    };
}

std::string ParsedTemplateString::resolve_key(Scope& scope, const TemplateVariable& var) const
{
    std::string key;

    //EKTODO error check?
    if (var.child != nullptr)
    {
        key = var.child->resolve_impl(scope);
    }
    else 
    {
        key = get_variable(var);
    }

    return scope.get_value(key);
}

std::string ParsedTemplateString::resolve_conditional(Scope& scope, const TemplateVariable& var) const
{
    if(!scope.get_condition(var.label))
    {
        return "";
    }
    return var.child->resolve_impl(scope);
}

std::string ParsedTemplateString::resolve_loop(Scope& scope, const TemplateVariable& var) const
{
    auto arr = scope.get_array(var.label);
    if (arr.empty())
    {
        return "";
    }

    std::string result;
    for(auto& a : arr)
    {
        scope.add_child_context(var.label, &a);
        result += var.child->resolve_impl(scope);
        scope.remove_child_context(var.label);
    }
    return result;
}

}
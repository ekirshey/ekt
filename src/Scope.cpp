#include "Scope.h"
#include <iostream>
#include <ranges>
#include <string_view>
#include <vector>
#include <ranges>
#include "Context.h"
#include "utils.h"
#include "ekt_defines.h"

namespace ekt
{

bool Scope::contains(const std::string& key) const
{
    auto parsed_key = parse_key(key);
    if (m_child_contexts.empty() || !parsed_key.has_value() || !m_child_contexts.contains(parsed_key->parent_name))
    {
        return m_root_context->contains(key);
    }

    return m_child_contexts.at(parsed_key->parent_name)->contains(parsed_key->identifier);
}

std::string Scope::get_value(const std::string& key) const
{
    auto parsed_key = parse_key(key);
    if (m_child_contexts.empty() || !parsed_key.has_value() || !m_child_contexts.contains(parsed_key->parent_name))
    {
        return m_root_context->get_value(key);
    }

    return m_child_contexts.at(parsed_key->parent_name)->get_value(parsed_key->identifier);
}

bool Scope::get_condition(const std::string& key) const
{
    auto parsed_key = parse_key(key);
    if (m_child_contexts.empty() || !parsed_key.has_value() || !m_child_contexts.contains(parsed_key->parent_name))
    {
        return m_root_context->get_condition(key);
    }

    return m_child_contexts.at(parsed_key->parent_name)->get_condition(parsed_key->identifier);
}

std::span<const Context> Scope::get_array(const std::string& key) const
{
    auto parsed_key = parse_key(key);
    if (m_child_contexts.empty() || !parsed_key.has_value() || !m_child_contexts.contains(parsed_key->parent_name))
    {
        return m_root_context->get_array(key);
    }

    return m_child_contexts.at(parsed_key->parent_name)->get_array(parsed_key->identifier);
}

void Scope::set_root_context(const Context* context)
{
    m_root_context = context;
}

void Scope::add_child_context(const std::string& parent_name, const Context* context)
{
    m_child_contexts[utils::to_upper(parent_name)] = context;
}

void Scope::remove_child_context(const std::string& parent_name)
{
    m_child_contexts.erase(utils::to_upper(parent_name));
}

std::optional<Scope::Key> Scope::parse_key(const std::string& key) const
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

    return Scope::Key {
        .parent_name = utils::to_upper(split[0]),
        .identifier = utils::to_upper(split[1])
    };
}


}
#include "Scope.h"
#include <iostream>
#include <list>
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
    auto parent_key = get_parent_key(key);
    if (m_child_contexts.empty() || !parent_key.has_value() || !m_child_contexts.contains(*parent_key))
    {
        return m_root_context->contains(key);
    }

    return m_child_contexts.at(*parent_key)->contains(key);
}

std::string Scope::get_value(const std::string& key) const
{
    auto parent_key = get_parent_key(key);
    if (m_child_contexts.empty() || !parent_key.has_value() || !m_child_contexts.contains(*parent_key))
    {
        return m_root_context->get_value(key);
    }

    return m_child_contexts.at(*parent_key)->get_value(key);
}

std::span<const Context> Scope::get_array(const std::string& key) const
{
    auto parent_key = get_parent_key(key);
    if (m_child_contexts.empty() || !parent_key.has_value() || !m_child_contexts.contains(*parent_key))
    {
        return m_root_context->get_array(key);
    }

    return m_child_contexts.at(*parent_key)->get_array(key);
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

std::optional<std::string> Scope::get_parent_key(const std::string& key) const
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


}

#include "Scope.h"
#include <iostream>
#include <list>
#include <ranges>
#include <string_view>
#include <vector>
#include "Context.h"
#include "utils.h"
#include "ekt_defines.h"

namespace ekt
{

bool Scope::contains(const std::string& key) const
{
    auto parent_key = utils::get_parent_key(key);
    if (m_child_contexts.empty() || !parent_key.has_value() || !m_child_contexts.contains(*parent_key))
    {
        return m_root_context->contains(key);
    }

    return m_child_contexts.at(*parent_key).back()->contains(key);
}

std::string Scope::get_value(const std::string& key) const
{
    auto parent_key = utils::get_parent_key(key);
    if (m_child_contexts.empty() || !parent_key.has_value() || !m_child_contexts.contains(*parent_key))
    {
        return m_root_context->get_value(key);
    }

    return m_child_contexts.at(*parent_key).back()->get_value(key);
}

std::span<const Context> Scope::get_array(const std::string& key) const
{
    auto parent_key = utils::get_parent_key(key);
    if (m_child_contexts.empty() || !parent_key.has_value() || !m_child_contexts.contains(*parent_key))
    {
        return m_root_context->get_array(key);
    }

    return m_child_contexts.at(*parent_key).back()->get_array(key);
}

void Scope::set_root_context(const Context* context)
{
    m_root_context = context;
}

void Scope::add_child_context(const std::string& parent_name, const Context* context)
{
    m_child_contexts[utils::to_upper(parent_name)].push_back(context);
}

int Scope::get_num_child_contexts(const std::string& parent_name) const
{
    const auto key = utils::to_upper(parent_name);
    if (!m_child_contexts.contains(key))
    {
        return 0;
    }

    return m_child_contexts.at(utils::to_upper(key)).size();
}

void Scope::clear_child_contexts(const std::string& parent_name)
{
    m_child_contexts[utils::to_upper(parent_name)].clear();
}

}

#include "Context.h"
#include <vector>

#include "utils.h"
#include <ekt_defines.h>

namespace ekt
{

bool Context::contains(const std::string& key) const
{
    return m_data.contains(utils::to_upper(key));
}

// What behavior on key collision?
void Context::concat(const Context& context, ResolveMode mode)
{
    for(auto& [k,v] : context.m_data)
    {
        if (m_data.contains(k))
        {
            switch(mode)
            {
                case ResolveMode::Ignore:
                    continue;
                case ResolveMode::Overwrite:
                    m_data[k] = v;
                    break;
            }
        }
        else
        {
            m_data.insert({k, v});
        }
    }
}

bool Context::set(const std::string& key, const std::string& value)
{
    const auto upper_key = utils::to_upper(key);
    if(!is_valid_key(upper_key))
    {
        return false;
    }
    m_data[upper_key].value = value;

    return true;
}

bool Context::append(const std::string& key, const Context& value)
{
    const auto upper_key = utils::to_upper(key);
    if(!is_valid_key(upper_key))
    {
        return false;
    }

    m_data[upper_key].array.emplace_back(value);

    return true;
}

std::string Context::get_value(const std::string& key) const
{
    auto upper_key = utils::to_upper(key);
    if (!m_data.contains(upper_key))
    {
        return "";
    }

    return m_data.at(upper_key).value;
}

std::span<const Context> Context::get_array(const std::string& key) const
{
    auto upper_key = utils::to_upper(key);
    if (!m_data.contains(upper_key))
    {
        return {};
    }
    return m_data.at(upper_key).array;
}

bool Context::is_valid_key(const std::string& key)
{
    if (key.contains('(') ||
        key.contains(')') ||
        key.contains('=') ||
        key.contains('!'))
    {
        return false;
    }

    if (key.empty())
    {
        return false;
    }

    return true;
}

}

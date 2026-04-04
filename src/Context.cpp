#include "Context.h"
#include "utils.h"

bool Context::contains(const std::string& key) const
{
    return m_data.contains(key);
}

void Context::insert(const Context& context)
{
    for(auto& [k,v] : context.m_data)
    {
        m_data.insert({k, v});
    }
}

void Context::insert(const std::string& key, const std::string& value)
{
    m_data[to_upper(key)] = value;
}

std::string Context::get(const std::string& key) const
{
    return m_data.at(key);
}

#pragma once
#include <expected>
#include <unordered_map>
#include <string>
#include <span>
#include <optional>
#include "Context.h"

namespace ekt
{

// struct wrapper to standardize keys
class Scope
{
public:
    bool contains(const std::string& key) const;

    std::string get_value(const std::string& key) const;
    std::span<const Context> get_array(const std::string& key) const;
    std::string get_array_value(const std::string key, int idx) const;

    void set_root_context(const Context* context);
    void add_child_context(const std::string& parent_name, const Context* context);
    void remove_child_context(const std::string& parent_name);

private:
    std::optional<std::string> get_parent_key(const std::string& key) const;

    const Context* m_root_context;
    std::unordered_map<std::string, const Context*> m_child_contexts;
};
}

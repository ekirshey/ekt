#pragma once
#include <unordered_map>
#include <vector>
#include <string>
#include <span>

namespace ekt
{

struct ContextValue;

enum class ResolveMode
{
    Ignore,
    Overwrite
};

// struct wrapper to standardize keys
class Context
{
public:
    bool contains(const std::string& key) const;

    void concat(const Context& context, ResolveMode mode);

    bool set(const std::string& key, const std::string& value);
    bool append(const std::string& key, const Context& value);

    std::string get_value(const std::string& key) const;
    std::span<const Context> get_array(const std::string& key) const;

private:
    bool is_valid_key(const std::string& key);

    std::unordered_map<std::string, ContextValue> m_data;
};

struct ContextValue
{
    std::string value;
    std::vector<Context> array;
};

}

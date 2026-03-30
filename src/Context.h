#pragma once
#include <unordered_map>
#include "string_hash.h"
#include <string>

// struct wrapper to standardize keys
class Context
{
public:
    bool contains(const std::string& key) const;
    bool contains(const std::string_view& key) const;

    void insert(const Context& context);
    void insert(const std::string& key, const std::string& value);

    std::string get(const std::string& key) const;
private:
    std::unordered_map<std::string, std::string, string_hash, std::equal_to<>> m_data;
};

#pragma once
#include <unordered_map>
#include "string_hash.h"
#include <string>

// struct wrapper to get around sol2 issues
struct Context
{
    std::unordered_map<std::string, std::string, string_hash, std::equal_to<>> data;
};

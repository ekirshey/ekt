#pragma once

#include <string>
#include <vector>
#include "Context.h"
#include "sol/sol.hpp"
#include "string_hash.h"

using TemplateFunction = std::unordered_map<std::string, sol::protected_function, string_hash, std::equal_to<>>;

struct TemplateInputVariable
{
    std::string name;
    std::string default_value;
};

struct TemplateComponent
{
    std::string input_file;
    std::string output_file;
};

struct Template
{
    std::vector<TemplateComponent> components;
    std::vector<TemplateInputVariable> user_input;
    Context context;
    TemplateFunction functions;
    std::vector<std::string> post_commands;
    std::vector<std::string> chained_templates;
};

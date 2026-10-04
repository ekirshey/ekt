#pragma once

#include <string>
#include <vector>
#include <expected>
#include "Context.h"
#include "sol/sol.hpp"
#include "string_hash.h"

using TemplateFunction = std::function<std::expected<std::string, std::string>(Context&)>;
using TemplateFunctionMap = std::unordered_map<std::string, TemplateFunction, string_hash, std::equal_to<>>;

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
    TemplateFunctionMap functions;
    std::vector<std::string> post_commands;
    std::vector<std::string> chained_templates;
};

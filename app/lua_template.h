#pragma once

#include <string>
#include <vector>
#include <expected>
#include <functional>
#include <unordered_map>
#include "Context.h"
#include "string_hash.h"

namespace ekt
{

using LuaTemplateFunction = std::function<std::expected<std::string, std::string>(Context&)>;
using LuaTemplateFunctionMap = std::unordered_map<std::string, LuaTemplateFunction, string_hash, std::equal_to<>>;

struct LuaTemplateInputVariable
{
    std::string name;
    std::string default_value;
};

struct LuaTemplateComponent
{
    std::string input_file;
    std::string output_file;
};

struct LuaTemplate
{
    std::vector<LuaTemplateComponent> components;
    std::vector<LuaTemplateInputVariable> user_input;
    Context context;
    LuaTemplateFunctionMap functions;
    std::vector<std::string> post_commands;
    std::vector<std::string> chained_templates;
};

}
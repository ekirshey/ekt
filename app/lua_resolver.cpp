#include "lua_resolver.h"
#include <fstream>
#include <ranges>
#include "ParsedTemplateString.h"
#include "utils.h"

namespace ekt
{
    
namespace
{
    bool get_input_file_content(const std::string& input_file, std::string& contents)
    {
        std::ifstream file(input_file);
        if (!file)
        {
            return false;
        }

       contents = std::string(
            (std::istreambuf_iterator<char>(file)),
            std::istreambuf_iterator<char>()
        );

       return true;
    }

    struct ParsedTemplateComponent
    {
        ParsedTemplateString input;
        ParsedTemplateString output_file;
    };

    std::expected<void, std::string> create_parsed_component(LuaTemplateComponent& component, ParsedTemplateComponent& p)
    {
        std::string file_contents;
        if(!get_input_file_content(component.input_file, file_contents))
        {
            return std::unexpected("Could not open input file: " + component.input_file);
        }

        auto input_result = ParsedTemplateString::parse(file_contents);
        if(!input_result)
        {
            return std::unexpected("Failed to parse input contents for: " + component.input_file);
        }

        auto output_result = ParsedTemplateString::parse(component.output_file);
        if(!output_result)
        {
            return std::unexpected("Failed to parse output file: " + component.output_file);
        }
 
        p.input = *input_result;
        p.output_file = *output_result;

        return {};
    }
}

void LuaResolver::add_template(const std::string& name, const LuaTemplate& ekt_template)
{
    m_templates.insert({name, ekt_template});
}

void LuaResolver::add_global_var(const std::string& key, const std::string& value)
{
    m_global_context.insert(key, value);
}

LuaResolver::Result LuaResolver::resolve_template(const std::string& template_name, const InputCallback& input_cb, const MissingVarCallback& missing_var_cb)
{
    EktTemplateResult resolved;

    if(auto result = resolve_template_impl(template_name, input_cb, missing_var_cb, resolved); !result)
    {
        return std::unexpected(result.error());
    }

    return resolved;
}

bool LuaResolver::template_exists(const std::string& template_name)
{
    return m_templates.contains(template_name);
}

std::vector<std::string> LuaResolver::available_templates()
{
    auto kv = std::views::keys(m_templates);
    return std::vector<std::string>(kv.begin(), kv.end());
}

std::expected<void, std::string> LuaResolver::resolve_template_impl(const std::string& template_name, const InputCallback& input_cb, const MissingVarCallback& missing_var_cb, EktTemplateResult& out)
{
    if (!m_templates.contains(template_name))
    {
        return std::unexpected("Invalid template name: " + template_name);
    }

    auto& selected_template = m_templates[template_name];

    Context context;
    context = selected_template.context;
    context.insert(m_global_context);

    std::vector<ParsedTemplateComponent> parsed_components;
    parsed_components.resize(selected_template.components.size());

    int i = 0;
    for(auto& c : selected_template.components)
    {
        if(auto result = create_parsed_component(c, parsed_components[i++]); !result)
        {
            return std::unexpected("Failed to parse component: " + c.input_file + " : " + c.output_file + " error: " + result.error() + " \n");
        }
    }

    // Get explicit user input variables
    get_user_input_variables(context, selected_template, input_cb);

    for(auto& p : parsed_components)
    {
        // Prompt users for any remaining variable names in output file and template
        get_missing_variables(context, selected_template, p.output_file, missing_var_cb);
        get_missing_variables(context, selected_template, p.input, missing_var_cb);
    }

    std::vector<ParsedTemplateString> parsed_cmds;
    // Allows for variables in post commands
    for(auto& cmd : selected_template.post_commands)
    {
        auto result = ParsedTemplateString::parse(cmd);
        if (!result)
        {
            return std::unexpected("Failed to parse command: " + cmd);
        }
        get_missing_variables(context, selected_template, *result, missing_var_cb);
        parsed_cmds.push_back(*result);
    }

    // Resolve commands
    if(auto result = resolve_functions(context, selected_template); !result)
    {
        return std::unexpected("Failed to resolve functions. error: " + result.error());
    }

    for(auto& p : parsed_components)
    {
        out.templates.push_back({
            .output_path = p.output_file.resolve(context),
            .content = p.input.resolve(context)
        });
    }

    for(auto& t : selected_template.chained_templates)
    {
        auto r = resolve_template_impl(t, input_cb, missing_var_cb, out);
        if(!r)
        {
            return std::unexpected("Failed to resolve chained template: " + t);
        }
    }

    for(auto& cmd : parsed_cmds)
    {
        auto resolved_cmd = cmd.resolve(context);
        out.post_commands.push_back(resolved_cmd);
    }

    return {};
}

void LuaResolver::get_user_input_variables(Context& context, const LuaTemplate& selected_template, const InputCallback& input_cb)
{
    for(const auto& user_input : selected_template.user_input)
    {
        context.insert(user_input.name, input_cb(user_input));
    }
}

void LuaResolver::get_missing_variables(Context& context, const LuaTemplate& selected_template, const ParsedTemplateString& parsed_template, const MissingVarCallback& missing_var_cb)
{
    const auto& found_variables = parsed_template.variables();
    for(const auto& loc : found_variables)
    {
        const auto& v = parsed_template.get_variable(loc);
        if (context.contains(v) || selected_template.functions.contains(v))
        {
            continue;
        }

        context.insert(v, missing_var_cb(v));
    }
}

std::expected<void, std::string>  LuaResolver::resolve_functions(Context& context, const LuaTemplate& selected_template)
{
    for(auto& [k,v] : selected_template.functions)
    {
        auto result = v(context);
        if (!result.has_value())
        {
            return std::unexpected(result.error());
        }

        context.insert(k, result.value());
    }

    return {};
}

}
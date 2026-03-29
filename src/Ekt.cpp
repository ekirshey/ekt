#include "Ekt.h"
#include <iostream>
#include <fstream>
#include <ranges>
#include "ParsedTemplateString.h"
#include "Template.h"
#include "utils.h"
#include "sol/sol.hpp"

#ifdef _WIN32
    #define POPEN _popen
    #define PCLOSE _pclose
#else
    #define POPEN popen
    #define PCLOSE pclose
#endif

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

    std::string get_user_input()
    {
        std::string line;
        std::getline(std::cin, line);

        return line;
    }

    std::pair<int, std::string> execute_command(const std::string& command)
    {
        std::array<char, 256> buffer;
        std::string output;

        FILE* pipe = popen(command.c_str(), "r");
        if (!pipe)
        {
            return {-1, ""};
        }

        while (fgets(buffer.data(), buffer.size(), pipe))
        {
            output += buffer.data();
        }

        int status = pclose(pipe);
        return {WEXITSTATUS(status), output};
    }

    struct ParsedTemplateComponent
    {
        ParsedTemplateString input;
        ParsedTemplateString output_file;
    };

    bool create_parsed_component(TemplateComponent& component, ParsedTemplateComponent& p)
    {
        std::string file_contents;
        if(!get_input_file_content(component.input_file, file_contents))
        {
            std::cerr << "Could not open input file: " << component.input_file;
            return false;
        }

        TRY_UNWRAP(input, ParsedTemplateString::parse(file_contents));
        TRY_UNWRAP(output_file, ParsedTemplateString::parse(component.output_file));

        p.input = input;
        p.output_file = output_file;

        return true;
    }
}

void Ekt::add_template(const std::string& name, const Template& ekt_template)
{
    m_templates.insert({name, ekt_template});
}

bool Ekt::resolve_template(const std::string& template_name)
{
    if (!m_templates.contains(template_name))
    {
        std::cerr << "Invalid template name: " << template_name;
        return false;
    }

    auto& selected_template = m_templates[template_name];

    Context context;
    context = selected_template.context;
    context.data.insert(m_global_context.begin(), m_global_context.end());

    std::vector<ParsedTemplateComponent> parsed_components;
    parsed_components.resize(selected_template.components.size());

    int i = 0;
    for(auto& c : selected_template.components)
    {
        if(!create_parsed_component(c, parsed_components[i++]))
        {
            std::cerr << "Failed to parse component: " << c.input_file << " : " << c.output_file << "\n";
            return false;
        }
    }

    // Get explicit user input variables
    get_user_input_variables(context, selected_template);

    for(auto& p : parsed_components)
    {
        // Prompt users for any remaining variable names in output file and template
        get_missing_variables(context, selected_template, p.output_file);
        get_missing_variables(context, selected_template, p.input);
    }

    // Resolve commands
    if(!resolve_functions(context, selected_template))
    {
        return false;
    }

    for(auto& p : parsed_components)
    {
        std::string outputfile = p.output_file.resolve(context);
        std::string content = p.input.resolve(context);
        std::cout << outputfile << std::endl;
        std::ofstream(outputfile) << content;
    }

    if(!run_post_commands(selected_template))
    {
        return false;
    }

    for(auto& t : selected_template.chained_templates)
    {
        if(!resolve_template(t))
        {
            return false;
        }
    }

    return true;
}

bool Ekt::template_exists(const std::string& template_name)
{
    return m_templates.contains(template_name);
}

std::vector<std::string> Ekt::available_templates()
{
    auto kv = std::views::keys(m_templates);
    return std::vector<std::string>(kv.begin(), kv.end());
}

void Ekt::get_user_input_variables(Context& context, const Template& selected_template)
{
    for(const auto& user_input : selected_template.user_input)
    {
        bool has_default = !user_input.default_value.empty();
        std::string default_value = has_default ? "[Default: " + user_input.default_value + "]" : "";
        std::cout << user_input.name << " " << default_value << ": ";
        std::string value = get_user_input();
        if (value.empty() && has_default)
        {
            value = user_input.default_value;
        }
        context.data.insert({user_input.name, value});
    }
}

void Ekt::get_missing_variables(Context& context, const Template& selected_template, const ParsedTemplateString& parsed_template)
{
    const auto& found_variables = parsed_template.variables();
    for(const auto& loc : found_variables)
    {
        const auto& v = parsed_template.get_variable(loc);
        if (context.data.contains(v) || selected_template.functions.contains(v))
        {
            continue;
        }

        // Get input for missing variables
        // TODO: maybe sanitize?s
        std::cout << v << ": ";
        std::string value = get_user_input();
        context.data.insert({std::string(v), value});
    }
}

bool Ekt::resolve_functions(Context& context, const Template& selected_template)
{
    for(auto& [k,v] : selected_template.functions)
    {
        //TODO: Move this into the interface
        auto result = v(context);
        if (!result.valid())
        {
            sol::error err = result;
            std::cerr << "Lua error: " << err.what() << '\n';
            return false;
        }

        sol::object ret = result;
        if (ret.get_type() != sol::type::string) {
            std::cerr << "callback must return a string\n";
            return false;
        }

        context.data[k] = ret.as<std::string>();
    }

    return true;
}

bool Ekt::run_post_commands(const Template& selected_template)
{
    for(auto& cmd : selected_template.post_commands)
    {
        auto [res, output] = execute_command(cmd);
        if (res < 0)
        {
            std::cerr << "Failed to execute command: " << cmd << "\n";
            return false;
        }
        else
        {
            std::cout << output << "\n";
        }
    }

    return true;
}

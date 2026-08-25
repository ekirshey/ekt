#include <sol/sol.hpp>
#include <iostream>
#include <fstream>

#include "CommandLine.h"
#include "Ekt.h"
#include "Config.h"

#ifdef _WIN32
    #define POPEN _popen
    #define PCLOSE _pclose
    #define WEXITSTATUS(s) (s)
#else
    #define POPEN popen
    #define PCLOSE pclose
#endif

std::string get_input(const TemplateInputVariable& user_input)
{
    bool has_default = !user_input.default_value.empty();
    std::string default_value = has_default ? "[Default: " + user_input.default_value + "]" : "";
    std::cout << user_input.name << " " << default_value << ": ";
    std::string value;
    std::getline(std::cin, value);
    if (value.empty() && has_default)
    {
        value = user_input.default_value;
    }
    return value;
}

std::string get_missing_var(const std::string& key)
{
    std::cout << key << ": ";
    std::string value;
    std::getline(std::cin, value);
    return value;
}

std::pair<int, std::string> execute_command(const std::string& command)
{
    std::array<char, 256> buffer;
    std::string output;

    FILE* pipe = POPEN(command.c_str(), "r");
    if (!pipe)
    {
        return {-1, ""};
    }

    while (fgets(buffer.data(), buffer.size(), pipe))
    {
        output += buffer.data();
    }

    int status = PCLOSE(pipe);
    return {WEXITSTATUS(status), output};
}

int main(int argc, char* argv[])
{
    Ekt ekt;

    CommandLineArgs args;
    if(!CommandLine::process_args(argc, argv, args))
    {
        return 1;
    }

    // Collect ekt scripts
    auto scripts = Config::find_scripts();
    if (!args.user_provided_script.empty())
    {
        scripts.push_back(args.user_provided_script);
    }

    if(auto result = ekt.initialize(scripts); !result)
    {
        std::cout << result.error() << std::endl;
        return 1;
    }

    if (args.chosen_template.empty() || !ekt.template_exists(args.chosen_template))
    {
        std::cout << "No valid template provided. \n\nAvailable templates are: \n";
        for(auto& name : ekt.available_templates())
        {
            std::cout << name << std::endl;
        }

        return 0;
    }

    auto ekt_result = ekt.resolve_template(args.chosen_template, get_input, get_missing_var);
    if(!ekt_result)
    {
        return 1;
    }

    for(auto& t : ekt_result->templates)
    {
        std::ofstream(t.output_path) << t.content;
    }
     
    for(auto& c : ekt_result->post_commands)
    {
        std::cout << "\nRunning command: \n " << c << "\n";
        auto [res, output] = execute_command(c);
        if (res < 0)
        {
            std::cerr << "Failed to execute command: " << c << "\n";
            return false;
        }
        else
        {
            std::cout << output << "\n";
        }
    }

    return 0;
}

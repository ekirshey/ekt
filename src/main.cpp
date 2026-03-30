#include <sol/sol.hpp>
#include <iostream>

#include "CommandLine.h"
#include "Ekt.h"
#include "lua_interface.h"
#include "Config.h"

int main(int argc, char* argv[])
{
    Ekt ekt;

    LuaInterface::build(ekt);

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

    if(!Config::load_scripts(scripts))
    {
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

    ekt.resolve_template(args.chosen_template);
    return 0;
}

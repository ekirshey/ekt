#include "CommandLine.h"
#include "lua_interface.h"
#include <iostream>

namespace fs = std::filesystem;

bool CommandLine::process_args(int argc, char* argv[], CommandLineArgs& args)
{
    if (argc == 1)
    {
        return true;
    }

    int template_idx = 1;

    // Validate a provided path
    if (std::string(argv[1]) == "--path")
    {
        fs::path path(argv[2]);
        if (!path.string().ends_with(LuaInterface::script_ext))
        {
            std::cerr << "Malformed path: " << path << "\n must have extension " << LuaInterface::script_ext << "\n";
            return false;
        }
        if (!fs::exists(path))
        {
            std::cerr << "Provided path does not exist: " << path << "\n";
            return false;
        }
        args.user_provided_script = argv[2];

        template_idx = 3;
    }

    if (template_idx < argc)
    {
        args.chosen_template = argv[template_idx];
    }

    return true;
}

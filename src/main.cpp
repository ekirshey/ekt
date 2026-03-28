#include <sol/sol.hpp>
#include <iostream>
#include <filesystem>

#include "Ekt.h"
#include "lua_interface.h"

namespace fs = std::filesystem;

constexpr std::string ekt_config_ext = "ekt.lua";

struct Config
{
    std::vector<fs::path> files;
    std::string chosen_template;
};

void find_local_config(Config& config)
{
    fs::path lua_config_file;
    for (const auto& entry : fs::directory_iterator(fs::current_path()))
    {
        if (entry.is_regular_file() && entry.path().string().ends_with(ekt_config_ext))
        {
            config.files.push_back(entry);
        }
    }
}

bool parse_arguments(int argc, char* argv[], Config& config)
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
        if (!path.string().ends_with(ekt_config_ext))
        {
            std::cerr << "Malformed path: " << path << "\n must have extension " << ekt_config_ext << "\n";
            return false;
        }
        if (!fs::exists(path))
        {
            std::cerr << "Provided path does not exist: " << path << "\n";
            return false;
        }
        config.files.push_back(argv[2]);

        template_idx = 3;
    }

    if (template_idx < argc)
    {
        config.chosen_template = argv[template_idx];
    }

    return true;
}

bool load_config_files(const Config& config)
{
    std::string error;
    std::cout << "Loading the following config: \n";
    for(auto& f : config.files)
    {
        std::cout << f << "\n";
        if(!LuaInterface::load_script_file(f, error))
        {
            std::cerr << "Failed to load: " << error;
            return false;
        }
    }

    std::cout << "\n";

    return true;
}

int main(int argc, char* argv[])
{
    Config config;
    Ekt ekt;

    LuaInterface::build(ekt);

    if(!parse_arguments(argc, argv, config))
    {
        return 1;
    }

    // Search for config in the current directory
    find_local_config(config);

    if(!load_config_files(config))
    {
        return 1;
    }

    if (config.chosen_template.empty() || !ekt.template_exists(config.chosen_template))
    {
        std::cout << "No valid template provided. \nAvailable templates are: \n";
        for(auto& name : ekt.available_templates())
        {
            std::cout << name << std::endl;
        }

        return 0;
    }

    ekt.resolve_template(config.chosen_template);
    return 0;
}

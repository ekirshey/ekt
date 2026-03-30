#include "Config.h"
#include <filesystem>
#include <iostream>

#ifdef _WIN32
  #include <shlobj.h>   // SHGetKnownFolderPath
  #include <combaseapi.h>
#endif

#include "lua_interface.h"

namespace fs = std::filesystem;

namespace
{
    fs::path get_config_dir(const std::string app_name)
    {
    #ifdef _WIN32
        PWSTR path = nullptr;
        if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, nullptr, &path)))
        {
            fs::path result = fs::path(path) / app_name;
            CoTaskMemFree(path);
            return result;
        }
        // fallback
        if (auto* env = std::getenv("APPDATA"))
        {
            return fs::path(env) / app_name;
        }

        return fs::path(".");

    #elif defined(__APPLE__)
        if (auto* home = std::getenv("HOME"))
        {
            return fs::path(home) / "Library" / "Application Support" / app_name;
        }
        return fs::path(".");

    #else
        if (auto* xdg = std::getenv("XDG_CONFIG_HOME"))
        {
            return fs::path(xdg) / app_name;
        }

        if (auto* home = std::getenv("HOME"))
        {
            return fs::path(home) / ".config" / app_name;
        }

        return fs::path(".");
    #endif
    }

    void search_directory(const fs::path& path, std::vector<fs::path>& scripts)
    {
        for (const auto& entry : fs::recursive_directory_iterator(path))
        {
            if (entry.is_regular_file() && entry.path().string().ends_with(LuaInterface::script_ext))
            {
                scripts.push_back(entry);
            }
        }
    }
}

std::vector<fs::path> Config::find_scripts()
{
    std::vector<fs::path> scripts;
    auto config_dir = get_config_dir("ekt");
    search_directory(config_dir, scripts);
    search_directory(fs::current_path(), scripts);

    return scripts;
}

bool Config::load_scripts(const std::vector<std::filesystem::path>& scripts)
{
    std::string error;
    std::cout << "Loading the following config: \n";
    for(auto& f : scripts)
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

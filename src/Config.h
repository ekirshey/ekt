#pragma once

#include <vector>
#include <filesystem>

namespace Config
{
    std::vector<std::filesystem::path> find_scripts();
    bool load_scripts(const std::vector<std::filesystem::path>& scripts);
};

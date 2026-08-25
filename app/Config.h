#pragma once

#include <vector>
#include <filesystem>

namespace Config
{
    std::vector<std::filesystem::path> find_scripts();
};

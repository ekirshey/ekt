#pragma once

#include "Ekt.h"
#include <filesystem>

namespace LuaInterface
{
    void build(Ekt& ekt);
    bool load_script_file(const std::filesystem::path& script, std::string& error);
}

#pragma once

#include "Ekt.h"
#include <filesystem>


namespace LuaInterface
{
    constexpr std::string script_ext = "ekt.lua";

    void build(Ekt& ekt);
    bool load_script_file(const std::filesystem::path& script, std::string& error);
}

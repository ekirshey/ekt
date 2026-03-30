#pragma once

#include "Ekt.h"
#include "sol/forward.hpp"
#include <expected>
#include <filesystem>

namespace LuaInterface
{
    constexpr std::string script_ext = "ekt.lua";

    void build(Ekt& ekt);
    bool load_script_file(const std::filesystem::path& script, std::string& error);
    std::expected<std::string, std::string> run_template_function(Context& context, const sol::protected_function& func);
}

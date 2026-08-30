#pragma once

#include "sol/sol.hpp"
#include <expected>
#include <filesystem>

class Ekt;
class Context;

class LuaInterface
{
public:
    void build(Ekt& ekt);
    std::expected<void, std::string> load_script_file(const std::filesystem::path& script);
    std::expected<std::string, std::string> run_template_function(Context& context, const sol::protected_function& func);

    inline static const std::string script_ext = "ekt.lua";
private:
    std::expected<void, std::string> execute();

    sol::state m_lua;
};
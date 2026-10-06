#pragma once

#include "sol/sol.hpp"
#include <expected>
#include <filesystem>

class Context;

namespace ekt
{

class LuaResolver;

class LuaInterface
{
public:
    void build(LuaResolver& ekt);
    std::expected<void, std::string> load_script_file(const std::filesystem::path& script);

    inline static const std::string script_ext = "ekt.lua";
private:
    std::expected<void, std::string> execute();

    sol::state m_lua;
};

}
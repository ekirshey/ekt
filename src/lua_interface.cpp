#include "lua_interface.h"

#include <expected>
#include <filesystem>
#include <unordered_set>
#include <algorithm>
#include <cctype>

#include "sol/sol.hpp"
#include "Template.h"
#include "Context.h"
#include "lua_interface.h"
#include "utils.h"

namespace fs = std::filesystem;

namespace
{
    sol::state lua;
    fs::path current_script;

    const char* root_table = "ekt";
    const char* entry_point = "build";

    // EKT Interface
    const char* add_template = "add_template";
    const char* add_global_var = "add_global_var";
    const char* get_filenames = "get_filenames";
    const char* get_script_dir = "get_script_dir";
    const char* get_platform = "get_platform";

    // Template Interface
    const char* add_component = "add_component";
    const char* add_key_value = "add_key_value";
    const char* add_user_input_var = "add_user_input_var";
    const char* add_function_var = "add_function_var";
    const char* add_post_command = "add_post_command";
    const char* add_chained_template = "add_chained_template";

    bool execute(std::string& error)
    {
        sol::optional<sol::function> func = lua[root_table][entry_point];
        if (func)
        {
            sol::protected_function_result res = func.value()();
            if (!res.valid())
            {
                error = res;
            }
        }
        else
        {
            error = "Must implement ekt.build\n";
        }

        return true;
    }
}

void LuaInterface::build(Ekt& ekt)
{
    lua.open_libraries(sol::lib::base);

    sol::table ekt_table = lua.create_named_table(root_table);
    ekt_table[add_template] = [&ekt](const std::string& name, const Template& ekt_template)
        {
            ekt.add_template(name, ekt_template);
        };

    ekt_table[add_global_var] = [&ekt](const std::string& key, const std::string& value)
        {
            ekt.add_global_var(key, value);
        };

    ekt_table[get_filenames] = [](const std::string& path, std::vector<std::string> extensions) -> std::string
        {
            if (path.size() == 0)
            {
                return "";
            }

            std::unordered_set<std::string> exts(extensions.begin(), extensions.end());
            fs::path start(path);

            if(!fs::exists(start))
            {
                std::cerr << "Invalid path provided to ekt.get_filenames: " << start << "\n";
                return "";
            }

            std::string result;
            for (const auto& entry : fs::recursive_directory_iterator(start))
            {
                if (exts.contains(entry.path().extension().string()))
                {
                    auto relative = fs::relative(entry.path(), start).generic_string();
                    if (!result.empty())
                    {
                        result += '\n';
                    }
                    result += start.filename().string() + "/" + relative;
                }
            }

            return result;
        };

    ekt_table[get_script_dir] = []()
        {
            fs::path path(current_script);
            return path.parent_path().string();
        };

    ekt_table[get_platform] = []()
        {
            #ifdef _WIN32
            return "win";
            #elif defined(__APPLE__)
            return "mac";
            #else
            return "linux";
            #endif
        };

    lua.new_usertype<Context>("Context",
        sol::no_constructor,
        "get", [](const Context& context, const std::string& key) -> std::optional<std::string>
        {
            if (!context.contains(key))
            {
                return std::nullopt;
            }
            auto value = context.get(key);
            if (value.size() == 0)
            {
                return std::nullopt;
            }
            return value;
         }
    );

    lua.new_usertype<Template>("Template",
        sol::constructors<Template()>(),

        add_component, [](Template& t, const std::string& input_file, const std::string& output_file)
        {
            //TODO I don't want to uppercase the whole output, just the variables, if any
            t.components.push_back({
                .input_file = input_file,
                .output_file = output_file
            });
        },

        add_key_value, [](Template& t, const std::string& key, const std::string& value)
        {
            t.context.insert(key, value);
        },

        add_user_input_var, [](Template& t, const std::string& key, const std::string& default_value)
        {
            TemplateInputVariable input;
            input.name = to_upper(key);
            if (!default_value.empty())
            {
                input.default_value = default_value;
            }
            t.user_input.push_back(input);
        },

        add_function_var, [](Template& t, const std::string& key, sol::protected_function command)
        {
            t.functions[to_upper(key)] = command;
        },

        add_post_command, [](Template& t, const std::string& command)
        {
            t.post_commands.push_back(command);
        },

        add_chained_template, [](Template& t, const std::string& template_name)
        {
            t.chained_templates.push_back(template_name);
        }
    );

}

bool LuaInterface::load_script_file(const std::filesystem::path& script, std::string& error)
{
    if (script.empty())
    {
        error = "No config path provided";
        return false;
    }

    auto config = lua.safe_script_file(script);
    if (!config.valid())
    {
        error = config;
        return false;
    }

    current_script = script;
    auto result = execute(error);
    current_script.clear();

    return result;
}

std::expected<std::string, std::string>
LuaInterface::run_template_function(Context& context, const sol::protected_function& func)
{
    auto result = func(context);
    if (!result.valid())
    {
        sol::error err = result;
        return std::unexpected(err.what());
    }

    sol::object ret = result;
    if (ret.get_type() != sol::type::string)
    {
        return std::unexpected("callback must return a string\n");
    }

    return ret.as<std::string>();
}

#include "lua_interface.h"

#include <filesystem>
#include <unordered_set>

#include "sol/sol.hpp"
#include "Template.h"
#include "Context.h"
#include "lua_interface.h"

namespace fs = std::filesystem;

namespace
{
    sol::state lua;

    const char* root_table = "ekt";
    const char* entry_point = "build";

    // EKT Interface
    const char* add_template = "add_template";
    const char* get_filenames = "get_filenames";

    // Template Interface
    const char*  add_key_value = "add_key_value";
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
    ekt_table[add_template] = [&ekt](std::string name, const Template& ekt_template)
        {
            ekt.add_template(name, ekt_template);
        };

    ekt_table["get_filenames"] = [](const std::string& path, std::vector<std::string> extensions) -> std::string
        {
            if (path.size() == 0)
            {
                return "";
            }

            std::unordered_set<std::string> exts(extensions.begin(), extensions.end());
            fs::path start(path);

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

    lua.new_usertype<Context>("Context",
        sol::no_constructor,
        "get", [](const Context& context, const std::string& key) -> std::optional<std::string>
        {
             auto it = context.data.find(key);
             if (it == context.data.end())
             {
                 return std::nullopt;
             }
             return it->second;
         }
    );

    lua.new_usertype<Template>("Template",
        sol::constructors<Template()>(),
        "input_file",  &Template::input_file,
        "output_file", &Template::output_file,

        add_key_value, [](Template& t, const std::string& key, const std::string& value)
        {
            t.context.data[key] = value;
        },

        add_user_input_var, [](Template& t, const std::string& key, const std::string& default_value)
        {
            TemplateInputVariable input;
            input.name = key;
            if (!default_value.empty())
            {
                input.default_value = default_value;
            }
            t.user_input.push_back(input);
        },

        add_function_var, [](Template& t, const std::string& key, sol::protected_function command)
        {
            t.functions[key] = command;
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

    return execute(error);
}

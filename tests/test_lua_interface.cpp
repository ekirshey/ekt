#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_all.hpp>

#include "TempDir.h"

#include "Ekt.h"
#include "lua_interface.h"
#include "sol/sol.hpp"

#include <algorithm>
#include <filesystem>
#include <string>
#include <vector>

namespace fs = std::filesystem;

namespace
{
    // Constructing an Ekt builds its own LuaInterface, whose sol::state outlives
    // the templates that reference it, so tests just construct and load.
    void load(Ekt& ekt, const fs::path& script)
    {
        auto result = ekt.load_script_file(script);
        if (!result) { FAIL(result.error()); }
    }

    std::string ask_for_input(const TemplateInputVariable& input_var)
    {
        return "input:" + input_var.name;
    }

    std::string ask_for_missing(const std::string& name)
    {
        return "missing:" + name;
    }

    Ekt::Result resolve(Ekt& ekt, const std::string& name)
    {
        return ekt.resolve_template(name, ask_for_input, ask_for_missing);
    }

    // Renders a single component template and hands back its content.
    std::string render(Ekt& ekt, const std::string& name)
    {
        auto result = resolve(ekt, name);
        if (!result) { FAIL(result.error()); }
        REQUIRE(result->templates.size() == 1);
        return result->templates[0].content;
    }

    std::string normalize(const std::string& path)
    {
        return fs::path(path).lexically_normal().generic_string();
    }

    std::vector<std::string> split_lines(const std::string& in)
    {
        std::vector<std::string> lines;
        std::size_t pos = 0;
        while (!in.empty())
        {
            auto end = in.find('\n', pos);
            if (end == std::string::npos)
            {
                lines.push_back(in.substr(pos));
                break;
            }
            lines.push_back(in.substr(pos, end - pos));
            pos = end + 1;
        }

        std::sort(lines.begin(), lines.end());
        return lines;
    }
}

TEST_CASE("load_script_file rejects an empty path", "[lua_interface]")
{
    Ekt ekt;

    auto result = ekt.load_script_file({});
    REQUIRE_FALSE(result.has_value());
    REQUIRE(result.error() == "Failed to load: No config path provided");
}

TEST_CASE("load_script_file reports a script that cannot be loaded", "[lua_interface]")
{
    TempDir dir;

    Ekt ekt;

    SECTION("missing file")
    {
        auto result = ekt.load_script_file(dir.path() / "does_not_exist.ekt.lua");
        REQUIRE_FALSE(result.has_value());
        REQUIRE_THAT(result.error(), Catch::Matchers::ContainsSubstring("Failed to load:"));
    }

    SECTION("syntax error")
    {
        auto script = dir.write_script("function ekt.build( end");
        auto result = ekt.load_script_file(script);
        REQUIRE_FALSE(result.has_value());
        REQUIRE_THAT(result.error(), Catch::Matchers::ContainsSubstring("Failed to load:"));
    }
}

TEST_CASE("load_script_file reports a script without ekt.build", "[lua_interface]")
{
    TempDir dir;
    auto script = dir.write_script("local unused = 1");

    Ekt ekt;

    auto result = ekt.load_script_file(script);
    REQUIRE_FALSE(result.has_value());
    REQUIRE(result.error() == "Failed to load: Must implement ekt.build\n");
}

TEST_CASE("an error inside ekt.build is surfaced", "[lua_interface]")
{
    TempDir dir;
    auto script = dir.write_script(R"lua(
        function ekt.build()
            error("boom")
        end
    )lua");

    Ekt ekt;

    auto result = ekt.load_script_file(script);
    REQUIRE_FALSE(result.has_value());
    REQUIRE_THAT(result.error(), Catch::Matchers::ContainsSubstring("boom"));
}

TEST_CASE("ekt.add_template registers templates on the Ekt instance", "[lua_interface]")
{
    TempDir dir;
    auto script = dir.write_script(R"lua(
        function ekt.build()
            ekt.add_template("first", Template.new())
            ekt.add_template("second", Template.new())
        end
    )lua");

    Ekt ekt;
    load(ekt, script);

    REQUIRE(ekt.template_exists("first"));
    REQUIRE(ekt.template_exists("second"));
    REQUIRE_FALSE(ekt.template_exists("third"));

    auto available = ekt.available_templates();
    std::sort(available.begin(), available.end());
    REQUIRE(available == std::vector<std::string>{"first", "second"});
}

TEST_CASE("ekt.add_global_var values are visible to every template", "[lua_interface]")
{
    TempDir dir;
    dir.write("component.ekt", "author=![[author]]");
    auto script = dir.write_script(R"lua(
        function ekt.build()
            local dir = ekt.get_script_dir()
            ekt.add_global_var("author", "Erik Kirshey")

            local t = Template.new()
            t:add_component(dir .. "/component.ekt", dir .. "/out.txt")
            ekt.add_template("main", t)
        end
    )lua");

    Ekt ekt;
    load(ekt, script);

    REQUIRE(render(ekt, "main") == "author=Erik Kirshey");
}

TEST_CASE("Template bindings drive the resolved output", "[lua_interface]")
{
    TempDir dir;
    dir.write("component.ekt",
        "license=![[license]] project=![[project_name]] greeting=![[greeting]] unknown=![[unknown_var]]");
    dir.write("chained.ekt", "chained author=![[author]]");

    auto script = dir.write_script(R"lua(
        function greeting(context)
            return "hello " .. context:get("project_name")
        end

        function ekt.build()
            local dir = ekt.get_script_dir()
            ekt.add_global_var("author", "Erik Kirshey")

            local main = Template.new()
            main:add_component(dir .. "/component.ekt", dir .. "/out/![[project_name]].txt")
            main:add_key_value("license", "MIT")
            main:add_user_input_var("project_name", "default_project")
            main:add_function_var("greeting", greeting)
            main:add_post_command("echo ![[author]]")
            main:add_chained_template("extra")
            ekt.add_template("main", main)

            local extra = Template.new()
            extra:add_component(dir .. "/chained.ekt", dir .. "/out/extra.txt")
            ekt.add_template("extra", extra)
        end
    )lua");

    Ekt ekt;
    load(ekt, script);

    std::vector<TemplateInputVariable> requested;
    auto input_cb = [&requested](const TemplateInputVariable& input_var)
        {
            requested.push_back(input_var);
            return std::string("my_project");
        };

    auto result = ekt.resolve_template("main", input_cb, ask_for_missing);
    if (!result) { FAIL(result.error()); }

    SECTION("add_user_input_var forwards the name and default to the callback")
    {
        REQUIRE(requested.size() == 1);
        // Names are upper cased so they line up with the ![[..]] lookups
        REQUIRE(requested[0].name == "PROJECT_NAME");
        REQUIRE(requested[0].default_value == "default_project");
    }

    SECTION("add_component resolves both the content and the output path")
    {
        REQUIRE(result->templates.size() == 2);
        REQUIRE(normalize(result->templates[0].output_path) ==
                normalize((dir.path() / "out" / "my_project.txt").string()));
        REQUIRE(result->templates[0].content ==
                "license=MIT project=my_project greeting=hello my_project unknown=missing:UNKNOWN_VAR");
    }

    SECTION("add_chained_template pulls in the chained template's components")
    {
        REQUIRE(result->templates.size() == 2);
        REQUIRE(normalize(result->templates[1].output_path) ==
                normalize((dir.path() / "out" / "extra.txt").string()));
        REQUIRE(result->templates[1].content == "chained author=Erik Kirshey");
    }

    SECTION("add_post_command keeps the command and resolves its variables")
    {
        REQUIRE(result->post_commands == std::vector<std::string>{"echo Erik Kirshey"});
    }
}

TEST_CASE("template functions must return a string", "[lua_interface]")
{
    TempDir dir;
    dir.write("component.ekt", "value=![[value]]");

    auto script_returning = [&dir](const std::string& body)
        {
            return dir.write_script(
                "function value(context)\n" + body + "\nend\n"
                R"lua(
                function ekt.build()
                    local dir = ekt.get_script_dir()
                    local t = Template.new()
                    t:add_component(dir .. "/component.ekt", dir .. "/out.txt")
                    t:add_function_var("value", value)
                    ekt.add_template("main", t)
                end
                )lua");
        };

    SECTION("a string return lands in the context")
    {
        Ekt ekt;
        load(ekt, script_returning(R"(return "from lua")"));
        REQUIRE(render(ekt, "main") == "value=from lua");
    }

    SECTION("a non string return is an error")
    {
        Ekt ekt;
        load(ekt, script_returning("return 42"));

        auto result = resolve(ekt, "main");
        REQUIRE_FALSE(result.has_value());
        REQUIRE_THAT(result.error(), Catch::Matchers::ContainsSubstring("callback must return a string"));
    }

    SECTION("a lua error is propagated")
    {
        Ekt ekt;
        load(ekt, script_returning(R"(error("function blew up"))"));

        auto result = resolve(ekt, "main");
        REQUIRE_FALSE(result.has_value());
        REQUIRE_THAT(result.error(), Catch::Matchers::ContainsSubstring("function blew up"));
    }
}

TEST_CASE("Context:get is exposed to template functions", "[lua_interface]")
{
    TempDir dir;
    dir.write("component.ekt", "![[probe]]");

    auto script_looking_up = [&dir](const std::string& key)
        {
            return dir.write_script(
                "function probe(context)\n"
                "    local v = context:get(\"" + key + "\")\n"
                "    if v then return \"got:\" .. v end\n"
                "    return \"nil\"\n"
                "end\n"
                R"lua(
                function ekt.build()
                    local dir = ekt.get_script_dir()
                    ekt.add_global_var("mixed_Case", "value")
                    ekt.add_global_var("empty_var", "")

                    local t = Template.new()
                    t:add_component(dir .. "/component.ekt", dir .. "/out.txt")
                    t:add_function_var("probe", probe)
                    ekt.add_template("main", t)
                end
                )lua");
        };

    SECTION("lookups are case insensitive")
    {
        Ekt lower;
        load(lower, script_looking_up("mixed_case"));
        REQUIRE(render(lower, "main") == "got:value");

        Ekt upper;
        load(upper, script_looking_up("MIXED_CASE"));
        REQUIRE(render(upper, "main") == "got:value");
    }

    SECTION("an unknown key is nil")
    {
        Ekt ekt;
        load(ekt, script_looking_up("not_a_key"));
        REQUIRE(render(ekt, "main") == "nil");
    }

    SECTION("an empty value is nil")
    {
        Ekt ekt;
        load(ekt, script_looking_up("empty_var"));
        REQUIRE(render(ekt, "main") == "nil");
    }
}

TEST_CASE("ekt.get_script_dir returns the directory of the running script", "[lua_interface]")
{
    TempDir dir;
    dir.write("component.ekt", "![[script_dir]]");
    auto script = dir.write_script(R"lua(
        function ekt.build()
            local dir = ekt.get_script_dir()
            ekt.add_global_var("script_dir", dir)

            local t = Template.new()
            t:add_component(dir .. "/component.ekt", dir .. "/out.txt")
            ekt.add_template("main", t)
        end
    )lua");

    Ekt ekt;
    load(ekt, script);

    REQUIRE(normalize(render(ekt, "main")) == normalize(dir.path().string()));
}

TEST_CASE("ekt.get_platform reports the host platform", "[lua_interface]")
{
    TempDir dir;
    dir.write("component.ekt", "![[platform]]");
    auto script = dir.write_script(R"lua(
        function ekt.build()
            local dir = ekt.get_script_dir()
            ekt.add_global_var("platform", ekt.get_platform())

            local t = Template.new()
            t:add_component(dir .. "/component.ekt", dir .. "/out.txt")
            ekt.add_template("main", t)
        end
    )lua");

    Ekt ekt;
    load(ekt, script);

#ifdef _WIN32
    const std::string expected = "win";
#elif defined(__APPLE__)
    const std::string expected = "mac";
#else
    const std::string expected = "linux";
#endif

    REQUIRE(render(ekt, "main") == expected);
}

TEST_CASE("ekt.get_filenames walks a directory", "[lua_interface]")
{
    TempDir dir;
    dir.write("component.ekt", "![[files]]");
    dir.write("src/a.cpp", "");
    dir.write("src/a.h", "");
    dir.write("src/nested/b.cpp", "");
    dir.write("src/notes.txt", "");

    auto script_calling = [&dir](const std::string& call)
        {
            return dir.write_script(
                "function ekt.build()\n"
                "    local dir = ekt.get_script_dir()\n"
                "    ekt.add_global_var(\"files\", " + call + ")\n"
                R"lua(
                    local t = Template.new()
                    t:add_component(dir .. "/component.ekt", dir .. "/out.txt")
                    ekt.add_template("main", t)
                end
                )lua");
        };

    SECTION("only the requested extensions come back, relative to the search root")
    {
        Ekt ekt;
        load(ekt, script_calling(R"(ekt.get_filenames(dir .. "/src", { ".cpp", ".h" }))"));

        REQUIRE(split_lines(render(ekt, "main")) ==
                std::vector<std::string>{"src/a.cpp", "src/a.h", "src/nested/b.cpp"});
    }

    SECTION("an empty extension list matches nothing")
    {
        Ekt ekt;
        load(ekt, script_calling(R"(ekt.get_filenames(dir .. "/src", { }))"));
        REQUIRE(render(ekt, "main").empty());
    }

    SECTION("an empty path is empty")
    {
        Ekt ekt;
        load(ekt, script_calling(R"(ekt.get_filenames("", { ".cpp" }))"));
        REQUIRE(render(ekt, "main").empty());
    }

    SECTION("a path that does not exist is empty")
    {
        Ekt ekt;
        load(ekt, script_calling(R"(ekt.get_filenames(dir .. "/nope", { ".cpp" }))"));
        REQUIRE(render(ekt, "main").empty());
    }
}

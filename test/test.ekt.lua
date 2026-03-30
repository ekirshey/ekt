-- Globals
local app_root = "."
local dir = ""

function example_function(context)
    return "a function result"
end

function example_function_with_context(context)
    res = context:get("global_result")
    if res then
        return "a result " .. res
    end

    return "missing"
end

function example_function_with_missing_context(context)
    res = context:get("missing")
    if res then
        return "a result " .. res
    end

    return "missing context"
end

function get_files(context)
    return ekt.get_filenames(context:get("src_path"), { ".h", ".cpp" })
end

function get_internal_files(context)
    local f = ekt.get_filenames(dir .. "/src", { ".h", ".cpp" })
    print("files " .. f)
    return f
end

-- Build function
function ekt.build()
    dir = ekt.get_script_dir()
    local os = ekt.get_platform()
    local cmake_preset = "win" --get_cmake_preset(os)

    ekt.add_global_var("generate_warning", "This file is generated, do not modify")
    ekt.add_global_var("author", "Erik Kirshey")

    local app_cmake = Template.new()
    app_cmake:add_component(dir .. "/test_templates/AppCMakeListsTemplate.ekt", dir .. "/CMakeLists.txt")
    app_cmake:add_post_command("cmake --preset=" .. cmake_preset)
    app_cmake:add_function_var("internal_source_files", get_internal_files)
    ekt.add_template("app_cmake", app_cmake)
end

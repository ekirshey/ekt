-- Globals
local app_root = "."

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

-- Build function
function ekt.build()
    local chain = Template.new()
    chain.input_file = "test/test_templates/chained.ekt"
    chain.output_file = app_root .. "/test/output/chain.txt"
    chain:add_user_input_var("some shit")
    ekt.add_template("chain", chain);


    local simple = Template.new()
    simple.input_file = "test/test_templates/simple.ekt"
    simple.output_file = app_root .. "/test/output/![[filename]].txt"
    simple:add_key_value("filename", "simple_test")
    simple:add_key_value("global_result", "global_value")
    simple:add_key_value("src_path", app_root .. "/src")

    simple:add_user_input_var("user_input", "a default user input")
    simple:add_user_input_var("has_default", "a default")
    simple:add_user_input_var("no_default")

    simple:add_function_var("function_result", example_function)
    simple:add_function_var("function_result_with_context", example_function_with_context)
    simple:add_function_var("function_result_with_missing_context", example_function_with_missing_context)
    simple:add_function_var("file_list", get_files)

    simple:add_chained_template("chain")

    ekt.add_template("simple", simple);
end

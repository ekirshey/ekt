# EKT Lua Interface

EKT ("Erik Kirshey Templates") is a small code/file generator. You describe **templates** in a
Lua script; EKT loads the script, resolves the template you ask for, writes the generated files,
and runs any post-commands. This document describes the Lua-facing API and how a script is
structured, loaded, and resolved.

---

## 1. Script files

- A script is any file whose name ends in **`.ekt.lua`** (constant `LuaInterface::script_ext`).
- Scripts are discovered automatically from two places, searched **recursively**:
  1. The per-user config directory for `ekt`:
     - Windows: `%APPDATA%\ekt` (Roaming AppData)
     - macOS: `~/Library/Application Support/ekt`
     - Linux: `$XDG_CONFIG_HOME/ekt`, else `~/.config/ekt`
  2. The current working directory.
- A single extra script can be passed explicitly on the command line with `--path`
  (it must still end in `.ekt.lua` and exist).
- All discovered scripts are loaded into one shared `Ekt` instance, so templates and global
  variables defined across multiple scripts coexist.

Only the Lua `base` library is opened. Other standard libraries (`io`, `os`, `string`, `table`,
math, etc.) are **not** available unless you add them.

### Command-line usage

```
ekt <template_name>
ekt --path path/to/thing.ekt.lua <template_name>
```

Running with no valid template name prints the list of available templates and exits.

---

## 2. Entry point: `ekt.build()`

Every script **must** define `ekt.build`. After a script is loaded, EKT calls it. This is where
you register global variables and templates.

```lua
function ekt.build()
    -- register globals and templates here
end
```

If `ekt.build` is missing, loading the script fails with `"Must implement ekt.build"`.

---

## 3. The global `ekt` table

These functions are available anywhere in the script (typically used inside `ekt.build` and in
your function-vars).

| Function | Signature | Description |
|---|---|---|
| `ekt.add_template` | `(name: string, template: Template)` | Register a template under `name`. If the name already exists, the first registration wins (insert, not overwrite). |
| `ekt.add_global_var` | `(key: string, value: string)` | Add a variable available to **every** template. |
| `ekt.get_filenames` | `(path: string, extensions: {string}) -> string` | Recursively list files under `path` whose extension is in `extensions`. Returns a **newline-separated** string. |
| `ekt.get_script_dir` | `() -> string` | Absolute directory of the script currently being executed. Valid only during loading (e.g. inside `ekt.build`). |
| `ekt.get_platform` | `() -> string` | Returns `"win"`, `"mac"`, or `"linux"`. |

### `ekt.get_filenames` details

- `extensions` are matched exactly, **including the leading dot**, e.g. `{ ".h", ".cpp" }`.
- Each returned line is `<name-of-path-folder>/<relative-path>`. For `path = ".../src"` a match
  becomes `src/sub/foo.h`.
- Returns `""` if `path` is empty or does not exist (a warning is printed to stderr for a
  non-existent path).
- Because the result is one big newline-separated string, it is typically fed into a template
  variable and expanded in the output file (e.g. a generated `CMakeLists.txt` file list).

---

## 4. Templates

Create a template with `Template.new()`, configure it with the methods below, then register it
with `ekt.add_template`.

```lua
local t = Template.new()
t:add_component(dir .. "/templates/Foo.ekt", dir .. "/generated/Foo.cpp")
t:add_user_input_var("class_name", "MyClass")
t:add_key_value("namespace", "app")
t:add_function_var("source_files", list_sources)
t:add_post_command("clang-format -i " .. dir .. "/generated/Foo.cpp")
t:add_chained_template("another_template")
ekt.add_template("foo", t)
```

### Template methods

| Method | Signature | Description |
|---|---|---|
| `add_component` | `(input_file: string, output_file: string)` | An input template file and the output path to write. A template may have many components. Both paths may themselves contain `![[VAR]]` placeholders. |
| `add_key_value` | `(key: string, value: string)` | A fixed variable local to this template. |
| `add_user_input_var` | `(key: string, default_value: string)` | Prompt the user for this variable when the template is resolved. Pass `""` for no default. |
| `add_function_var` | `(key: string, fn: function)` | Bind a variable to a Lua function `fn(context) -> string`, evaluated at resolve time. |
| `add_post_command` | `(command: string)` | A shell command run after files are written. May contain `![[VAR]]` placeholders. |
| `add_chained_template` | `(template_name: string)` | Also resolve another registered template as part of this one. |

### Variable names are case-insensitive

All variable keys — globals, key-values, user-input vars, function-vars, and `![[...]]`
placeholders — are normalized to **UPPERCASE** internally. `namespace`, `Namespace`, and
`NAMESPACE` all refer to the same variable. Use whatever case you like in the script and in
template files; they resolve to the same slot.

---

## 5. The `Context` object

Function-vars receive a read-only `Context`. It exposes a single method:

| Method | Signature | Description |
|---|---|---|
| `get` | `(key: string) -> string \| nil` | Look up a variable (case-insensitive). Returns `nil` if the key is absent **or** its value is the empty string. |

```lua
function list_sources(context)
    local root = context:get("src_path")
    if not root then
        return ""            -- variable not set
    end
    return ekt.get_filenames(root, { ".h", ".cpp" })
end
```

A function-var **must return a string**; returning any other type is an error.

---

## 6. Template file syntax (`.ekt` files)

Input files, output paths, and post-commands are plain text with placeholders:

```
![[VARIABLE_NAME]]
```

- Delimiters are `![[` and `]]`.
- The name between the delimiters is matched case-insensitively.
- On resolution each placeholder is replaced with its variable's value. A placeholder whose
  variable resolves to nothing is replaced with an **empty string**.
- Nesting is not allowed — a `![[` appearing before the closing `]]` of an open placeholder is a
  parse error, as is an unterminated `![[`.

Example `Foo.cpp.ekt`:

```cpp
// ![[generate_warning]]
// author: ![[author]]
namespace ![[namespace]] {
    // sources:
    // ![[source_files]]
}
```

---

## 7. Resolution model

When you resolve a template (CLI: `ekt <template_name>`), EKT performs these steps for the
selected template:

1. **Seed the context** with the template's key-values, then merge in global vars.
2. **Read and parse** every component's input file and output path.
3. **Collect user-input vars** by calling the input callback (the CLI prompts on stdin, honoring
   defaults).
4. **Prompt for missing vars**: any `![[VAR]]` found in output paths, input contents, and
   post-commands that is not already in the context and is not a function-var name is requested
   via the missing-variable callback (the CLI prompts on stdin).
5. **Evaluate function-vars**: each function is called with the context; its returned string is
   stored under the (uppercased) key. Functions run *after* the prompts above, so they see all
   collected input.
6. **Resolve components**: each output path and file content is produced by substituting
   placeholders.
7. **Resolve chained templates** recursively; their outputs and post-commands are appended.
8. **Resolve post-commands** (placeholder substitution) and collect them.

The CLI then **writes** each resolved file to its output path and **runs** each post-command in
order, echoing output.

> Ordering note: because missing-variable prompting (step 4) skips names that are function-vars,
> a `![[VAR]]` backed by a function-var is filled by the function (step 5), not by a prompt.

---

## 8. Worked example

```lua
-- project.ekt.lua
local dir = ""

function internal_sources(context)
    return ekt.get_filenames(dir .. "/src", { ".h", ".cpp" })
end

function ekt.build()
    dir = ekt.get_script_dir()
    local preset = ekt.get_platform()          -- "win" | "mac" | "linux"

    ekt.add_global_var("generate_warning", "This file is generated, do not modify")
    ekt.add_global_var("author", "Erik Kirshey")

    local app_cmake = Template.new()
    app_cmake:add_component(dir .. "/templates/CMakeLists.ekt", dir .. "/CMakeLists.txt")
    app_cmake:add_function_var("internal_source_files", internal_sources)
    app_cmake:add_post_command("cmake --preset=" .. preset)
    ekt.add_template("app_cmake", app_cmake)
end
```

Resolve it:

```
ekt app_cmake
```

EKT reads `templates/CMakeLists.ekt`, substitutes `![[generate_warning]]`, `![[author]]`, and
`![[internal_source_files]]` (produced by the function-var), writes `CMakeLists.txt`, then runs
`cmake --preset=win` (or the current platform's preset).

---

## 9. Quick reference

```lua
-- Global ekt API
ekt.add_template(name, template)
ekt.add_global_var(key, value)
ekt.get_filenames(path, { ".ext", ... })   -> "line\nline\n..."
ekt.get_script_dir()                        -> "/abs/dir"
ekt.get_platform()                          -> "win" | "mac" | "linux"
function ekt.build() ... end                -- required entry point

-- Template
local t = Template.new()
t:add_component(input_file, output_file)
t:add_key_value(key, value)
t:add_user_input_var(key, default_value)
t:add_function_var(key, function(context) ... return "str" end)
t:add_post_command(command)
t:add_chained_template(template_name)

-- Context (passed to function-vars)
context:get(key)                            -> string | nil

-- Template file placeholder
![[VARIABLE_NAME]]
```

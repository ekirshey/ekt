find_package(PkgConfig REQUIRED)
pkg_check_modules(LuaJIT REQUIRED IMPORTED_TARGET luajit)
find_path(SOL2_INCLUDE_DIRS "sol/abort.hpp")

# The Lua scripting front-end for ekt. Kept as its own library, separate from
# the core template machinery in ekt_lib, so the tests can drive it too.
set(EKT_LUA_SOURCE
    lua_interface.cpp
    lua_interface.h
)

add_library(ekt_lua STATIC ${EKT_LUA_SOURCE})
add_library(ekt::ekt_lua ALIAS ekt_lua)

# Public: lua_interface.h exposes sol2 (and therefore luajit) in its headers,
# so consumers need both on their include path.
target_include_directories(ekt_lua PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${SOL2_INCLUDE_DIRS}
)

target_link_libraries(ekt_lua PUBLIC
    ekt::ekt_lib
    PkgConfig::LuaJIT
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${EKT_LUA_SOURCE})

set(EKT_APP_SOURCE
    main.cpp
    Config.cpp
    CommandLine.cpp
)

add_executable(ekt ${EKT_APP_SOURCE})

target_link_libraries(ekt PRIVATE
    ekt::ekt_lua
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${EKT_APP_SOURCE})

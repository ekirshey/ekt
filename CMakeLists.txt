cmake_minimum_required(VERSION 3.20)
project(ekt CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(LuaJIT REQUIRED IMPORTED_TARGET luajit)
find_path(SOL2_INCLUDE_DIRS "sol/abort.hpp")

add_executable(ekt
    src/main.cpp
    src/lua_interface.cpp
    src/Ekt.cpp
    src/Config.cpp
    src/Context.cpp
    src/CommandLine.cpp
    src/Template.cpp
    src/utils.cpp
    src/ParsedTemplateString.cpp
)

target_link_libraries(ekt PRIVATE
    PkgConfig::LuaJIT
)

target_include_directories(ekt PUBLIC
    ${SOL2_INCLUDE_DIRS}
)

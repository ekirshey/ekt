find_package(PkgConfig REQUIRED)
pkg_check_modules(LuaJIT REQUIRED IMPORTED_TARGET luajit)
find_path(SOL2_INCLUDE_DIRS "sol/abort.hpp")

set(EKT_LIB_SOURCE
    lua_interface.cpp
    lua_interface.h
    Ekt.cpp
    Ekt.h
    Context.cpp
    Context.h
    Template.cpp
    Template.h
    utils.cpp
    utils.h
    ParsedTemplateString.cpp
    ParsedTemplateString.h
    string_hash.h
)

add_library(ekt_lib STATIC ${EKT_LIB_SOURCE})
add_library(ekt::ekt_lib ALIAS ekt_lib)

target_compile_features(ekt_lib PUBLIC cxx_std_23)

# Public: Template.h and lua_interface.h expose sol2 (and therefore luajit)
# in the ekt_lib headers, so consumers need both on their include path.
target_include_directories(ekt_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${SOL2_INCLUDE_DIRS}
)

target_link_libraries(ekt_lib PUBLIC
    PkgConfig::LuaJIT
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${EKT_LIB_SOURCE})

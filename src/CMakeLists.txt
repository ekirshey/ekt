set(EKT_LIB_SOURCE
    Context.cpp
    Context.h
    utils.cpp
    utils.h
    ParsedTemplateString.cpp
    ParsedTemplateString.h
    string_hash.h
)

add_library(ekt_lib STATIC ${EKT_LIB_SOURCE})
add_library(ekt::ekt_lib ALIAS ekt_lib)

target_compile_features(ekt_lib PUBLIC cxx_std_23)

# The core template machinery no longer depends on Lua; the scripting front-end
# lives in the ekt_lua library alongside the app.
target_include_directories(ekt_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${EKT_LIB_SOURCE})

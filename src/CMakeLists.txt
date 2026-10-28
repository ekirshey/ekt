include(FetchContent)

# ekcore shared utilities (https://github.com/sad-brains/ekcore). Declared here
# rather than in the top level CMakeLists so consumers that FetchContent ekt
# with SOURCE_SUBDIR src still get it. SOURCE_SUBDIR src builds only the
# ekcore_lib target, skipping the repo's tests.
FetchContent_Declare(
    ekcore
    GIT_REPOSITORY https://github.com/sad-brains/ekcore.git
    GIT_TAG ab3b353328e3f2d341ba89
    SOURCE_SUBDIR src
)
FetchContent_MakeAvailable(ekcore)

set(EKT_LIB_SOURCE
    Context.cpp
    Context.h
    evaluator.cpp
    evaluator.h
    EvalNode.h
    EvalNode.cpp
    functions.cpp
    functions.h
    Scope.cpp
    Scope.h
    utils.cpp
    utils.h
    ParsedTemplateString.cpp
    ParsedTemplateString.h
    ekt_defines.h
)

add_library(ekt_lib STATIC ${EKT_LIB_SOURCE})
add_library(ekt::ekt_lib ALIAS ekt_lib)

target_compile_features(ekt_lib PUBLIC cxx_std_23)

# The core template machinery no longer depends on Lua; the scripting front-end
# lives in the ekt_lua library alongside the app.
target_include_directories(ekt_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(ekt_lib PRIVATE ekcore::ekcore_lib)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${EKT_LIB_SOURCE})

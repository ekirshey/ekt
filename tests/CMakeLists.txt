find_package(Catch2 3 REQUIRED)

set(EKT_TEST_SOURCE
    test_lua_interface.cpp
    test_template_resolution.cpp
    TempDir.h
)

add_executable(ekt_tests ${EKT_TEST_SOURCE})

target_link_libraries(ekt_tests PRIVATE
    ekt::ekt_lua
    Catch2::Catch2WithMain
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${EKT_TEST_SOURCE})

include(Catch)
catch_discover_tests(ekt_tests)

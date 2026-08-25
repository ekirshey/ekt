set(EKT_APP_SOURCE
    main.cpp
    Config.cpp
    CommandLine.cpp
)

add_executable(ekt ${EKT_APP_SOURCE})

target_link_libraries(ekt PRIVATE
    ekt::ekt_lib
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${EKT_APP_SOURCE})

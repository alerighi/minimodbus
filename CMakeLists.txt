cmake_minimum_required(VERSION 3.5)

project(minimodbus)

add_library(minimodbus STATIC minimodbus.c)
target_include_directories(minimodbus PUBLIC include/)

if (BUILD_EXAMPLE)
    add_executable(example_tcp example_tcp.c)
    target_link_libraries(example_tcp minimodbus)
endif ()

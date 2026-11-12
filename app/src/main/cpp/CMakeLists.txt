cmake_minimum_required(VERSION 3.22.1)

project(RavenaEngine)

add_library(
    ravena_renderer
    SHARED
    native-lib.cpp
)

find_library(log-lib log)

target_link_libraries(
    ravena_renderer
    GLESv3
    ${log-lib}
)

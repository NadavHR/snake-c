cmake_minimum_required(VERSION 3.7)

project(snake)

add_executable(snake main.c)

find_package(SDL2 REQUIRED)

include_directories(${SDL2_INCLUDE_DIRS})

target_link_libraries(snake ${SDL2_LIBRARIES})

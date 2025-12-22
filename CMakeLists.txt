cmake_minimum_required(VERSION 3.26)
project(main C CXX)

find_package(SDL3 REQUIRED)

add_executable(main)

target_sources(main
PRIVATE
    src/main.cpp
    src/chip8.cpp
)

target_include_directories(main 
PRIVATE 
    include
)

target_link_libraries(main SDL3::SDL3)
cmake_minimum_required(VERSION 3.10)

set(CMAKE_C_COMPILER "gcc")
set(CMAKE_CXX_COMPILER "g++")

project(BoxESP)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options(-finput-charset=UTF-8 -fexec-charset=UTF-8)
add_executable(main src/main.cpp src/basic.cpp src/triggerbot.cpp src/aimbot.cpp)

target_link_libraries(main PRIVATE gdi32)
target_link_options(main PRIVATE -s)

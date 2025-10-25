cmake_minimum_required(VERSION 3.30)
project(PP_lab3)

set(CMAKE_CXX_STANDARD 20)

add_executable(PP_lab3 main.cpp)

target_link_libraries(PP_lab3 PRIVATE winmm)

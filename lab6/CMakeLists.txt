cmake_minimum_required(VERSION 3.30)
project(lab6)

set(CMAKE_CXX_STANDARD 20)

add_executable(lab6 main.cpp)
add_executable(lab6_v2 main_v2.cpp)
add_executable(lab6_v3 main_v3.cpp)
add_executable(lab6_v4 main_v4.cpp)

target_compile_options(lab6 PRIVATE -fopenmp)
target_compile_options(lab6_v2 PRIVATE -fopenmp)
target_compile_options(lab6_v3 PRIVATE -fopenmp)
target_compile_options(lab6_v4 PRIVATE -fopenmp)

target_link_libraries(lab6 PRIVATE -fopenmp)
target_link_libraries(lab6_v2 PRIVATE -fopenmp)
target_link_libraries(lab6_v3 PRIVATE -fopenmp)
target_link_libraries(lab6_v4 PRIVATE -fopenmp)
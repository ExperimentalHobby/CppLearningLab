cmake_minimum_required(VERSION 3.20)
project(ParallelAlgorithms CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    add_compile_options(/utf-8)
endif()

add_executable(ParallelAlgorithms src/main.cpp)
target_include_directories(ParallelAlgorithms PRIVATE include)

enable_testing()
add_subdirectory(test)

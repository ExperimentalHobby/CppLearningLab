cmake_minimum_required(VERSION 3.20)
project(LockFreeStack CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    add_compile_options(/utf-8)
endif()

add_executable(LockFreeStack src/main.cpp)
target_include_directories(LockFreeStack PRIVATE include)

enable_testing()
add_subdirectory(test)

cmake_minimum_required(VERSION 3.20)
project(FuturesAndAsync CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    add_compile_options(/utf-8)
endif()

add_library(FuturesAndAsyncCore src/async_tasks.cpp)
target_include_directories(FuturesAndAsyncCore PUBLIC include)

add_executable(FuturesAndAsync src/main.cpp)
target_link_libraries(FuturesAndAsync PRIVATE FuturesAndAsyncCore)

enable_testing()
add_subdirectory(test)

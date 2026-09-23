cmake_minimum_required(VERSION 3.20)
project(ThreadPool CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    add_compile_options(/utf-8 /EHsc)
endif()

add_library(ThreadPoolCore src/thread_pool.cpp)
target_include_directories(ThreadPoolCore PUBLIC include)

add_executable(ThreadPool src/main.cpp)
target_link_libraries(ThreadPool PRIVATE ThreadPoolCore)

enable_testing()
add_subdirectory(test)

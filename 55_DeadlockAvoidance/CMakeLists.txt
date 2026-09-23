cmake_minimum_required(VERSION 3.20)
project(DeadlockAvoidance CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    add_compile_options(/utf-8)
endif()

add_library(DeadlockAvoidanceCore src/transfer.cpp)
target_include_directories(DeadlockAvoidanceCore PUBLIC include)

add_executable(DeadlockAvoidance src/main.cpp)
target_link_libraries(DeadlockAvoidance PRIVATE DeadlockAvoidanceCore)

enable_testing()
add_subdirectory(test)

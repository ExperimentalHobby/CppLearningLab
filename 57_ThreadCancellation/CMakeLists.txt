cmake_minimum_required(VERSION 3.20)
project(ThreadCancellation CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    add_compile_options(/utf-8)
endif()

add_library(ThreadCancellationCore
    src/cancellable_task.cpp
    src/timeout_runner.cpp
)
target_include_directories(ThreadCancellationCore PUBLIC include)

add_executable(ThreadCancellation src/main.cpp)
target_link_libraries(ThreadCancellation PRIVATE ThreadCancellationCore)

enable_testing()
add_subdirectory(test)

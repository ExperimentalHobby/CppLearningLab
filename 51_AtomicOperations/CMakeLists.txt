cmake_minimum_required(VERSION 3.20)
project(AtomicOperations CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    add_compile_options(/utf-8)
endif()

add_library(AtomicOperationsCore
    src/atomic_counter.cpp
    src/atomic_max.cpp
    src/spin_lock.cpp
)
target_include_directories(AtomicOperationsCore PUBLIC include)

add_executable(AtomicOperations src/main.cpp)
target_link_libraries(AtomicOperations PRIVATE AtomicOperationsCore)

enable_testing()
add_subdirectory(test)

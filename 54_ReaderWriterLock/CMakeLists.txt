cmake_minimum_required(VERSION 3.20)
project(ReaderWriterLock CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    add_compile_options(/utf-8)
endif()

add_library(ReaderWriterLockCore src/shared_cache.cpp)
target_include_directories(ReaderWriterLockCore PUBLIC include)

add_executable(ReaderWriterLock src/main.cpp)
target_link_libraries(ReaderWriterLock PRIVATE ReaderWriterLockCore)

enable_testing()
add_subdirectory(test)

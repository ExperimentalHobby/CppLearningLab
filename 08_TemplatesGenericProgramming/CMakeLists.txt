cmake_minimum_required(VERSION 3.20)

project(TemplatesGenericProgramming CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    # ソースはUTF-8で記述しているため、MSVCの既定コードページ(Shift-JIS等)による
    # 誤読を防ぐために明示する。
    add_compile_options(/utf-8)
endif()

add_executable(TemplatesGenericProgramming
    src/main.cpp
    src/display.cpp
)
target_include_directories(TemplatesGenericProgramming PRIVATE include)

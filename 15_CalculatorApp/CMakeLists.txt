cmake_minimum_required(VERSION 3.20)

project(CalculatorApp CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    # ソースはUTF-8で記述しているため、MSVCの既定コードページ(Shift-JIS等)による
    # 誤読を防ぐために明示する。
    add_compile_options(/utf-8)
endif()

add_executable(CalculatorApp WIN32
    src/main.cpp
    src/calculator.cpp
)
target_compile_definitions(CalculatorApp PRIVATE UNICODE _UNICODE)
target_include_directories(CalculatorApp PRIVATE include)

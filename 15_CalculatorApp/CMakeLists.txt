cmake_minimum_required(VERSION 3.20)

project(CalculatorApp CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    # ソースはUTF-8で記述しているため、MSVCの既定コードページ(Shift-JIS等)による
    # 誤読を防ぐために明示する。
    add_compile_options(/utf-8)
endif()

# Calculatorの計算ロジックをライブラリに分離し、本体の実行ファイルとテスト実行ファイルの
# 両方からリンクできるようにする。
add_library(CalculatorAppCore src/calculator.cpp)
target_include_directories(CalculatorAppCore PUBLIC include)

add_executable(CalculatorApp WIN32
    src/main.cpp
)
target_compile_definitions(CalculatorApp PRIVATE UNICODE _UNICODE)
target_link_libraries(CalculatorApp PRIVATE CalculatorAppCore)

enable_testing()
add_subdirectory(test)

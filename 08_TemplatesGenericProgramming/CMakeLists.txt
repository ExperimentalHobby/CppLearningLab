cmake_minimum_required(VERSION 3.20)

project(TemplatesGenericProgramming CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    # ソースはUTF-8で記述しているため、MSVCの既定コードページ(Shift-JIS等)による
    # 誤読を防ぐために明示する。
    add_compile_options(/utf-8)
endif()

# ToDisplayStringの明示的特殊化(bool/std::string版)をライブラリに分離し、本体の
# 実行ファイルとテスト実行ファイルの両方からリンクできるようにする。
# FixedStack/FixedQueue/Max/ToDisplayStringの汎用版はヘッダオンリーのテンプレートのため、
# テストファイル側でinclude/を直接インクルードするだけで検証できる。
add_library(TemplatesGenericProgrammingCore src/display.cpp)
target_include_directories(TemplatesGenericProgrammingCore PUBLIC include)

add_executable(TemplatesGenericProgramming src/main.cpp)
target_link_libraries(TemplatesGenericProgramming PRIVATE TemplatesGenericProgrammingCore)

enable_testing()
add_subdirectory(test)

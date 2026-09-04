cmake_minimum_required(VERSION 3.20)

project(FunctionsAndScope CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    # ソースはUTF-8で記述しているため、MSVCの既定コードページ(Shift-JIS等)による
    # 誤読を防ぐために明示する。
    add_compile_options(/utf-8)
endif()

# math_utilsのロジックをライブラリに分離し、本体の実行ファイルとテスト実行ファイルの
# 両方からリンクできるようにする。
add_library(FunctionsAndScopeCore src/math_utils.cpp)
target_include_directories(FunctionsAndScopeCore PUBLIC include)

add_executable(FunctionsAndScope src/main.cpp)
target_link_libraries(FunctionsAndScope PRIVATE FunctionsAndScopeCore)

enable_testing()
add_subdirectory(test)

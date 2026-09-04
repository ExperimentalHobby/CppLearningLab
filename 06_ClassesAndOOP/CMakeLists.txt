cmake_minimum_required(VERSION 3.20)

project(ClassesAndOOP CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    # ソースはUTF-8で記述しているため、MSVCの既定コードページ(Shift-JIS等)による
    # 誤読を防ぐために明示する。
    add_compile_options(/utf-8)
endif()

# 図形クラス階層をライブラリに分離し、本体の実行ファイルとテスト実行ファイルの
# 両方からリンクできるようにする。
add_library(ClassesAndOOPCore
    src/shape.cpp
    src/circle.cpp
    src/rectangle.cpp
    src/square.cpp
)
target_include_directories(ClassesAndOOPCore PUBLIC include)

add_executable(ClassesAndOOP src/main.cpp)
target_link_libraries(ClassesAndOOP PRIVATE ClassesAndOOPCore)

enable_testing()
add_subdirectory(test)

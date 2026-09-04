cmake_minimum_required(VERSION 3.20)

project(PointersAndMemory CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    # ソースはUTF-8で記述しているため、MSVCの既定コードページ(Shift-JIS等)による
    # 誤読を防ぐために明示する。
    add_compile_options(/utf-8)
endif()

# 各データ構造のロジックをライブラリに分離し、本体の実行ファイルとテスト実行ファイルの
# 両方からリンクできるようにする。
add_library(PointersAndMemoryCore
    src/linked_list.cpp
    src/tree_node.cpp
    src/scoped_resource.cpp
)
target_include_directories(PointersAndMemoryCore PUBLIC include)

add_executable(PointersAndMemory src/main.cpp)
target_link_libraries(PointersAndMemory PRIVATE PointersAndMemoryCore)

enable_testing()
add_subdirectory(test)

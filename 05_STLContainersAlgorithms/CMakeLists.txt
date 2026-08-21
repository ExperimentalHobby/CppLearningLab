cmake_minimum_required(VERSION 3.20)

project(STLContainersAlgorithms CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    # ソースはUTF-8で記述しているため、MSVCの既定コードページ(Shift-JIS等)による
    # 誤読を防ぐために明示する。
    add_compile_options(/utf-8)
endif()

add_executable(STLContainersAlgorithms
    src/main.cpp
    src/student.cpp
    src/csv_utils.cpp
    src/stats.cpp
)
target_include_directories(STLContainersAlgorithms PRIVATE include)

# 実行ディレクトリに依存せずサンプルCSVを読み込めるよう、ソースディレクトリ内の
# dataフォルダへの絶対パスをコンパイル時定数として埋め込む。
target_compile_definitions(STLContainersAlgorithms PRIVATE
    DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data"
)

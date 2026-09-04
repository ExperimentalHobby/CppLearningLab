cmake_minimum_required(VERSION 3.20)

project(STLContainersAlgorithms CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    # ソースはUTF-8で記述しているため、MSVCの既定コードページ(Shift-JIS等)による
    # 誤読を防ぐために明示する。
    add_compile_options(/utf-8)
endif()

# 学生データの読み込み・集計ロジックをライブラリに分離し、本体の実行ファイルと
# テスト実行ファイルの両方からリンクできるようにする。
add_library(STLContainersAlgorithmsCore
    src/student.cpp
    src/csv_utils.cpp
    src/stats.cpp
)
target_include_directories(STLContainersAlgorithmsCore PUBLIC include)

add_executable(STLContainersAlgorithms src/main.cpp)
target_link_libraries(STLContainersAlgorithms PRIVATE STLContainersAlgorithmsCore)

# 実行ディレクトリに依存せずサンプルCSVを読み込めるよう、ソースディレクトリ内の
# dataフォルダへの絶対パスをコンパイル時定数として埋め込む。
target_compile_definitions(STLContainersAlgorithms PRIVATE
    DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data"
)

enable_testing()
add_subdirectory(test)

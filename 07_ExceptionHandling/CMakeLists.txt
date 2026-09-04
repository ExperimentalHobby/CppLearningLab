cmake_minimum_required(VERSION 3.20)

project(ExceptionHandling CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    # ソースはUTF-8で記述しているため、MSVCの既定コードページ(Shift-JIS等)による
    # 誤読を防ぐために明示する。
    add_compile_options(/utf-8)
endif()

# 例外階層・設定ファイルパーサーをライブラリに分離し、本体の実行ファイルと
# テスト実行ファイルの両方からリンクできるようにする。
add_library(ExceptionHandlingCore
    src/app_errors.cpp
    src/scoped_trace.cpp
    src/config_parser.cpp
)
target_include_directories(ExceptionHandlingCore PUBLIC include)

add_executable(ExceptionHandling src/main.cpp)
target_link_libraries(ExceptionHandling PRIVATE ExceptionHandlingCore)

# 実行ディレクトリに依存せずサンプル設定ファイルを読み込めるよう、ソースディレクトリ内の
# dataフォルダへの絶対パスをコンパイル時定数として埋め込む。
target_compile_definitions(ExceptionHandling PRIVATE
    DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data"
)

enable_testing()
add_subdirectory(test)

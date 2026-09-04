cmake_minimum_required(VERSION 3.20)

project(SimpleTextEditor CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    # ソースはUTF-8で記述しているため、MSVCの既定コードページ(Shift-JIS等)による
    # 誤読を防ぐために明示する。
    add_compile_options(/utf-8)
endif()

# UTF-8/UTF-16変換とファイル読み書きロジックをライブラリに分離し、本体の実行ファイルと
# テスト実行ファイルの両方からリンクできるようにする。
add_library(SimpleTextEditorCore src/text_file.cpp)
target_include_directories(SimpleTextEditorCore PUBLIC include)

add_executable(SimpleTextEditor WIN32
    src/main.cpp
)
target_compile_definitions(SimpleTextEditor PRIVATE UNICODE _UNICODE)
target_link_libraries(SimpleTextEditor PRIVATE SimpleTextEditorCore)
# ファイルを開く/保存の共通ダイアログ(GetOpenFileName/GetSaveFileName)に必要。
target_link_libraries(SimpleTextEditor PRIVATE comdlg32)

enable_testing()
add_subdirectory(test)

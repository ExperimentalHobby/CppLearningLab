cmake_minimum_required(VERSION 3.20)

project(MicrocontrollerUSBComm CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    # ソースはUTF-8で記述しているため、MSVCの既定コードページ(Shift-JIS等)による
    # 誤読を防ぐために明示する。
    add_compile_options(/utf-8)
    # C++の例外(try/catch)を使うため、例外の巻き戻しセマンティクスを有効にする
    # (未指定だとC4530警告が出る)。
    add_compile_options(/EHsc)
endif()

# コマンド-応答プロトコルの組み立て・解釈ロジック(mcu_protocol)をライブラリに
# 分離し、本体の実行ファイルとテスト実行ファイルの両方からリンクできるように
# する。serial_port(実COMポート依存)も同じライブラリに含める。
add_library(MicrocontrollerUSBCommCore
    src/mcu_protocol.cpp
    src/serial_port.cpp
)
target_include_directories(MicrocontrollerUSBCommCore PUBLIC include)

add_executable(MicrocontrollerUSBComm src/main.cpp)
target_link_libraries(MicrocontrollerUSBComm PRIVATE MicrocontrollerUSBCommCore)

enable_testing()
add_subdirectory(test)

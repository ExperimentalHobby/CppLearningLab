cmake_minimum_required(VERSION 3.20)

project(HIDDataTransfer CXX)

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

# レポート解析ロジック(hid_report)とデバイス列挙・読み取り(hid_device)を
# ライブラリに分離し、本体の実行ファイルとテスト実行ファイルの両方から
# リンクできるようにする。
add_library(HIDDataTransferCore
    src/hid_report.cpp
    src/hid_device.cpp
)
target_include_directories(HIDDataTransferCore PUBLIC include)
# hid.lib/setupapi.libはいずれもWindows SDK標準のためvcpkg等の追加インストール不要。
target_link_libraries(HIDDataTransferCore PUBLIC hid setupapi)

add_executable(HIDDataTransfer src/main.cpp)
target_link_libraries(HIDDataTransfer PRIVATE HIDDataTransferCore)

enable_testing()
add_subdirectory(test)

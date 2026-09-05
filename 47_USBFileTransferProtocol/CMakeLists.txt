cmake_minimum_required(VERSION 3.20)

project(USBFileTransferProtocol CXX)

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

# プロトコルのフレーミング(binary_protocol)とファイル転送ロジック(file_transfer)を
# ライブラリに分離し、本体の実行ファイルとテスト実行ファイルの両方からリンクできる
# ようにする。
add_library(USBFileTransferProtocolCore
    src/binary_protocol.cpp
    src/file_transfer.cpp
)
target_include_directories(USBFileTransferProtocolCore PUBLIC include)
# ws2_32(htonl等)/bcrypt(SHA-256)はいずれもWindows SDK標準のため
# vcpkg等の追加インストール不要。
target_link_libraries(USBFileTransferProtocolCore PUBLIC ws2_32 bcrypt)

add_executable(USBFileTransferProtocol src/main.cpp)
target_link_libraries(USBFileTransferProtocol PRIVATE USBFileTransferProtocolCore)

enable_testing()
add_subdirectory(test)

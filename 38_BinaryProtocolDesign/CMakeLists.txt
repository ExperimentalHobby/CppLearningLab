cmake_minimum_required(VERSION 3.20)

project(BinaryProtocolDesign CXX)

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

add_library(binproto_common STATIC
    src/tcp_socket.cpp
    src/winsock_guard.cpp
    src/binary_protocol.cpp
)
target_include_directories(binproto_common PUBLIC include)
# Winsock2はWindows SDK標準のためvcpkg等の追加インストール不要。
target_link_libraries(binproto_common PUBLIC ws2_32)

add_executable(BinaryProtocolServer src/server.cpp)
target_link_libraries(BinaryProtocolServer PRIVATE binproto_common)

add_executable(BinaryProtocolClient src/client.cpp)
target_link_libraries(BinaryProtocolClient PRIVATE binproto_common)

enable_testing()
add_subdirectory(test)

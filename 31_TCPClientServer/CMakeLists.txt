cmake_minimum_required(VERSION 3.20)

project(TCPClientServer CXX)

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

add_library(tcp_common STATIC
    src/tcp_socket.cpp
    src/winsock_guard.cpp
)
target_include_directories(tcp_common PUBLIC include)
# Winsock2はWindows SDK標準のためvcpkg等の追加インストール不要。
target_link_libraries(tcp_common PUBLIC ws2_32)

add_executable(TCPEchoServer src/server.cpp)
target_link_libraries(TCPEchoServer PRIVATE tcp_common)

add_executable(TCPEchoClient src/client.cpp)
target_link_libraries(TCPEchoClient PRIVATE tcp_common)

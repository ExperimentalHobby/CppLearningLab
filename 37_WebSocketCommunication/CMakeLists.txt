cmake_minimum_required(VERSION 3.20)

project(WebSocketCommunication CXX)

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

add_library(websocket_common STATIC
    src/tcp_socket.cpp
    src/winsock_guard.cpp
    src/base64.cpp
    src/websocket.cpp
)
target_include_directories(websocket_common PUBLIC include)
# Winsock2/BCryptはいずれもWindows SDK標準のためvcpkg等の追加インストール不要。
target_link_libraries(websocket_common PUBLIC ws2_32 bcrypt)

add_executable(WebSocketServer src/server.cpp)
target_link_libraries(WebSocketServer PRIVATE websocket_common)

add_executable(WebSocketClient src/client.cpp)
target_link_libraries(WebSocketClient PRIVATE websocket_common)

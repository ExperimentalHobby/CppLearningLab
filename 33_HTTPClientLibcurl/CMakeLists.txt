cmake_minimum_required(VERSION 3.20)

project(HTTPClient CXX)

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

add_executable(HTTPClient
    src/main.cpp
    src/http_client.cpp
    src/json_value.cpp
)
target_include_directories(HTTPClient PRIVATE include)
# WinHTTPはWindows SDK標準のためvcpkg等の追加インストール不要。
target_link_libraries(HTTPClient PRIVATE winhttp)

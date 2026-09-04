cmake_minimum_required(VERSION 3.20)

project(MySQLConnection CXX)

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

add_executable(MySQLConnection
    src/main.cpp
    src/connection_string.cpp
    src/odbc_connection.cpp
)
target_include_directories(MySQLConnection PRIVATE include)

# ODBCはWindows SDK標準のためvcpkg等の追加インストール不要。
# MySQL/PostgreSQLいずれも公式ODBCドライバを提供しており、同じAPIで接続できる。
target_link_libraries(MySQLConnection PRIVATE odbc32 odbccp32)

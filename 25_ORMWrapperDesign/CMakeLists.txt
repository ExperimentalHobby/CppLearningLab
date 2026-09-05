cmake_minimum_required(VERSION 3.20)

project(ORMWrapperDesign CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    # ソースはUTF-8で記述しているため、MSVCの既定コードページ(Shift-JIS等)による
    # 誤読を防ぐために明示する。
    add_compile_options(/utf-8)
    # C++の例外(try/catch)を使うため、例外の巻き戻しセマンティクスを有効にする
    # (未指定だとC4530警告が出る)。C言語のsqlite3には適用されないようCXX限定にする。
    add_compile_options($<$<COMPILE_LANGUAGE:CXX>:/EHsc>)
endif()

# SQLiteはvcpkg等を使わず、third_party/sqlite3にvendoringしたamalgamationを
# 直接ビルドして使う（詳細はthird_party/sqlite3/NOTICE.mdを参照）。
set(SQLITE3_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../third_party/sqlite3)
add_library(sqlite3 STATIC ${SQLITE3_DIR}/sqlite3.c)
target_include_directories(sqlite3 PUBLIC ${SQLITE3_DIR})
if(MSVC)
    target_compile_options(sqlite3 PRIVATE /utf-8)
endif()

# Repository<T, Mapper>やUser/Productはヘッダオンリー(実装を持つ.cppが無い)ため、
# INTERFACEライブラリとしてinclude/とsqlite3の依存だけをまとめ、本体の実行ファイルと
# テスト実行ファイルの両方から同じ設定でインクルードできるようにする。
add_library(ORMWrapperDesignCore INTERFACE)
target_include_directories(ORMWrapperDesignCore INTERFACE include)
target_link_libraries(ORMWrapperDesignCore INTERFACE sqlite3)

add_executable(ORMWrapperDesign
    src/main.cpp
)
target_link_libraries(ORMWrapperDesign PRIVATE ORMWrapperDesignCore)

enable_testing()
add_subdirectory(test)

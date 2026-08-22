cmake_minimum_required(VERSION 3.20)

project(HelloWindow CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    # ソースはUTF-8で記述しているため、MSVCの既定コードページ(Shift-JIS等)による
    # 誤読を防ぐために明示する。
    add_compile_options(/utf-8)
endif()

# WIN32を付けることで、コンソールを伴わないGUIサブシステムの実行ファイルとしてビルドする
# （エントリポイントはmain()ではなくWinMain()になる）。
add_executable(HelloWindow WIN32
    src/main.cpp
)
target_compile_definitions(HelloWindow PRIVATE UNICODE _UNICODE)

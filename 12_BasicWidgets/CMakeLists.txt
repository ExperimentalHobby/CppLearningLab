cmake_minimum_required(VERSION 3.20)

project(BasicWidgets CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    # ソースはUTF-8で記述しているため、MSVCの既定コードページ(Shift-JIS等)による
    # 誤読を防ぐために明示する。
    add_compile_options(/utf-8)
endif()

add_executable(BasicWidgets WIN32
    src/main.cpp
)
target_compile_definitions(BasicWidgets PRIVATE UNICODE _UNICODE)
# ツールバー/ステータスバーはコモンコントロールなので comctl32 が必要。
target_link_libraries(BasicWidgets PRIVATE comctl32)

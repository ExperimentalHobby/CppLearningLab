cmake_minimum_required(VERSION 3.20)

project(PaintApp CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    # ソースはUTF-8で記述しているため、MSVCの既定コードページ(Shift-JIS等)による
    # 誤読を防ぐために明示する。
    add_compile_options(/utf-8)
endif()

# windows.h(wingdi.h)のGDI関数(Rectangle/Ellipse等)との名前衝突は使っていないが、
# 統一のため他の課題と同様にNOGDIは付けない(このアプリはGDIを積極的に使うため)。
add_executable(PaintApp WIN32
    src/main.cpp
)
target_compile_definitions(PaintApp PRIVATE UNICODE _UNICODE)

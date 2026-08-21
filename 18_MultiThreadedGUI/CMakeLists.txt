cmake_minimum_required(VERSION 3.20)

project(MultiThreadedGUI CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    # ソースはUTF-8で記述しているため、MSVCの既定コードページ(Shift-JIS等)による
    # 誤読を防ぐために明示する。
    add_compile_options(/utf-8)
endif()

add_executable(MultiThreadedGUI WIN32
    src/main.cpp
)
target_compile_definitions(MultiThreadedGUI PRIVATE UNICODE _UNICODE)
# プログレスバー(コモンコントロール)用。
target_link_libraries(MultiThreadedGUI PRIVATE comctl32)

find_package(Threads REQUIRED)
target_link_libraries(MultiThreadedGUI PRIVATE Threads::Threads)

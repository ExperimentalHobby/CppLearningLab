cmake_minimum_required(VERSION 3.20)

project(EventHandling CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    # ソースはUTF-8で記述しているため、MSVCの既定コードページ(Shift-JIS等)による
    # 誤読を防ぐために明示する。
    add_compile_options(/utf-8)
endif()

# 反映結果・反映回数の表示文字列組み立てロジックをライブラリに分離し、本体の実行ファイルと
# テスト実行ファイルの両方からリンクできるようにする。
add_library(EventHandlingCore src/reflect_format.cpp)
target_include_directories(EventHandlingCore PUBLIC include)

add_executable(EventHandling WIN32
    src/main.cpp
)
target_compile_definitions(EventHandling PRIVATE UNICODE _UNICODE)
target_link_libraries(EventHandling PRIVATE EventHandlingCore)

enable_testing()
add_subdirectory(test)

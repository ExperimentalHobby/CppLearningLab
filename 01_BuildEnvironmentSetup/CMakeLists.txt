cmake_minimum_required(VERSION 3.20)

project(BuildEnvironmentSetup CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    # ソースはUTF-8で記述しているため、MSVCの既定コードページ(Shift-JIS等)による
    # 誤読を防ぐために明示する。
    add_compile_options(/utf-8)
endif()

# main()から呼び出す挨拶メッセージ生成ロジックをライブラリに分離し、
# 本体の実行ファイルとテスト実行ファイルの両方からリンクできるようにする。
add_library(BuildEnvironmentSetupCore src/greeting.cpp)
target_include_directories(BuildEnvironmentSetupCore PUBLIC include)

add_executable(BuildEnvironmentSetup src/main.cpp)
target_link_libraries(BuildEnvironmentSetup PRIVATE BuildEnvironmentSetupCore)

enable_testing()
add_subdirectory(test)

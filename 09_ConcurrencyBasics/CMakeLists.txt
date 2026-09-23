cmake_minimum_required(VERSION 3.20)

project(ConcurrencyBasics CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    # ソースはUTF-8で記述しているため、MSVCの既定コードページ(Shift-JIS等)による
    # 誤読を防ぐために明示する。
    add_compile_options(/utf-8)
endif()

# ThreadSafeCounterをライブラリに分離し、本体の実行ファイルとテスト実行ファイルの
# 両方からリンクできるようにする。UnsafeCounter/BlockingQueue<T>はヘッダーオンリーの
# ためinclude/を直接インクルードするだけで使える。
add_library(ConcurrencyBasicsCore src/thread_safe_counter.cpp)
target_include_directories(ConcurrencyBasicsCore PUBLIC include)

add_executable(ConcurrencyBasics src/main.cpp)
target_link_libraries(ConcurrencyBasics PRIVATE ConcurrencyBasicsCore)

enable_testing()
add_subdirectory(test)

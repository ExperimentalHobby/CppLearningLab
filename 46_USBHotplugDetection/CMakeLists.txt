cmake_minimum_required(VERSION 3.20)

project(USBHotplugDetection CXX)

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

# VID/PIDパースロジック(usb_device_info、42番と共通)をライブラリに分離し、
# 本体の実行ファイルとテスト実行ファイルの両方からリンクできるようにする。
add_library(USBHotplugDetectionCore src/usb_device_info.cpp)
target_include_directories(USBHotplugDetectionCore PUBLIC include)

add_executable(USBHotplugDetection WIN32 src/main.cpp)
target_compile_definitions(USBHotplugDetection PRIVATE UNICODE _UNICODE)
target_link_libraries(USBHotplugDetection PRIVATE USBHotplugDetectionCore comctl32)

enable_testing()
add_subdirectory(test)

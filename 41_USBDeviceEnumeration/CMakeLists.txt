cmake_minimum_required(VERSION 3.20)

project(USBDeviceEnumeration CXX)

# usb_device.cpp/main.cppがSetupAPI(windows.h)に直接依存しており非Windows環境では
# 必ずビルド不能なため、原因不明のビルドエラーになる前にconfigure時点で
# 明示的に失敗させる。
if(NOT WIN32)
    message(FATAL_ERROR "41_USBDeviceEnumerationはWindows標準API(SetupAPI)に依存しており、Windows専用です。")
endif()

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

add_executable(USBDeviceEnumeration
    src/main.cpp
    src/usb_device.cpp
)
target_include_directories(USBDeviceEnumeration PRIVATE include)
# SetupAPIはWindows SDK標準のためvcpkg等の追加インストール不要。
target_link_libraries(USBDeviceEnumeration PRIVATE setupapi)

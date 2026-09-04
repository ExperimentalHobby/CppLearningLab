cmake_minimum_required(VERSION 3.20)

project(VariablesAndControlFlow CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    # ソースはUTF-8で記述しているため、MSVCの既定コードページ(Shift-JIS等)による
    # 誤読を防ぐために明示する。
    add_compile_options(/utf-8)
endif()

# 各プログラムの判定ロジックをライブラリに分離し、本体の実行ファイルと
# テスト実行ファイルの両方からリンクできるようにする。
add_library(FizzBuzzCore src/fizzbuzz.cpp)
target_include_directories(FizzBuzzCore PUBLIC include)

add_library(GradeJudgmentCore src/grade_judgment.cpp)
target_include_directories(GradeJudgmentCore PUBLIC include)

add_library(NumberGuessingCore src/number_guessing.cpp)
target_include_directories(NumberGuessingCore PUBLIC include)

add_executable(FizzBuzz src/fizzbuzz_main.cpp)
target_link_libraries(FizzBuzz PRIVATE FizzBuzzCore)

add_executable(GradeJudgment src/grade_judgment_main.cpp)
target_link_libraries(GradeJudgment PRIVATE GradeJudgmentCore)

add_executable(NumberGuessingGame src/number_guessing_main.cpp)
target_link_libraries(NumberGuessingGame PRIVATE NumberGuessingCore)

enable_testing()
add_subdirectory(test)

cmake_minimum_required(VERSION 3.20)

project(VariablesAndControlFlow CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    # ソースはUTF-8で記述しているため、MSVCの既定コードページ(Shift-JIS等)による
    # 誤読を防ぐために明示する。
    add_compile_options(/utf-8)
endif()

add_executable(FizzBuzz src/fizzbuzz.cpp)
add_executable(GradeJudgment src/grade_judgment.cpp)
add_executable(NumberGuessingGame src/number_guessing_game.cpp)

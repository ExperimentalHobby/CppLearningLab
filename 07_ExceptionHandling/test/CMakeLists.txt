set(GOOGLETEST_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../third_party/googletest)

add_library(gtest STATIC ${GOOGLETEST_DIR}/src/gtest-all.cc)
target_include_directories(gtest PUBLIC ${GOOGLETEST_DIR}/include)
target_include_directories(gtest PRIVATE ${GOOGLETEST_DIR})

add_library(gtest_main STATIC ${GOOGLETEST_DIR}/src/gtest_main.cc)
target_link_libraries(gtest_main PUBLIC gtest)

add_executable(ExceptionHandlingTests
    app_errors_test.cpp
    config_parser_test.cpp
    scoped_trace_test.cpp
)
target_link_libraries(ExceptionHandlingTests PRIVATE ExceptionHandlingCore gtest_main)

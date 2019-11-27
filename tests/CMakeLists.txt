include(CTest)

add_executable (Test unconstexpr.cpp)
target_link_libraries(Test unconstexpr)

add_test (NAME MyTest COMMAND Test)
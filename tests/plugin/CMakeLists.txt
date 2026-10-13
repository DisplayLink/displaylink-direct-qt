set(TESTS_EXE_NAME "qdlsdk-tests-plugin")

add_executable(${TESTS_EXE_NAME}
    dirty_rects_test.cpp
)

target_link_libraries(${TESTS_EXE_NAME} PRIVATE
    qdlsdk-plugin-impl
    GTest::gmock_main
)

include(GoogleTest)
gtest_discover_tests(${TESTS_EXE_NAME})

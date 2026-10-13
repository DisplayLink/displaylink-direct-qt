set(TESTS_EXE_NAME "qdlsdk-tests-core")

add_executable(${TESTS_EXE_NAME}
	display_manager_test.cpp
	single_display_controller_test.cpp
)

target_link_libraries(${TESTS_EXE_NAME} PRIVATE
	qdlsdk-core
	GTest::gmock_main
)

include(GoogleTest)
gtest_discover_tests(${TESTS_EXE_NAME})

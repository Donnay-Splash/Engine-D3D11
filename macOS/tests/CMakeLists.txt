add_executable(engine-lifecycle-tests engine_lifecycle_tests.cpp)
target_link_libraries(engine-lifecycle-tests PRIVATE engine-core)
add_test(NAME engine-lifecycle COMMAND engine-lifecycle-tests)

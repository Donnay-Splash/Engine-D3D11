add_executable(engine-lifecycle-tests engine_lifecycle_tests.cpp)
target_link_libraries(engine-lifecycle-tests PRIVATE engine)
add_test(NAME engine-lifecycle COMMAND engine-lifecycle-tests)

add_executable(portable-core-tests portable_core_tests.cpp)
target_link_libraries(portable-core-tests PRIVATE engine)
add_test(NAME portable-core COMMAND portable-core-tests)

add_test(
    NAME portable-core-independent-build
    COMMAND ${CMAKE_COMMAND} --build ${PROJECT_BINARY_DIR} --target engine
)
set_tests_properties(portable-core-independent-build PROPERTIES RUN_SERIAL TRUE)

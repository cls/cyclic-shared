find_package(Boost
    REQUIRED
    COMPONENTS
        unit_test_framework
)

file(GLOB UNIT_TEST_SOURCE_FILES "test_*.cpp")

add_executable(tests
    main.cpp
    ${UNIT_TEST_SOURCE_FILES}
)

target_include_directories(tests
    PRIVATE
        Boost::unit_test_framework
)

target_link_libraries(tests
    PRIVATE
        Boost::unit_test_framework
        cyclic_shared
)

add_test(NAME test_shared_ptr
    COMMAND $<TARGET_FILE:tests>
)

add_executable(basic_test
    test.cpp
)

target_link_libraries(basic_test
    PRIVATE cyclic_shared
)

add_test(NAME basic_test
    COMMAND $<TARGET_FILE:basic_test>
)

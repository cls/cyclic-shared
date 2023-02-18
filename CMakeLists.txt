cmake_minimum_required(VERSION 3.7)

project(cyclic_shared
    LANGUAGES CXX
)

include_directories(include)

add_subdirectory(src)

if(BUILD_TESTING)
    # When running CTest with --test-action memcheck, a leak means failure.
    set(MEMORYCHECK_COMMAND_OPTIONS "--error-exitcode=3 --leak-check=full")
    include(CTest)

    add_subdirectory(tests)
endif()

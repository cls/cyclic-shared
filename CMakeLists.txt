cmake_minimum_required(VERSION 3.7)

project(cyclic_shared
    LANGUAGES CXX
)

if(BUILD_TESTING)
    # When running CTest with --test-action memcheck, a leak means failure.
    set(MEMORYCHECK_COMMAND_OPTIONS "--error-exitcode=3 --leak-check=full")
    include(CTest)
endif()

include_directories(include)

add_subdirectory(lib)
add_subdirectory(tests)

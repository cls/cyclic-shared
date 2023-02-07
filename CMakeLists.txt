cmake_minimum_required(VERSION 3.7)

project(cyclic_shared
    LANGUAGES CXX
)

if(BUILD_TESTING)
    enable_testing()
endif()

include_directories(include)

add_subdirectory(lib)
add_subdirectory(tests)

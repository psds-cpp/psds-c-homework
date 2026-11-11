cmake_minimum_required(VERSION 3.22)
project(cstack)
enable_testing()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(WITH_TEST "Build test (GTest library required)" OFF)

# build library
add_library(cstack STATIC cstack.c)

# build test
if (WITH_TEST)
    enable_testing()
    add_executable(cstack_test test.cpp)
    target_link_libraries(cstack_test cstack)
    add_test(NAME cstack_test COMMAND cstack_test)

    # link with googletest
    find_package(GTest)
    if (NOT ${GTest_FOUND})
        include(cmake/gtest.cmake)
    endif()
    target_link_libraries(cstack_test
        GTest::gtest
        GTest::gtest_main
        GTest::gmock
        GTest::gmock_main
    )
endif()

# compiler-specific project settings
if (MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    set(MSVC_FLAGS /Wall /WX)
    target_compile_options(cstack PRIVATE ${MSVC_FLAGS})
    target_link_options(cstack PRIVATE ${MSVC_FLAGS})
    if (WITH_TEST)
        target_compile_options(cstack PRIVATE /fsanitize=address)
        target_compile_definitions(cstack PRIVATE _DISABLE_VECTOR_ANNOTATION)
        target_compile_options(cstack_test PRIVATE /fsanitize=address)
        target_compile_definitions(cstack_test PRIVATE _DISABLE_VECTOR_ANNOTATION)
    endif()
else()
    target_compile_options(cstack PRIVATE -Wall -Wextra -Wformat -Wformat-security -Werror)
    set(SANITIZERS_FLAGS -fsanitize=undefined -fsanitize=address)
    if (WITH_TEST)
        target_compile_options(cstack PRIVATE ${SANITIZERS_FLAGS} -g3 -O0)
        target_compile_options(cstack_test PRIVATE ${SANITIZERS_FLAGS} -g3 -O0)
        target_link_options(cstack_test PRIVATE ${SANITIZERS_FLAGS})
    endif()
endif()

cmake_minimum_required(VERSION 3.16)
project(poker_bot LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(POKER_BUILD_TESTS "Build unit tests" ON)
option(POKER_BUILD_EXAMPLES "Build example binaries" ON)

add_library(poker_lib
    src/card.cpp
    src/deck.cpp
    src/game_engine.cpp
    src/hand_evaluator.cpp
    src/strategy.cpp
    src/opponent_model.cpp
    src/monte_carlo.cpp
    src/bot_config.cpp
    src/poker_bot_interface.cpp
)
target_include_directories(poker_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_compile_definitions(poker_lib PUBLIC $<$<BOOL:${WIN32}>:NOMINMAX>)

if(POKER_BUILD_EXAMPLES)
    add_executable(poker_example examples/simple_bot_main.cpp)
    target_link_libraries(poker_example PRIVATE poker_lib)
endif()

if(POKER_BUILD_TESTS)
    include(FetchContent)
    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
    )
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
    enable_testing()
    add_executable(poker_tests
        tests/test_deck.cpp
        tests/test_game_engine.cpp
        tests/test_hand_evaluator.cpp
        tests/test_strategy.cpp
        tests/test_opponent_model.cpp
        tests/test_monte_carlo.cpp
        tests/test_bot_config.cpp
    )
    target_link_libraries(poker_tests PRIVATE poker_lib GTest::gtest_main)
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        target_link_libraries(poker_tests PRIVATE stdc++fs)
    endif()
    include(GoogleTest)
    gtest_discover_tests(poker_tests)
endif()

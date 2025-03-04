# Tests CMakeLists.txt
cmake_minimum_required(VERSION 3.10)

# Enable testing first
enable_testing()

# Find Google Test or fetch it if not found
find_package(GTest QUIET)
if(NOT GTest_FOUND)
    include(FetchContent)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG release-1.12.1
    )
    FetchContent_MakeAvailable(googletest)
endif()

# Board tests
add_executable(board_tests board_tests.cpp ../src/board.cpp)
target_include_directories(board_tests PRIVATE ../src)
target_link_libraries(board_tests PRIVATE GTest::gtest GTest::gtest_main)
add_test(NAME board_tests COMMAND board_tests)

# Game tests
add_executable(game_tests
    game_tests.cpp
    ../src/board.cpp
    ../src/game.cpp
)
target_include_directories(game_tests PRIVATE ../src)
target_link_libraries(game_tests PRIVATE GTest::gtest GTest::gtest_main)
add_test(NAME game_tests COMMAND game_tests)

# Enable test discovery
include(GoogleTest)
gtest_discover_tests(board_tests)
gtest_discover_tests(game_tests)
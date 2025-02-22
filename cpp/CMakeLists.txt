cmake_minimum_required(VERSION 3.10)
project(2048 VERSION 1.0)

# Specify C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add compiler warnings
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Source files
set(SOURCES
    main.cpp
    board.cpp
    game.cpp
    heuristic_player.cpp
    mcts_player.cpp
    random_player.cpp
)

# Header files
set(HEADERS
    board.hpp
    game.hpp
    player.hpp
)

# Create executable
add_executable(2048 ${SOURCES} ${HEADERS})

# Include directories
target_include_directories(2048 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Optional: Enable testing
option(BUILD_TESTS "Build the tests" OFF)
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# Optional: Install rules
install(TARGETS 2048
    RUNTIME DESTINATION bin
)

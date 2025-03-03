cmake_minimum_required(VERSION 3.10)
project(2048 VERSION 1.0)

# Set default build type if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Choose the type of build (Debug or Release)" FORCE)
endif()

# Set optimization flags for Release builds
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    if(MSVC)
        add_compile_options(/O2)
    else()
        add_compile_options(-O3)
    endif()
    # Define NDEBUG to disable assertions in Release mode
    add_definitions(-DNDEBUG)
else()
    # Debug-specific flags
    if(MSVC)
        # No specific debug flags needed for MSVC
    else()
        add_compile_options(-O0 -g)
    endif()
endif()

# Specify C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add compiler warnings
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Source files for the main game
set(SOURCES
    src/main.cpp
    src/board.cpp
    src/game.cpp
    src/random_player.cpp
    src/heuristic_player.cpp
    src/mcts_player.cpp
)

# Header files
set(HEADERS
    src/board.hpp
    src/game.hpp
    src/player.hpp
)

# Create main game executable
add_executable(2048 ${SOURCES} ${HEADERS})

# Include directories
target_include_directories(2048 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

# Source files for the heuristic tuning program
set(TUNE_SOURCES
    src/tune_heuristic.cpp
    src/board.cpp
    src/game.cpp
    src/heuristic_player.cpp
    src/random_player.cpp
)

# Create heuristic tuning executable
add_executable(tune_heuristic ${TUNE_SOURCES} ${HEADERS})

# Include directories for tuning program
target_include_directories(tune_heuristic PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

# Optional: Enable testing
option(BUILD_TESTS "Build tests" ON)

# Installation rules
install(TARGETS 2048 tune_heuristic
    RUNTIME DESTINATION bin
)

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

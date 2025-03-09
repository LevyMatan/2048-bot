cmake_minimum_required(VERSION 3.10)
project(2048 CXX)

# Enable testing at the top level
enable_testing()

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(APPLE)
    # Get the macOS SDK path dynamically
    execute_process(
        COMMAND xcrun --show-sdk-path
        OUTPUT_VARIABLE MACOS_SDK_PATH
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    # Construct the full path to the libc++ headers
    set(LIBCXX_INCLUDE_PATH "${MACOS_SDK_PATH}/usr/include/c++/v1")

    # Pass the flags to the compiler
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -stdlib=libc++ -isystem ${LIBCXX_INCLUDE_PATH}")
elseif(MSVC)
    # Use MSVC-specific warning flags
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
endif()


# Set optimization flags for different build types
if(MSVC)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2 /DNDEBUG")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /Od /Zi")
else()
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -g")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -march=native -flto -DNDEBUG")
endif()

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/src)

# Main executable source files
set(MAIN_SOURCES
    src/main.cpp
    src/board.cpp
    src/game.cpp
    src/random_player.cpp
    src/heuristic_player.cpp
    src/expectimax_player.cpp
    src/evaluation.cpp
    src/arg_parser.cpp
    src/logger.cpp
)

# Create main executable
add_executable(2048 ${MAIN_SOURCES})

# Tuning executable source files
set(TUNE_SOURCES
    src/tune_heuristic.cpp
    src/board.cpp
    src/game.cpp
    src/heuristic_player.cpp
    src/expectimax_player.cpp
    src/evaluation.cpp
    src/logger.cpp
)

# Create tuning executable
add_executable(tune_heuristic ${TUNE_SOURCES})

# Find Google Test package
find_package(GTest QUIET)
if(NOT GTest_FOUND)
    # If not found, fetch and build it
    include(FetchContent)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG release-1.12.1
    )
    # For Windows: Prevent overriding the parent project's compiler/linker settings
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
endif()

# Enable testing
include(GoogleTest)

add_subdirectory(tests)

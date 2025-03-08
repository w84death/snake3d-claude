cmake_minimum_required(VERSION 3.10)
project(SnakeAI)

set(CMAKE_CXX_STANDARD 14)

# Raylib setup options
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE) # don't build the supplied examples
set(BUILD_GAMES    OFF CACHE BOOL "" FORCE) # don't build the supplied example games

# Find raylib package or build it
find_package(raylib 4.0 QUIET)
if (NOT raylib_FOUND)
    include(FetchContent)
    FetchContent_Declare(
        raylib
        GIT_REPOSITORY https://github.com/raysan5/raylib.git
        GIT_TAG 4.5.0
    )
    FetchContent_MakeAvailable(raylib)
endif()

# Add source files
set(SOURCES
    main.cpp
    snake.cpp
    game.cpp
    camera_controller.cpp
)

# Create executable
add_executable(snake_game ${SOURCES})
target_link_libraries(snake_game raylib)

# Create resources directory
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/resources)

# Create empty placeholder textures
file(WRITE ${CMAKE_BINARY_DIR}/resources/snake_texture.png "")
file(WRITE ${CMAKE_BINARY_DIR}/resources/apple_texture.png "")

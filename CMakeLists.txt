cmake_minimum_required(VERSION 3.10)

# Set the project name
project(StupidTetrisBullshit)

# Set debug flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0 -Wall -Wextra -Wpedantic")

# Find the SDL2 library
find_package(SDL2 REQUIRED)

# Add the executable
add_executable(game_main src/game_main.cpp src/Stacker.cpp)

# Add the executable for the tests
add_executable(tests src/tests.cpp)

# Link the SDL2 library
target_link_libraries(game_main SDL2::SDL2)
target_link_libraries(tests SDL2::SDL2)
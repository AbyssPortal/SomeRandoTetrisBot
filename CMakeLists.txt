cmake_minimum_required(VERSION 3.10)

# Set the project name
project(StupidTetrisBullshit)

# Set debug flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0 -Wall -Wextra -Wpedantic -Wreturn-type")

# Set the module path to the directory containing FindSDL2TTF.cmake
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

# Find the SDL2 and SDL2_ttf libraries
find_package(SDL2 REQUIRED)

# Include SDL2 and SDL2_ttf directories
include_directories(${SDL2_INCLUDE_DIRS})

# Add the executable
add_executable(game_main src/game_main.cpp src/Stacker.cpp src/StackerBot.cpp)

# Add the executable for the tests
# add_executable(tests src/tests.cpp)

# Link the SDL2 and SDL2_ttf libraries
target_link_libraries(game_main ${SDL2_LIBRARIES})
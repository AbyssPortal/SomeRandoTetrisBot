cmake_minimum_required(VERSION 3.10)

# Set the project name
project(StupidTetrisBullshit)

# Set debug and release flags
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0 -Wall -Wextra -Wpedantic -Wreturn-type")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -Wall -Wextra -Wpedantic -Wreturn-type")

# Set the module path to the directory containing FindSDL2TTF.cmake
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

find_library(SDL_MIXER_LIBRARY
  NAMES SDL2_mixer
  HINTS
    ENV SDLMIXERDIR
    ENV SDLDIR
  PATH_SUFFIXES lib
)

# Find the SDL2 and SDL2_mixer libraries
find_package(SDL2 REQUIRED)
find_package(SDL2_mixer REQUIRED)



# Include SDL2 and SDL2_mixer directories
include_directories(${SDL2_INCLUDE_DIRS} ${SDL2_MIXER_INCLUDE_DIRS})

# Add the executable for the debug build
add_executable(game_main_debug src/game_main.cpp src/Stacker.cpp src/StackerBot.cpp)
set_target_properties(game_main_debug PROPERTIES COMPILE_FLAGS "${CMAKE_CXX_FLAGS_DEBUG}")
target_link_libraries(game_main_debug ${SDL2_LIBRARIES} ${SDL2_MIXER_LIBRARIES})

# Add the executable for the release build
add_executable(game_main_release src/game_main.cpp src/Stacker.cpp src/StackerBot.cpp)
set_target_properties(game_main_release PROPERTIES COMPILE_FLAGS "${CMAKE_CXX_FLAGS_RELEASE}")
target_link_libraries(game_main_release ${SDL2_LIBRARIES} ${SDL2_MIXER_LIBRARIES})

# Add the executable for the tests
# add_executable(tests src/tests.cpp)

add_executable(evolution_main src/evolution_main.cpp src/Stacker.cpp src/StackerBot.cpp)
set_target_properties(evolution_main PROPERTIES COMPILE_FLAGS "${CMAKE_CXX_FLAGS_RELEASE}")


add_executable(evolution_main_debug src/evolution_main.cpp src/Stacker.cpp src/StackerBot.cpp)
set_target_properties(evolution_main_debug PROPERTIES COMPILE_FLAGS "${CMAKE_CXX_FLAGS_DEBUG}")

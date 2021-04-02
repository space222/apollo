cmake_minimum_required(VERSION 3.10)

project(agc)

find_package(SDL2 REQUIRED)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS "-O2 -Wall -Wpedantic -Wextra -Wimplicit-fallthrough")

add_executable(agc main.cpp cpu.cpp mem.cpp)

target_include_directories(agc PUBLIC ${SDL2_INCLUDE_DIRS} )

target_link_libraries(agc PUBLIC dl GL GLEW ${SDL2_LIBRARIES})




cmake_minimum_required(VERSION 3.1)

project(spacesim)

set(CMAKE_CXX_STANDARD 17)

find_package(Curses REQUIRED)
include_directories(${CURSES_INCLUDE_DIR})

file(GLOB SOURCES "*.cpp")
add_executable(spacesim main.cpp)

target_link_libraries(spacesim ${CURSES_LIBRARIES} pthread)


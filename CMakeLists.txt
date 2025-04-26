cmake_minimum_required(VERSION 3.16)
project(project_2)

set(CMAKE_CXX_STANDARD 20)


# set C++ Version & executable output path
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

# make your compiler aware of header directory
include_directories(${PROJECT_SOURCE_DIR}/include)

# find & include ncurses library
find_package(Curses REQUIRED)
include_directories(${CURSES_INCLUDE_DIR})

# create a variable which includes needed source files
set(MY_SOURCES
        ${PROJECT_SOURCE_DIR}/src/main.cpp
)

# specify an executable,
# build from the specified source files
add_executable(minivim ${MY_SOURCES})

# link ncurses library with your executable
target_link_libraries(minivim ${CURSES_LIBRARY})
cmake_minimum_required(VERSION 3.13)

# set the project name
project(DSPSimulator)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

option(PDEBUG "Activate debug logs and store variables in csv files" OFF)

# file(COPY resources DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(MAKE_DIRECTORY ${RESOURCES_DIR}/debugData)


add_subdirectory(src)
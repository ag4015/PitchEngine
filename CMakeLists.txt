cmake_minimum_required(VERSION 3.13)

# set the project name
project(DSPSim)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

set(RESOURCES_DIR  ${PROJECT_SOURCE_DIR}/resources  CACHE PATH "This is the resources directory")
file(MAKE_DIRECTORY ${RESOURCES_DIR}/debugData)
file(MAKE_DIRECTORY ${RESOURCES_DIR}/outputAudio)

macro(SET_OPTION option value)
  set(${option} ${value} CACHE "" INTERNAL FORCE)
endmacro()

add_subdirectory(src)
add_subdirectory(tools)

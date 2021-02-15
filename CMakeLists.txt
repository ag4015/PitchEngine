cmake_minimum_required(VERSION 3.13)

# set the project name
project(DSPSim)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(RESOURCES_DIR  ${PROJECT_SOURCE_DIR}/resources/  CACHE PATH "This is the resources directory")
# SET(PDEBUG   "-pDebug")

# file(COPY resources DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(MAKE_DIRECTORY ${RESOURCES_DIR}/debugData)
file(MAKE_DIRECTORY ${RESOURCES_DIR}/outputAudio)


add_subdirectory(src)
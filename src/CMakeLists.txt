
cmake_minimum_required(VERSION 3.13)

# set the project name
project(DSPSimulator VERSION 1.0)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

option(PDEBUG "Activate debug logs and store variables in csv files" OFF)

configure_file(DSPConfig.h.in DSPConfig.h)

add_subdirectory(kissfft)

file(GLOB_RECURSE data resources/*)

# add the executable
add_executable(DSPSimulator
               main.c
               wavio.c
               audioUtils.cpp
               ${data}
               )

target_compile_options(DSPSimulator PUBLIC -Wall)

target_include_directories(DSPSimulator PUBLIC
                           src/
                           )

target_link_libraries(DSPSimulator PUBLIC kissfft)

install(TARGETS DSPSimulator DESTINATION bin)
install(DIRECTORY resources DESTINATION bin)
install(FILES "${PROJECT_BINARY_DIR}/DSPSimulatorConfig.h"
  DESTINATION include
  )
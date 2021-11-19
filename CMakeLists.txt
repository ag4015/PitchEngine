cmake_minimum_required(VERSION 3.15)

project(DSPSim)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

add_library(DSPSim "")

set(DSPSIM_DIR ${CMAKE_CURRENT_SOURCE_DIR}/)
set(DATA_DIR ${PROJECT_SOURCE_DIR}/data/)
set(INPUT_AUDIO_DIR ${DATA_DIR}inputAudio/)
set(OUTPUT_AUDIO_DIR ${DATA_DIR}outputAudio/)
set(DEBUG_DIR ${DATA_DIR}debugData/)

file(MAKE_DIRECTORY ${DEBUG_DIR})
file(MAKE_DIRECTORY ${OUTPUT_AUDIO_DIR})

target_compile_definitions(DSPSim PUBLIC "DATA_DIR=\"${DATA_DIR}\"")
target_compile_definitions(DSPSim PUBLIC "INPUT_AUDIO_DIR=\"${INPUT_AUDIO_DIR}\"")
target_compile_definitions(DSPSim PUBLIC "OUTPUT_AUDIO_DIR=\"${OUTPUT_AUDIO_DIR}\"")
target_compile_definitions(DSPSim PUBLIC "DEBUG_DIR=\"${DEBUG_DIR}\"")

if(MSVC)
  message("Using MSVC compiler.")
  target_compile_options(DSPSim PRIVATE /W2 /WX)
else()
  target_compile_options(DSPSim PRIVATE -Wall -Wextra -pedantic -Werror)
endif()

set(USE_KISSFFT  ON)
if(USE_KISSFFT)
    message("Using the kissfft library")
	target_compile_definitions(DSPSim PRIVATE -DUSE_KISSFFT)
endif()

option(USE_WAVE_LIBRARY "Use the wave library for .wav files." ON)
if(USE_WAVE_LIBRARY)
    message("Using the wave library.")
	target_compile_definitions(DSPSim PUBLIC -DUSE_WAVE_LIBRARY)
else()
    message("Not using the wave library.")
endif()

option(USE_DOUBLE "Floating point double" ON)
if(USE_DOUBLE)
    message("Using double as floating point type.")
	target_compile_definitions(DSPSim PUBLIC -DUSE_DOUBLE)
    set(KISSFFT_FLOAT_TYPE  "double")
else()
    message("Using float as floating point type.")
    set(KISSFFT_FLOAT_TYPE  "float")
endif()

add_subdirectory(src)
add_subdirectory(deps)
add_subdirectory(test)
add_subdirectory(tools)

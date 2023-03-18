cmake_minimum_required(VERSION 3.15)

project(PitchEngine)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

add_library(PitchEngine "")

set(PITCH_ENGINE_DIR             ${CMAKE_CURRENT_SOURCE_DIR}/)
set(DATA_DIR                     ${PROJECT_SOURCE_DIR}/data/)
set(TRAINING_INPUT_AUDIO_DIR     ${DATA_DIR}training_input_audio/)     # generated input audio (optional)
set(TRAINING_INPUT_DIR           ${DATA_DIR}training_input/)           # fft data from overlapped frames from generated input audio
set(TRAINING_PREDICTED_AUDIO_DIR ${DATA_DIR}training_predicted_audio/) # predicted audio (optional)
set(TRAINING_PREDICTED_DIR       ${DATA_DIR}training_predicted/)       # predicted fft shifted data
set(TRAINING_TARGET_AUDIO_DIR    ${DATA_DIR}training_target_audio/)    # generated target audio (optional)
set(TRAINING_TARGET_DIR          ${DATA_DIR}training_target/)          # target fft shifted data
set(OUTPUT_AUDIO_DIR             ${DATA_DIR}output_audio/)             # output from testing input
set(TEST_AUDIO_DIR               ${DATA_DIR}test_audio/)               # samples for testing the algorithm
set(DEBUG_DIR                    ${DATA_DIR}debug/)                    # variable data to debug (not training)

file(MAKE_DIRECTORY ${DATA_DIR})
file(MAKE_DIRECTORY ${TRAINING_INPUT_AUDIO_DIR})
file(MAKE_DIRECTORY ${TRAINING_INPUT_DIR})
file(MAKE_DIRECTORY ${TRAINING_PREDICTED_AUDIO_DIR})
file(MAKE_DIRECTORY ${TRAINING_PREDICTED_DIR})
file(MAKE_DIRECTORY ${TRAINING_TARGET_AUDIO_DIR})
file(MAKE_DIRECTORY ${TRAINING_TARGET_DIR})
file(MAKE_DIRECTORY ${OUTPUT_AUDIO_DIR})
file(MAKE_DIRECTORY ${TEST_AUDIO_DIR})
file(MAKE_DIRECTORY ${DEBUG_DIR})

target_compile_definitions(PitchEngine PUBLIC "DATA_DIR=\"${DATA_DIR}\"")
target_compile_definitions(PitchEngine PUBLIC "TRAINING_INPUT_AUDIO_DIR=\"${TRAINING_INPUT_AUDIO_DIR}\"")
target_compile_definitions(PitchEngine PUBLIC "TRAINING_INPUT_DIR=\"${TRAINING_INPUT_DIR}\"")
target_compile_definitions(PitchEngine PUBLIC "TRAINING_PREDICTED_AUDIO_DIR=\"${TRAINING_PREDICTED_AUDIO_DIR}\"")
target_compile_definitions(PitchEngine PUBLIC "TRAINING_PREDICTED_DIR=\"${TRAINING_PREDICTED_DIR}\"")
target_compile_definitions(PitchEngine PUBLIC "TRAINING_TARGET_AUDIO_DIR=\"${TRAINING_TARGET_AUDIO_DIR}\"")
target_compile_definitions(PitchEngine PUBLIC "TRAINING_TARGET_DIR=\"${TRAINING_TARGET_DIR}\"")
target_compile_definitions(PitchEngine PUBLIC "OUTPUT_AUDIO_DIR=\"${OUTPUT_AUDIO_DIR}\"")
target_compile_definitions(PitchEngine PUBLIC "TEST_AUDIO_DIR=\"${TEST_AUDIO_DIR}\"")
target_compile_definitions(PitchEngine PUBLIC "DEBUG_DIR=\"${DEBUG_DIR}\"")

if(MSVC)
  message("Using MSVC compiler.")
  target_compile_options(PitchEngine PRIVATE /W2 /WX)
else()
  target_compile_options(PitchEngine PRIVATE -Wall -Wextra -pedantic -Werror)
endif()

set(USE_KISSFFT  ON)
if(USE_KISSFFT)
    message("Using the kissfft library")
	target_compile_definitions(PitchEngine PRIVATE -DUSE_KISSFFT)
endif()

option(USE_WAVE_LIBRARY "Use the wave library for .wav files." ON)
if(USE_WAVE_LIBRARY)
    message("Using the wave library.")
	target_compile_definitions(PitchEngine PUBLIC -DUSE_WAVE_LIBRARY)
else()
    message("Not using the wave library.")
endif()

option(USE_DOUBLE "Floating point double" ON)
if(USE_DOUBLE)
    message("Using double as floating point type.")
	target_compile_definitions(PitchEngine PUBLIC -DUSE_DOUBLE)
    set(KISSFFT_FLOAT_TYPE  "double")
else()
    message("Using float as floating point type.")
    set(KISSFFT_FLOAT_TYPE  "float")
endif()

add_subdirectory(deps)
add_subdirectory(src)
add_subdirectory(test)
add_subdirectory(tools)

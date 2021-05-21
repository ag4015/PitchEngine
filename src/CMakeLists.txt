
# if(PDEBUG)
#   add_definitions(-DPDEBUG)
# endif()
add_definitions(${PDEBUG})
option(PDEBUG "Activate debug logs and store variables in csv files" OFF)
option(USE_DOUBLE "Floating point double" ON)
option(USE_WAVE_LIBRARY "Use the wave library for .wav files." OFF)
option(SIMPLE_PV OFF)
option(KISSFFT_DATATYPE "Float or double" double)

if(PDEBUG)
    message("Printing of logs active.")
	add_compile_definitions(PDEBUG)
endif()

if(SIMPLE_PV)
    message("Algorithm is a simple phase vocoder.")
	add_compile_definitions(SIMPLE_PV)
endif()

if(USE_WAVE_LIBRARY)
    message("Using the wave library.")
	add_definitions(USE_WAVE_LIBRARY)
endif()

if(USE_DOUBLE)
    message("Using double as floating point type.")
	add_compile_definitions(USE_DOUBLE)
    SET_OPTION(KISSFFT_FLOAT_TYPE  "double")
else()
    message("Using float as floating point type.")
    SET_OPTION(KISSFFT_FLOAT_TYPE  "float")
endif()

configure_file(DSPConfig.h.in ${CMAKE_CURRENT_SOURCE_DIR}/DSPConfig.h)

# Override the default option in the kissfft subidrectory
SET_OPTION(KISSFFT_TOOLS  ON) # Build kissfft/tools/

add_subdirectory(kissfft)
add_subdirectory(wave)

# add the executable
add_executable(DSPSim
               main.c
               wavio.cpp
               audioData.c
               audioUtils.cpp
               logger.cpp
               constant-q-cpp/src/Chromagram.cpp
               constant-q-cpp/src/CQSpectrogram.cpp
               constant-q-cpp/src/ConstantQ.cpp
               constant-q-cpp/src/CQInverse.cpp
               constant-q-cpp/src/CQKernel.cpp
               constant-q-cpp/src/Pitch.cpp
               constant-q-cpp/src/dsp/FFT.cpp
               constant-q-cpp/src/dsp/MathUtilities.cpp
               constant-q-cpp/src/dsp/Resampler.cpp
               constant-q-cpp/src/dsp/SincWindow.cpp
               constant-q-cpp/src/dsp/KaiserWindow.cpp
               kissfft/tools/kiss_fftr.c
               )

target_include_directories(DSPSim PUBLIC
                           constant-q-cpp/cq)

target_include_directories(DSPSim PUBLIC
                           constant-q-cpp/src)

#add_executable(DSPSimTest
#               test.cpp
#               audioData.c
#               audioUtils.cpp
#               logger.cpp
#               )
target_compile_options(DSPSim PUBLIC -Wall -pedantic -g)

target_include_directories(DSPSim PUBLIC
                           src/
                           kissfft/tools/
                           )

target_link_libraries(DSPSim PUBLIC
                        kissfft
                        wave)

#target_link_libraries(DSPSimTest PUBLIC
#                        kissfft)



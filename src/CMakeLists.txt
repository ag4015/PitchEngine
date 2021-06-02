
option(DEBUG_DUMP "Store variables in csv files" OFF)
option(USE_DOUBLE "Floating point double" ON)
option(USE_WAVE_LIBRARY "Use the wave library for .wav files." OFF)
option(SIMPLE_PV "Pitch shift with a simple phase vocoder" OFF)
option(KISSFFT_DATATYPE "Float or double" double)
option(CONSTANT_Q_T "Use the Constant Q transform." ON)

if(DEBUG_DUMP)
    message("CSV dumping of variables is active.")
    add_compile_definitions(DEBUG_DUMP)
endif()

if(SIMPLE_PV)
    message("Algorithm is a simple phase vocoder.")
    add_compile_definitions(SIMPLE_PV)
endif()

if(USE_WAVE_LIBRARY)
    message("Using the wave library.")
    add_compile_definitions(USE_WAVE_LIBRARY)
endif()

if(USE_DOUBLE)
    message("Using double as floating point type.")
    SET_OPTION(KISSFFT_FLOAT_TYPE  "double")
    add_compile_definitions(USE_DOUBLE)
else()
    message("Using float as floating point type.")
    SET_OPTION(KISSFFT_FLOAT_TYPE  "float")
endif()

if(CONSTANT_Q_T)
    message("Using the constant Q transform")
    add_compile_definitions(CONSTANT_Q_T)
endif()

configure_file(DSPConfig.h.in ${CMAKE_CURRENT_SOURCE_DIR}/DSPConfig.h)

# Override the default option in the kissfft subidrectory
SET_OPTION(KISSFFT_TOOLS  ON) # Build kissfft/tools/

add_subdirectory(kissfft)
add_subdirectory(wave)
#add_subdirectory(libsndfile)

if(CONSTANT_Q_T)
	set(CONSTANT_CQ_SOURCE_FILES
				   constant-q-cpp/src/Chromagram.cpp;
				   constant-q-cpp/src/CQSpectrogram.cpp;
				   constant-q-cpp/src/ConstantQ.cpp;
				   constant-q-cpp/src/CQInverse.cpp;
				   constant-q-cpp/src/CQKernel.cpp;
				   constant-q-cpp/src/Pitch.cpp;
				   constant-q-cpp/src/dsp/FFT.cpp;
				   constant-q-cpp/src/dsp/MathUtilities.cpp;
				   constant-q-cpp/src/dsp/Resampler.cpp;
				   constant-q-cpp/src/dsp/SincWindow.cpp;
				   constant-q-cpp/src/dsp/KaiserWindow.cpp;
				   )
else()
	set(CONSTANT_CQ_SOURCE_FILES "")
endif()


set(KISSFFT_EXTRA_SOURCE_FILES
               kissfft/tools/kiss_fftr.c
               )

# add the executable
add_executable(DSPSim
               main.c
               wavio.cpp
               audioData.c
               audioUtils.cpp
               PitchEngine.cpp
               PVEngine.cpp
               logger.cpp
               ${CONSTANT_CQ_SOURCE_FILES}
               ${KISSFFT_EXTRA_SOURCE_FILES}
               )

if(CONSTANT_Q_T)
target_include_directories(DSPSim PUBLIC
                           constant-q-cpp/cq
                           constant-q-cpp/src
                           )
endif()

# add_executable(DSPSimTest
#                test.cpp
#                audioData.c
#                audioUtils.cpp
#                logger.cpp
#                )
target_compile_options(DSPSim PUBLIC -Wall -pedantic -g)

target_include_directories(DSPSim PUBLIC
                           src/
                           kissfft/tools/
                           )

target_link_libraries(DSPSim PUBLIC
                        kissfft
                        wave)

# target_link_libraries(DSPSimTest PUBLIC
#                         kissfft)

#add_executable(processfile_cq
#                constant-q-cpp/test/processfile.cpp
#                ${CONSTANT_CQ_SOURCE_FILES}
#                ${KISSFFT_EXTRA_SOURCE_FILES})

#target_include_directories(processfile_cq PUBLIC
#                           constant-q-cpp/src/
#                           constant-q-cpp/cq/
#                           kissfft/
#                           kissfft/tools)

#target_link_libraries(processfile_cq sndfile)



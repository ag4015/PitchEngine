
option(DEBUG_DUMP "Store variables in csv files" OFF)
option(DEBUG_LOG "Enable logs" ON)
option(USE_DOUBLE "Floating point double" OFF)
option(USE_WAVE_LIBRARY "Use the wave library for .wav files." OFF)
option(SIMPLE_PV "Pitch shift with a simple phase vocoder" OFF)
#option(KISSFFT_DATATYPE "Float or double" double)

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
    #add_compile_definitions(USE_DOUBLE)
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
SET_OPTION(USE_KISSFFT  ON) # Build kissfft/tools/

add_subdirectory(wave)
#add_subdirectory(libsndfile)

if(USE_KISSFFT)
#    add_subdirectory(kissfft)
    set(KISSFFT_LIB_DIR constant-q-cpp/src/ext/kissfft/)
    set(KISSFFT_TOOLS_DIR ${KISSFFT_LIB_DIR}tools/)
	set(KISSFFT_EXTRA_SOURCE_FILES
				   ${KISSFFT_TOOLS_DIR}kiss_fftr.c
				   ${KISSFFT_LIB_DIR}kiss_fft.c
				   )
    set(KISSFFT_LIB_NAME kissfft)
    set(KISSFFT_TOOLS_SOURCE_FILES
			   ${KISSFFT_TOOLS_DIR}fftutil.c
			   ${KISSFFT_TOOLS_DIR}kfc.c
			   ${KISSFFT_TOOLS_DIR}kiss_fastfir.c
			   ${KISSFFT_TOOLS_DIR}kiss_fftnd.c
			   ${KISSFFT_TOOLS_DIR}kiss_fftndr.c
			   ${KISSFFT_TOOLS_DIR}kiss_fftr.c
			   ${KISSFFT_TOOLS_DIR}psdpng.c
               )
else()
	set(KISSFFT_EXTRA_SOURCE_FILES "")
    set(KISSFFT_TOOLS_DIR "")
    set(KISSFFT_LIB_NAME "")
endif()

set(CONSTANT_Q_LIB_DIR constant-q-cpp/)
set(CONSTANT_Q_SRC_DIR ${CONSTANT_Q_LIB_DIR}src/)
set(CONSTANT_Q_CQ_DIR ${CONSTANT_Q_LIB_DIR}cq/)
set(CONSTANT_Q_SRC_DSP_DIR ${CONSTANT_Q_SRC_DIR}dsp/)
set(CONSTANT_Q_SOURCE_FILES
			   ${CONSTANT_Q_SRC_DIR}Chromagram.cpp;
			   ${CONSTANT_Q_SRC_DIR}CQSpectrogram.cpp;
			   ${CONSTANT_Q_SRC_DIR}ConstantQ.cpp;
			   ${CONSTANT_Q_SRC_DIR}CQInverse.cpp;
			   ${CONSTANT_Q_SRC_DIR}CQKernel.cpp;
			   ${CONSTANT_Q_SRC_DIR}Pitch.cpp;
			   ${CONSTANT_Q_SRC_DIR}dsp/FFT.cpp;
			   ${CONSTANT_Q_SRC_DSP_DIR}MathUtilities.cpp;
			   ${CONSTANT_Q_SRC_DSP_DIR}Resampler.cpp;
			   ${CONSTANT_Q_SRC_DSP_DIR}SincWindow.cpp;
			   ${CONSTANT_Q_SRC_DSP_DIR}KaiserWindow.cpp;
			   )

configure_file(${CONSTANT_Q_LIB_DIR}ConstantQConfig.h.in ${CONSTANT_Q_CQ_DIR}ConstantQConfig.h)

set(DSPSIM_SOURCE_FILES
               audioData.c
               audioUtils.cpp
               PitchEngine.cpp
               PVEngine.cpp
               PVDREngine.cpp
               CQPVEngine.cpp
               wavio.cpp
               Dumper.cpp
               DumperContainer.cpp
               ${CONSTANT_Q_SOURCE_FILES}
               ${KISSFFT_EXTRA_SOURCE_FILES}
               )

add_library(DSPSim STATIC ${DSPSIM_SOURCE_FILES})

target_include_directories(DSPSim PUBLIC
                           src/
                           constant-q-cpp/cq
                           constant-q-cpp/src
                           ${KISSFFT_LIB_DIR}
                           ${KISSFFT_TOOLS_DIR}
                           )

target_link_libraries(DSPSim PUBLIC
#                        ${KISSFFT_LIB_NAME}
                        wave)


if(MSVC)
  message("Usign MSVC compiler.")
  target_compile_options(DSPSim PRIVATE /W4 /WX)
else()
  target_compile_options(DSPSim PRIVATE -Wall -Wextra -pedantic -Werror)
endif()


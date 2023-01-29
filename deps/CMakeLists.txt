
set(PITCH_ENGINE_DEPENDENCIES_DIR ${PITCH_ENGINE_DIR}deps/)

# override wave_enable_tests from the subproject wave
SET(wave_enable_tests OFF CACHE BOOL "Build Unit tests wave")

add_subdirectory(wave)
add_subdirectory(ParameterCombinator)
add_subdirectory(Variable-Dumper)

# CONSTANT_Q Folder
set(CONSTANT_Q_LIB_DIR ${PITCH_ENGINE_DEPENDENCIES_DIR}constant-q-cpp/)

add_library(constant_q_lib "")

set(CONSTANT_Q_SRC_DIR ${CONSTANT_Q_LIB_DIR}src/)
set(CONSTANT_Q_CQ_DIR ${CONSTANT_Q_LIB_DIR}cq/)
set(CONSTANT_Q_CQ_MISC_DIR ${CONSTANT_Q_LIB_DIR}misc/)
set(CONSTANT_Q_SRC_DSP_DIR ${CONSTANT_Q_SRC_DIR}dsp/)

configure_file(ConstantQConfig.h.in ${CONSTANT_Q_CQ_DIR}/ConstantQConfig.h)

set(CONSTANT_Q_SOURCE_FILES
			   ${CONSTANT_Q_SRC_DIR}Chromagram.cpp;
			   ${CONSTANT_Q_SRC_DIR}CQSpectrogram.cpp;
			   ${CONSTANT_Q_SRC_DIR}ConstantQ.cpp;
			   ${CONSTANT_Q_SRC_DIR}CQInverse.cpp;
			   ${CONSTANT_Q_SRC_DIR}CQKernel.cpp;
			   ${CONSTANT_Q_SRC_DIR}Pitch.cpp;
			   ${CONSTANT_Q_SRC_DSP_DIR}FFT.cpp;
			   ${CONSTANT_Q_SRC_DSP_DIR}MathUtilities.cpp;
			   ${CONSTANT_Q_SRC_DSP_DIR}Resampler.cpp;
			   ${CONSTANT_Q_SRC_DSP_DIR}SincWindow.cpp;
			   ${CONSTANT_Q_SRC_DSP_DIR}KaiserWindow.cpp;
			   )


target_sources(constant_q_lib
	PRIVATE
		${CONSTANT_Q_SOURCE_FILES}
)
target_include_directories(constant_q_lib
	PUBLIC
		${CONSTANT_Q_SRC_DIR}
		${CONSTANT_Q_CQ_DIR}
		${CONSTANT_Q_CQ_MISC_DIR}
		${CONSTANT_Q_SRC_DSP_DIR}
)

# KissFFT Folder
if(USE_KISSFFT)

	add_library(kissfft "")

    set(KISSFFT_DIR ${CONSTANT_Q_SRC_DIR}ext/kissfft/)
    set(KISSFFT_TOOLS_DIR ${KISSFFT_DIR}tools/)

    set(KISSFFT_TOOLS_SOURCE_FILES
#			   ${KISSFFT_TOOLS_DIR}fftutil.c
			   ${KISSFFT_TOOLS_DIR}kfc.c
			   ${KISSFFT_TOOLS_DIR}kiss_fastfir.c
			   ${KISSFFT_TOOLS_DIR}kiss_fftnd.c
			   ${KISSFFT_TOOLS_DIR}kiss_fftndr.c
			   ${KISSFFT_TOOLS_DIR}kiss_fftr.c
#			   ${KISSFFT_TOOLS_DIR}psdpng.c
               )
	target_sources(kissfft
		PUBLIC
			   ${KISSFFT_TOOLS_SOURCE_FILES}
			   ${KISSFFT_DIR}kiss_fft.c
	)
	target_include_directories(kissfft
		PUBLIC
			${KISSFFT_DIR}
			${KISSFFT_TOOLS_DIR}
	)
	target_link_libraries(constant_q_lib PUBLIC kissfft)
else()
	set(KISSFFT_EXTRA_SOURCE_FILES "")
    set(KISSFFT_TOOLS_DIR "")
    set(KISSFFT_TOOLS_SOURCE_FILES "")
    set(KISSFFT_TOOLS_HEADER_FILES "")
endif()

target_sources(constant_q_lib
	PUBLIC
		${CONSTANT_Q_SOURCE_FILES}
		${KISSFFT_SOURCE_FILES}
)

target_include_directories(constant_q_lib PUBLIC
			  ${CONSTANT_Q_SRC_DIR}
			  ${CONSTANT_Q_CQ_DIR}
			  ${CONSTANT_Q_SRC_DSP_DIR}
			  ${CONSTANT_Q_CQ_MISC_DIR}
)

target_link_libraries(constant_q_lib PUBLIC kissfft)

# Maximilian folder
add_library(Maximilian "")

set(MAXIMILIAN_DIR ${PITCH_ENGINE_DEPENDENCIES_DIR}Maximilian/)
set(MAXIMILIAN_SRC_DIR ${MAXIMILIAN_DIR}src/)

set(MAXIMILIAN_SOURCE_FILES
			   ${MAXIMILIAN_SRC_DIR}maximilian.cpp
			   )

target_sources(Maximilian PUBLIC ${MAXIMILIAN_SOURCE_FILES} )

target_include_directories(Maximilian PUBLIC ${MAXIMILIAN_SRC_DIR})

target_link_libraries(PitchEngine PUBLIC constant_q_lib wave ParameterCombinator VariableDumper)


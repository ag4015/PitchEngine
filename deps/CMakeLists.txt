
set(PITCH_ENGINE_DEPENDENCIES_DIR ${PITCH_ENGINE_DIR}deps/)

add_subdirectory(wave)
add_subdirectory(googletest)

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
set(CONSTANT_Q_HEADER_FILES
			   ${CONSTANT_Q_CQ_DIR}ConstantQConfig.h
			   ${CONSTANT_Q_SRC_DSP_DIR}FFT.h;
			   ${CONSTANT_Q_SRC_DSP_DIR}KaiserWindow.h;
			   ${CONSTANT_Q_SRC_DSP_DIR}MathUtilities.h;
			   ${CONSTANT_Q_SRC_DSP_DIR}nan-inf.h;
			   ${CONSTANT_Q_SRC_DSP_DIR}pi.h;
			   ${CONSTANT_Q_SRC_DSP_DIR}Resampler.h;
			   ${CONSTANT_Q_SRC_DSP_DIR}SincWindow.h;
			   ${CONSTANT_Q_SRC_DSP_DIR}Window.h;
			   )


target_sources(constant_q_lib
	PUBLIC
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
		${CONSTANT_Q_HEADER_FILES}
		${KISSFFT_SOURCE_FILES}
		${KISSFFT_HEADER_FILES}
)


target_include_directories(constant_q_lib PUBLIC
			  ${CONSTANT_Q_SRC_DIR}
			  ${CONSTANT_Q_CQ_DIR}
			  ${CONSTANT_Q_SRC_DSP_DIR}
			  ${CONSTANT_Q_CQ_MISC_DIR}
)

add_library(waveform_generator "")

set(WAVEFORM_GENERATOR_DIR ${PITCH_ENGINE_DEPENDENCIES_DIR}waveform-generator/)
set(WAVEFORM_GENERATOR_SRC_DIR ${WAVEFORM_GENERATOR_DIR}WaveformGenerator/)

set(WAVEFORM_GENERATOR_SOURCE_FILES
		   ${WAVEFORM_GENERATOR_SRC_DIR}waveform_generator.cpp
		   )

target_sources(waveform_generator PUBLIC ${WAVEFORM_GENERATOR_SOURCE_FILES})

target_link_libraries(constant_q_lib PUBLIC kissfft)

target_link_libraries(PitchEngine PUBLIC constant_q_lib wave)

set(CPP_TQDM_DIR ${PITCH_ENGINE_DEPENDENCIES_DIR}cpptqdm/)
set(PROGRESSBAR_DIR ${PITCH_ENGINE_DEPENDENCIES_DIR}progressbar/)

target_include_directories(PitchEngine
	PUBLIC 
		${CPP_TQDM_DIR}
		${PROGRESSBAR_DIR}
)


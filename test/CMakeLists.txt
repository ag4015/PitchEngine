
set(PITCH_ENGINE_TEST_DIR ${PITCH_ENGINE_DIR}test/)

add_executable(PitchEngineTs
               main.cpp
               PitchEngineTs.cpp
               #../deps/waveform-generator/WaveformGenerator/waveform_generator.h
               )


target_compile_options(PitchEngineTs PUBLIC -Wall -pedantic -g)

target_include_directories(PitchEngineTs PUBLIC
                           ../src/
                           ../src/Engine/
                           ${PITCH_ENGINE_SRC_DIR}
                           ${PITCH_ENGINE_SRC_DIR}Engine/
                           ${WAVEFORM_GENERATOR_DIR}
                           ${WAVEFORM_GENERATOR_SRC_DIR}
                           ${PARAMETER_COMBINATOR_SRC_DIR}
                           ${MAXIMILIAN_SRC_DIR}
                           ../kissfft/tools/
                           ../deps/cpptqdm/
                           )

target_link_libraries(PitchEngineTs PRIVATE
                        PitchEngine
                        ParameterCombinator
                        Maximilian
                        )

option(USE_MULTITHREADING "Use multithreading for each parameter" ON)
if(USE_MULTITHREADING)
    message("Multithreading activated.")
	target_compile_definitions(PitchEngineTs PRIVATE -DUSE_MULTITHREADING)
	target_compile_definitions(PitchEngine PRIVATE -DUSE_MULTITHREADING)
else()
    message("Multithreading not activated.")
endif()

option(USE_PROGRESS_BAR "Use progress bar" OFF)
if(USE_PROGRESS_BAR)
    message("Progress bar activated.")
	target_compile_definitions(PitchEngineTs PRIVATE -DUSE_PROGRESS_BAR)
else()
    message("Progress bar not activated.")
endif()


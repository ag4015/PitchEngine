
set(PITCH_ENGINE_TEST_DIR ${PITCH_ENGINE_DIR}test/)

add_executable(PitchEngineTs
               main.cpp
               PitchEngineTs.cpp
               )


target_compile_options(PitchEngineTs PUBLIC -Wall -pedantic -g)

target_include_directories(PitchEngineTs PUBLIC
                           ../src/
                           ../src/Engine/
                           ${PITCH_ENGINE_SRC_DIR}
                           ${PITCH_ENGINE_SRC_DIR}Engine/
                           ../kissfft/tools/
                           ../deps/cpptqdm/
                           )

target_link_libraries(PitchEngineTs PUBLIC
                        PitchEngine
                        )

option(USE_MULTITHREADING "Use multithreading for each parameter" ON)
if(USE_MULTITHREADING)
    message("Multithreading activated.")
	target_compile_definitions(PitchEngineTs PRIVATE -DUSE_MULTITHREADING)
else()
    message("Multithreading not activated.")
endif()

option(USE_PROGRESS_BAR "Use progress bar" ON)
if(USE_PROGRESS_BAR)
    message("Progress bar activated.")
	target_compile_definitions(PitchEngineTs PRIVATE -DUSE_PROGRESS_BAR)
else()
    message("Progress bar not activated.")
endif()


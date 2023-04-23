
set(PITCH_ENGINE_TEST_DIR ${PITCH_ENGINE_DIR}test/)

add_executable(PitchEngineTs
               main.cpp
               PitchEngineTs.cpp
               )


target_compile_options(PitchEngineTs PUBLIC -Wall -pedantic -g)

target_include_directories(PitchEngineTs PRIVATE . )

target_link_libraries(PitchEngineTs PRIVATE
                        PitchEngine
                        ParameterCombinator
                        Maximilian
                        pthread
                        )

option(USE_MULTITHREADING "Use multithreading for each parameter" ON)
if(USE_MULTITHREADING)
    message("Multithreading activated.")
	target_compile_definitions(PitchEngineTs PRIVATE -DUSE_MULTITHREADING)
	target_compile_definitions(PitchEngine PRIVATE -DUSE_MULTITHREADING)
else()
    message("Multithreading not activated.")
endif()

option(DEBUG_LOG "Enable logs." ON)
if(DEBUG_LOG)
    message("Logging enabled.")
	target_compile_definitions(PitchEngineTs PRIVATE -DDEBUG_LOG)
	target_compile_definitions(PitchEngine PRIVATE -DDEBUG_LOG)
else()
    message("Multithreading not activated.")
endif()


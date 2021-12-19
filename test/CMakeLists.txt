
set(DSPSIM_TEST_DIR ${DSPSIM_DIR}test/)

add_executable(DSPSimTs
               main.cpp
               )


target_compile_options(DSPSimTs PUBLIC -Wall -pedantic -g)

target_include_directories(DSPSimTs PUBLIC
                           ../src/
                           ../src/Engine/
                           ${DSPSIM_SRC_DIR}
                           ${DSPSIM_SRC_DIR}Engine/
                           ../kissfft/tools/
                           )

target_link_libraries(DSPSimTs PUBLIC
                        DSPSim
                        )

option(USE_MULTITHREADING "Use multithreading for each parameter" OFF)
if(USE_MULTITHREADING)
    message("Multithreading activated.")
	target_compile_definitions(DSPSimTs PRIVATE -DUSE_MULTITHREADING)
else()
    message("Multithreading not activated.")
endif()


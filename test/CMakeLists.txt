
add_executable(DSPSimTs
               main.cpp
               )

target_compile_options(DSPSimTs PUBLIC -Wall -pedantic -g)

target_include_directories(DSPSimTs PUBLIC
                           ../src/
                           ../kissfft/tools/
                           )

target_link_libraries(DSPSimTs PUBLIC
                        DSPSim
#                        kissfft
                        wave)

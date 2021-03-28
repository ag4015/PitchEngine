
# option(PDEBUG "Activate debug logs and store variables in csv files" OFF)
# if(PDEBUG)
#   add_definitions(-DPDEBUG)
# endif()
add_definitions(${PDEBUG})

configure_file(DSPConfig.h.in ${CMAKE_CURRENT_SOURCE_DIR}/DSPConfig.h)

option(USE_WAVE_LIBRARY "Use the wave library for .wav files." OFF)
if(USE_WAVE_LIBRARY)
add_definitions(-DUSE_WAVE_LIBRARY)
endif()

add_subdirectory(kissfft)
add_subdirectory(wave)

# add the executable
add_executable(DSPSim
               main.c
               wavio.cpp
               audioData.c
               audioUtils.cpp
               logger.cpp
               )

target_compile_options(DSPSim PUBLIC -Wall -pedantic -g)

target_include_directories(DSPSim PUBLIC
                           src/
                           )

target_link_libraries(DSPSim PUBLIC
                        kissfft
                        wave)

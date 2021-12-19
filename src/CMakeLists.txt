
set(DSPSIM_SRC_DIR ${DSPSIM_DIR}src/)

option(CONSTANT_Q_T "Constant Q Transform" OFF)
if(CONSTANT_Q_T)
    message("Using the constant Q transform")
	target_compile_definitions(DSPSim PRIVATE -DCONSTANT_Q_T)
endif()

configure_file(DSPSimConfig.h.in ${DSPSIM_SRC_DIR}/DSPSimConfig.h)

add_subdirectory(engine)
add_subdirectory(debug)

target_include_directories(DSPSim PUBLIC
                           src/
                           src/Engine/
                           constant-q-cpp/cq
                           constant-q-cpp/src
                           ${KISSFFT_LIB_DIR}
                           ${KISSFFT_TOOLS_DIR}
                           )

if(NOT MSVC)
	target_link_libraries(DSPSim PRIVATE stdc++fs)
endif()



set(PITCH_ENGINE_SRC_DIR ${PITCH_ENGINE_DIR}src/)

option(CONSTANT_Q_T "Constant Q Transform" OFF)
if(CONSTANT_Q_T)
    message("Using the constant Q transform")
	target_compile_definitions(PitchEngine PRIVATE -DCONSTANT_Q_T)
endif()

configure_file(PitchEngineConfig.h.in ${PITCH_ENGINE_SRC_DIR}/PitchEngineConfig.h)

add_subdirectory(engine)
add_subdirectory(debug)

target_include_directories(PitchEngine PUBLIC
                           src/
                           src/Engine/
                           constant-q-cpp/cq
                           constant-q-cpp/src
                           ${KISSFFT_LIB_DIR}
                           ${KISSFFT_TOOLS_DIR}
                           )

if(NOT MSVC)
	target_link_libraries(PitchEngine PRIVATE stdc++fs)
endif()


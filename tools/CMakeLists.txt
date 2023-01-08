
set(PITCH_ENGINE_SRC_TOOLS_DR ${PITCH_ENGINE_SRC_DIR}tools/)

target_sources(PitchEngineTs
    PRIVATE
        wavio.cpp
)

target_include_directories(PitchEngineTs
    PRIVATE
        .
)


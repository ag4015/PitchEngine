
set(PITCH_ENGINE_SRC_ENGINE_DIR ${PITCH_ENGINE_SRC_DIR}engine/)

target_sources(PitchEngine 
    PRIVATE
	   PitchEngine.cpp
	   PVEngine.cpp
	   PVDREngine.cpp
	   CQPVEngine.cpp
	   StrechEngine.cpp
	   NNPVEngineTrainer.cpp
)

target_include_directories(PitchEngine PUBLIC .)


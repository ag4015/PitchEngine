
set(DSPSIM_SRC_ENGINE_DIR ${DSPSIM_SRC_DIR}engine/)

target_sources(DSPSim 
    PRIVATE
	   PitchEngine.cpp
	   PitchEngine.h
	   PVEngine.cpp
	   PVEngine.h
	   PVDREngine.cpp
	   PVDREngine.h
	   CQPVEngine.cpp
	   CQPVEngine.h
	   audioData.c
	   audioData.h
	   audioUtils.cpp
	   audioUtils.h
)


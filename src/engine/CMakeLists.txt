
set(PITCH_ENGINE_SRC_ENGINE_DIR ${PITCH_ENGINE_SRC_DIR}engine/)

target_sources(PitchEngine 
    PRIVATE
	   PitchEngine.cpp
	   PitchEngine.h
	   PVEngine.cpp
	   PVEngine.h
	   PVDREngine.cpp
	   PVDREngine.h
	   CQPVEngine.cpp
	   CQPVEngine.h
	   StrechEngine.cpp
	   StrechEngine.h
)


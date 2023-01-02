
set(PITCH_ENGINE_SRC_DEBUG_DIR ${PITCH_ENGINE_SRC_DIR}debug/)

target_sources(PitchEngine 
    PRIVATE
		Dumper.cpp
		DumperContainer.cpp
		Timer.cpp
		TimerContainer.cpp
)

target_include_directories(PitchEngine PUBLIC . )

option(DEBUG_DUMP "Store variables in csv files" ON)
if(DEBUG_DUMP)
    message("CSV dumping of variables is active.")
	target_compile_definitions(PitchEngine PUBLIC -DDEBUG_DUMP)
endif()

option(DEBUG_TIMING "Measure time of execution" ON)
if(DEBUG_TIMING)
    message("Time measurement activated")
	target_compile_definitions(PitchEngine PUBLIC -DDEBUG_TIMING)
endif()

option(DEBUG_LOG "Enable logs" ON)
if(DEBUG_LOG)
    message("Logs activated")
	target_compile_definitions(PitchEngine PUBLIC -DDEBUG_LOG)
endif()


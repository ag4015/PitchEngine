
set(DSPSIM_SRC_DEBUG_DIR ${DSPSIM_SRC_DIR}debug/)

target_sources(DSPSim 
    PRIVATE
		Dumper.cpp
		Dumper.h
		DumperContainer.cpp
		DumperContainer.h
		Timer.cpp
		Timer.h
		TimerContainer.cpp
		TimerContainer.h
		logger.h
)

target_include_directories(DSPSim
	PUBLIC
		.
)

option(DEBUG_DUMP "Store variables in csv files" OFF)
if(DEBUG_DUMP)
    message("CSV dumping of variables is active.")
	target_compile_definitions(DSPSim PRIVATE -DDEBUG_DUMP)
endif()

option(DEBUG_TIMING "Measure time of execution" ON)
if(DEBUG_TIMING)
    message("Time measurement activated")
	target_compile_definitions(DSPSim PRIVATE -DDEBUG_TIMING)
endif()

option(DEBUG_LOG "Enable logs" OFF)
if(DEBUG_LOG)
    message("Logs activated")
	target_compile_definitions(DSPSim PRIVATE -DDEBUG_LOG)
endif()


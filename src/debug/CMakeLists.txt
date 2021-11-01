
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

if(DEBUG_DUMP)
    message("CSV dumping of variables is active.")
	option(DEBUG_DUMP "Store variables in csv files" OFF)
	target_compile_definitions(DSPSim PRIVATE -DDEBUG_DUMP)
endif()

if(DEBUG_TIMING)
    message("Time measurement activated")
	option(DEBUG_TIMING "Measure time of execution" ON)
	target_compile_definitions(DSPSim PRIVATE -DDEBUG_TIMING)
endif()

if(DEBUG_LOG)
    message("Logs activated")
	option(DEBUG_LOG "Enable logs" OFF)
	target_compile_definitions(DSPSim PRIVATE -DDEBUG_LOG)
endif()


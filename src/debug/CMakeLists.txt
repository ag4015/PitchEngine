
set(PITCH_ENGINE_SRC_DEBUG_DIR ${PITCH_ENGINE_SRC_DIR}debug/)

target_sources(PitchEngine 
    PRIVATE
		#Timer.cpp
		#TimerContainer.cpp
)

target_include_directories(PitchEngine PUBLIC . )

option(VARIABLE_DUMPER_DISABLED "Store variables in csv files" OFF)
if(DEBUG_DUMP)
    message("CSV DUMPing of variables is active.")
	target_compile_definitions(PitchEngine PUBLIC -DVARIABLE_DUMPER_DISABLED)
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


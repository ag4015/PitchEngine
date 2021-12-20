
set(PITCH_ENGINE_SRC_TOOLS_DR ${PITCH_ENGINE_SRC_DIR}tools/)

target_sources(PitchEngineTs
    PRIVATE
        wavio.cpp
)

target_include_directories(PitchEngineTs
    PRIVATE
        .
)

#set(Eigen3_DIR ../../../Programs/eigen)
#find_package (Eigen3 3.3 REQUIRED NO_MODULE)
 
#add_executable(testCPUspeed
#								testCPUSpeed.cpp)

#target_link_libraries (testCPUspeed Eigen3::Eigen)


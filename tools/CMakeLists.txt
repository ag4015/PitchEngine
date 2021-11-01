
set(DSPSIM_SRC_TOOLS_DR ${DSPSIM_SRC_DIR}tools/)

target_sources(DSPSimTs
    PRIVATE
        wavio.cpp
)

target_include_directories(DSPSimTs
    PRIVATE
        .
)

#set(Eigen3_DIR ../../../Programs/eigen)
#find_package (Eigen3 3.3 REQUIRED NO_MODULE)
 
#add_executable(testCPUspeed
#								testCPUSpeed.cpp)

#target_link_libraries (testCPUspeed Eigen3::Eigen)


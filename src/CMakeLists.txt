
# set(RESOURCES_DIR "../resources/")
SET(RESOURCES_DIR    ${PROJECT_SOURCE_DIR}/resources/      CACHE PATH "This is the resources directory")

configure_file(DSPConfig.h.in ${CMAKE_CURRENT_SOURCE_DIR}/DSPConfig.h)

add_subdirectory(kissfft)

# add the executable
add_executable(DSPSimulator
               main.c
               wavio.c
               audioUtils.cpp
               )

target_compile_options(DSPSimulator PUBLIC -Wall)

target_include_directories(DSPSimulator PUBLIC
                           src/
                           )

target_link_libraries(DSPSimulator PUBLIC kissfft)

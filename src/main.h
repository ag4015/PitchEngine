#ifndef MAIN_H
#define MAIN_H

#include "DSPConfig.h"
#include "audioData.h"
#include <math.h>
#include <stdint.h>
#include <stddef.h>

 //#define PDEBUG                          // Print debug information
//#define DSPDEBUG
#define SIMULATION                      // This is a simulation of the device

#define COPY(x,y,z) for(uint16_t k = 0; k < z; k++) { x = y; } 

#ifdef DEBUG_LOG
       #define PRINT_LOG1(x)       printf(x)
       #define PRINT_LOG2(x,y)     printf(x,y)
       #define PRINT_LOG3(x,y,z)   printf(x,y,z)
#else
       #define PRINT_LOG1(x) 
       #define PRINT_LOG2(x,y) 
       #define PRINT_LOG3(x,y,z) 
#endif

#define INPUT_AUDIO_DIR RESOURCES_DIR "inputAudio/"
#define OUTPUT_AUDIO_DIR RESOURCES_DIR "outputAudio/"
#define DEBUG_DIR RESOURCES_DIR "debugData/"

#define USE_WAVE_LIBRARY


// Function declaration
#pragma once
#ifdef __cplusplus
extern "C"
{
#endif

void load_distortion_coefficients(float* coeffs, size_t* coeff_size);
void parse_arguments(int argc, char** argv, char** inputFilePath, char** outputFilePath, float var);

#ifdef __cplusplus
} // extern C
#endif

#endif // MAIN_H


#ifndef MAIN_H
#define MAIN_H

#include "DSPConfig.h"
#include "audioData.h"
#include <math.h>
#include <stdint.h>
#include <stddef.h>
#include <string>

#define COPY(x,y,z) for(uint16_t k = 0; k < z; k++) { x = y; } 

#define USE_WAVE_LIBRARY

void parse_arguments(int argc, char** argv, std::string& inputFilePath, std::string& outputFilePath, my_float* var);
void initializeDumpers(uint32_t & audio_ptr, buffer_data * bf, audio_data * audat);

// Function declaration
#pragma once
#ifdef __cplusplus
extern "C"
{
#endif

void load_distortion_coefficients(my_float* coeffs, size_t* coeff_size);

#ifdef __cplusplus
} // extern C
#endif

#endif // MAIN_H


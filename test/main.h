#ifndef MAIN_H
#define MAIN_H

#include "DSPConfig.h"
#include "audioData.h"
#include <math.h>
#include <stdint.h>
#include <stddef.h>
#include <string>
#include <unordered_map>
#include <set>

#define COPY(x,y,z) for(uint16_t k = 0; k < z; k++) { x = y; } 

#define USE_WAVE_LIBRARY

using parameterMap_t = std::unordered_map< std::string, std::set<uint32_t> >;
using parameterCombinations_t = std::unordered_map<std::string, std::vector<uint32_t> >;

void parse_arguments(int argc, char** argv, std::string& inputFilePath, std::string& outputFilePath, my_float* var);
void initializeDumpers(uint32_t & audio_ptr, buffer_data * bf, audio_data * audat, std::string & variationName);
void generateParameterCombinations(const parameterMap_t & allParams, parameterCombinations_t & paramCombs);

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


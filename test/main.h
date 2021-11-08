#ifndef MAIN_H
#define MAIN_H

#include "audioData.h"
#include <math.h>

#include <stdint.h>
#include <stddef.h>
#include <string>
#include <unordered_map>
#include <set>

#define COPY(x,y,z) for(uint16_t k = 0; k < z; k++) { x = y; } 

#define USE_WAVE_LIBRARY

enum
{
	PV = 0,
	PVDR,
	CQPV
};

//using parameterMap_t = std::unordered_map< std::string, std::vector<uint32_t> >;
using parameterInstanceMap_t = std::unordered_map< std::string, uint32_t>;
using parameterCombinations_t = std::unordered_map<std::string, std::vector<uint32_t> >;

void parse_arguments(int argc, char** argv, std::string& inputFilePath, std::string& outputFilePath, my_float* var);
void initializeDumpers(uint32_t & audio_ptr, buffer_data * bf, audio_data * audat, std::string & variationName);
//void generateParameterCombinations(const parameterMap_t & allParams, parameterCombinations_t & paramCombs);
void runTest(std::string & inputFilePath, std::string & outputFilePath, parameterInstanceMap_t paramInstance, std::string& variationName);


#endif // MAIN_H


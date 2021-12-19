#ifndef MAIN_H
#define MAIN_H

#include <math.h>
#include <stdint.h>
#include <stddef.h>
#include <string>
#include <unordered_map>
#include <set>
#include <vector>

#ifdef USE_DOUBLE
typedef double my_float;
#else
typedef float my_float;
#endif

#define COPY(x,y,z) for(uint16_t k = 0; k < z; k++) { x = y; } 

enum
{
	PV = 0,
	PVDR,
	CQPV,
};

//using parameterMap_t = std::unordered_map< std::string, std::vector<int> >;
using parameterInstanceMap_t = std::unordered_map< std::string, int>;
using parameterCombinations_t = std::unordered_map<std::string, std::vector<int> >;

void parse_arguments(int argc, char** argv, std::string& inputFilePath, std::string& outputFilePath, my_float* var);
void initializeDumpers(int& audio_ptr, int buflen, int numFrames, int hopS, std::string& variationName);
//void generateParameterCombinations(const parameterMap_t & allParams, parameterCombinations_t & paramCombs);
void runTest(std::string & inputFilePath, std::string & outputFilePath, parameterInstanceMap_t paramInstance, std::string& variationName);

#endif // MAIN_H


#ifndef MAIN_H
#define MAIN_H

#include <math.h>
#include <stdint.h>
#include <stddef.h>
#include <string>
#include <unordered_map>
#include <set>
#include <vector>
#include <variant>

#ifdef USE_DOUBLE
typedef double my_float;
#else
typedef float my_float;
#endif

enum
{
	PV = 0,
	PVDR,
	CQPV,
};

using var_t = std::variant<int, my_float, std::string>;
using parameterInstanceMap_t  = std::unordered_map<std::string, var_t >;
using parameterCombinations_t = std::unordered_map<std::string, std::vector<var_t> >;

void initializeDumpers(int& audio_ptr, int buflen, int numFrames, int hopS, std::string& variationName, std::string& fileName);
parameterCombinations_t generateParameterCombinations(parameterCombinations_t& paramCombs);
void runTest(std::string inputFilePath, std::string outputFilePath, parameterInstanceMap_t paramInstance, std::string variationName);
void printProgress();
bool runTestSuite(parameterCombinations_t& paramCombs, std::string inputFileDir, std::string outputFileDir);

#endif // MAIN_H


#pragma once
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

using var_t = std::variant<int, my_float, std::string>;
using parameterInstanceMap_t  = std::unordered_map<std::string, var_t >;
using parameterCombinations_t = std::unordered_map<std::string, std::vector<var_t> >;

int PitchEngineTs();
void initializeDumpers(int& audio_ptr, int buflen, int numFrames, int hopS, std::string& variationName, std::string& fileName);
parameterCombinations_t generateParameterCombinations(parameterCombinations_t& paramCombs);
void runTest(parameterCombinations_t& paramCombs, std::string inputFileDir, std::string outputFileDir);
void runPitchEngine(std::string inputFilePath, std::string outputFilePath,
	parameterInstanceMap_t paramInstance, std::string variationName);
std::string constructVariationName(parameterInstanceMap_t& paramInstance);
std::vector<std::string> getFailedTests(parameterCombinations_t& paramCombs, std::string testFileDir, std::string outputFileDir);


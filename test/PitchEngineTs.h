#pragma once
#include "ParameterCombinator.h"

#ifdef USE_DOUBLE
typedef double my_float;
#else
typedef float my_float;
#endif

using namespace parameterCombinator;

int PitchEngineTs();
void runPitchEngine(std::string variationName, parameterInstanceMap_t paramInstance);
std::vector<std::string> getFailedTests(ParameterCombinator& paramSet, std::string testFileDir, std::string outputFileDir);
void removeFileExtension(std::string& str);
void generateSignal(std::vector<float>& signal, parameterInstanceMap_t& paramInstance, std::string& debugFolder, int sampleRate);


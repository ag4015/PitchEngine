#pragma once
#include "ParameterCombinator.h"

#ifdef USE_DOUBLE
typedef double my_float;
#else
typedef float my_float;
#endif

using namespace parameterCombinator;

int PitchEngineTs();
void initializeDumpers(int& audio_ptr, int buflen, int numFrames, int hopS, std::string& variationName, std::string& fileName);
void runTest(ParameterCombinator& paramSet, std::string inputFileDir, std::string outputFileDir);
void runPitchEngine(std::string inputFilePath, std::string outputFilePath,
	parameterInstanceMap_t paramInstance, std::string variationName);
std::vector<std::string> getFailedTests(ParameterCombinator& paramSet, std::string testFileDir, std::string outputFileDir);
void removeFileExtension(std::string& str);
void generateSignal(std::vector<float>& signal, parameterInstanceMap_t& paramInstance, std::string& debugFolder, int sampleRate);


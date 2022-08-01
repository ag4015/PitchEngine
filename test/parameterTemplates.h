#pragma once
#include "ParameterManager.h"

ParameterManager generateExpectedDataSetFromTrainingDataSet(ParameterManager& trainingData)
{
	dontCares_t             dontCares;
	parameterTypeMap_t      parameterTypeMap;
	printableParams_t		printableParams;

	ParameterManager expectedDataSet = trainingData;

	parameterCombinations_t trainingCombs;
	const parameterCombinations_t* paramCombs  = trainingData.getParameterCombinations();

	for (const auto& freq : paramCombs->at("freq"))
	{
		for (auto& steps : paramCombs->at("steps"))
		{
			trainingCombs["freq"].push_back(std::get<int>(freq) * std::get<int>(steps));
		}
	}


	
	// List of parameters that don't affect the algorithm
	std::string dontCareKey = "steps";

	expectedDataSet.addParametersWithoutRegeneratingCombinations(trainingCombs, dontCares, dontCareKey, parameterTypeMap);

	return expectedDataSet;
}

ParameterManager generateInputFileCombinations()
{
	parameterCombinations_t paramCombs;
	dontCares_t             dontCares;
	parameterTypeMap_t      parameterTypeMap;
	printableParams_t		printableParams;

	// List of parameters to test
	paramCombs["inputFile"] = { "sine_short" };
	paramCombs["steps"]     = { 3 };
	paramCombs["hopA"]      = { 256 };
	paramCombs["algo"]      = { "pv" };
	paramCombs["magTol"]    = { 1e-6 };
	paramCombs["buflen"]    = { 1024 };

	// List of parameters that don't affect the algorithm
	dontCareKey_t dontCareKey = "algo";
	dontCares["se"]         = { "magTol" };
	dontCares["pv"]         = { "magTol" };

	// Type of each of the parameters
	parameterTypeMap["string"] = { "inputFile", "algo" };
	parameterTypeMap["double"] = { "magTol" };
	parameterTypeMap["int"]    = { "steps", "hopA", "algo", "magTol", "buflen" };

	printableParams = { "inputFile", "steps", "hopA", "algo", "buflen" };

	ParameterManager parameterManager(paramCombs, dontCares, dontCareKey, parameterTypeMap, printableParams);

	return parameterManager;
}

ParameterManager sineSweepCombinations()
{
	parameterCombinations_t paramCombs;
	dontCares_t             dontCares;
	parameterTypeMap_t      parameterTypeMap;
	printableParams_t		printableParams;

	// List of parameters to test
	paramCombs["signal"]    = { "sine" };
	paramCombs["freq"]      = { 440, 880 };
	paramCombs["steps"]     = { 3 };
	paramCombs["hopA"]      = { 256 };
	paramCombs["algo"]      = { "pv" };
	paramCombs["magTol"]    = { 1e-6 };
	paramCombs["buflen"]    = { 1024 };
	paramCombs["numSamp"]   = { 1024*120 };
	paramCombs["data"]      = { "input" };

	// List of parameters that don't affect the algorithm
	std::string dontCareKey = "algo";
	dontCares["se"]         = { "magTol" };
	dontCares["pv"]         = { "magTol" };

	// Type of each of the parameters
	parameterTypeMap["string"] = { "inputFile", "algo", "signal", "data"};
	parameterTypeMap["double"] = { "magTol" };
	parameterTypeMap["int"]    = { "freq", "steps", "hopA", "algo", "magTol", "buflen" };

	printableParams = { "signal", "steps", "algo", "buflen", "freq", "numSamp"};

	ParameterManager trainingData(paramCombs, dontCares, dontCareKey, parameterTypeMap, printableParams);
	ParameterManager expectedData = generateExpectedDataSetFromTrainingDataSet(trainingData);

	return expectedData;
}

ParameterManager trainEngineSine()
{
	parameterCombinations_t paramCombs;
	dontCares_t             dontCares;
	parameterTypeMap_t      parameterTypeMap;
	printableParams_t		printableParams;

	// List of parameters to test
	paramCombs["signal"]    = { "sine" };
	paramCombs["freq"]      = { 440 };
	paramCombs["steps"]     = { 3 };
	paramCombs["hopA"]      = { 256 };
	paramCombs["algo"]      = { "pv" };
	paramCombs["magTol"]    = { 1e-6 };
	paramCombs["buflen"]    = { 1024 };
	paramCombs["numSamp"]   = { 1024*120 };

	// List of parameters that don't affect the algorithm
	std::string dontCareKey = "algo";
	dontCares["se"]         = { "magTol" };
	dontCares["pv"]         = { "magTol" };

	// Type of each of the parameters
	parameterTypeMap["string"] = { "inputFile", "algo", "signal"};
	parameterTypeMap["double"] = { "magTol" };
	parameterTypeMap["int"]    = { "freq", "steps", "hopA", "algo", "magTol", "buflen" };

	printableParams = { "signal", "steps", "algo", "buflen", "freq", "numSamp"};

	ParameterManager paramSet(paramCombs, dontCares, dontCareKey, parameterTypeMap, printableParams);
	ParameterManager trainingSet = generateExpectedDataSetFromTrainingDataSet(paramSet);

	return paramSet;
}

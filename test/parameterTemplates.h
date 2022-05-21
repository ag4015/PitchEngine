#pragma once
#include "ParameterCombinations.h"

ParameterCombinations generateInputFileCombinations()
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
	std::string dontCareKey = "algo";
	dontCares["se"]         = { "magTol" };
	dontCares["pv"]         = { "magTol" };

	// Type of each of the parameters
	parameterTypeMap["string"] = { "inputFile", "algo" };
	parameterTypeMap["double"] = { "magTol" };
	parameterTypeMap["int"]    = { "steps", "hopA", "algo", "magTol", "buflen" };

	printableParams = { "inputFile", "steps", "hopA", "algo", "buflen" };

	ParameterCombinations paramSet(paramCombs, dontCares, dontCareKey, parameterTypeMap, printableParams);

	return paramSet;
}

ParameterCombinations sineSweepCombinations()
{
	parameterCombinations_t paramCombs;
	dontCares_t dontCares;
	parameterTypeMap_t parameterTypeMap;
	printableParams_t		printableParams;

	// List of parameters to test
	paramCombs["signal"]    = { "sine" };
	paramCombs["freq"]      = { 440, 880 };
	paramCombs["steps"]     = { 3 };
	paramCombs["hopA"]      = { 256 };
	paramCombs["algo"]      = { "pv" };
	paramCombs["magTol"]    = { 1e-6 };
	paramCombs["buflen"]    = { 1024 };

	// List of parameters that don't affect the algorithm
	std::string dontCareKey = "algo";
	dontCares["se"]         = { "magTol" };
	dontCares["pv"]         = { "magTol" };

	// Type of each of the parameters
	parameterTypeMap["string"] = { "inputFile", "algo", "signal"};
	parameterTypeMap["double"] = { "magTol" };
	parameterTypeMap["int"]    = { "freq", "steps", "hopA", "algo", "magTol", "buflen" };

	printableParams = { "signal", "steps", "algo", "buflen", "freq"};

	ParameterCombinations paramSet(paramCombs, dontCares, dontCareKey, parameterTypeMap, printableParams);

	return paramSet;
}

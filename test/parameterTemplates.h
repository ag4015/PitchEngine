#pragma once
#include "ParameterCombinator.h"
#include "pe_common_defs.h"

using namespace parameterCombinator;

ParameterCombinator generateExpectedDataSetFromTrainingDataSet(parameterCombinations_t& paramCombs)
{
	dontCares_t             dontCares;
	ParameterCombinator     expectedDataSet;
	parameterCombinations_t expectedCombs;

	// The expected frequency is the pitch shifted version of the original frequency
	for (auto& freqParam : paramCombs["freq"])
	{
		int freq = getVal<int>(freqParam);
		for (auto& stepsParam : paramCombs["steps"])
		{
			int steps = getVal<int>(stepsParam);
			expectedCombs["freq"].push_back(freq * POW(2, (steps / SEMITONES_PER_OCTAVE)));
		}
	}

	expectedCombs["numSamp"] = paramCombs["numSamp"];
	expectedCombs["data"]    = { "training" };

	expectedDataSet.combine(expectedCombs, dontCares);

	return expectedDataSet;
}

ParameterCombinator generateInputFileCombinations()
{
	parameterCombinations_t paramCombs;

	// List of parameters to test
	paramCombs["inputFile"] = { "sine_short" };
	paramCombs["steps"]     = { 3 };
	paramCombs["hopA"]      = { 256 };
	paramCombs["algo"]      = { "pv", "trainNN"};
	paramCombs["magTol"]    = { 1e-6 };
	paramCombs["buflen"]    = { 1024 };

	// List of parameters that don't affect the algorithm
	dontCares_t dontCares = { {"algo", { {"trainNN", {"magTol"} }, {"se", {"magTol"} }, {"pv", {"magTol"} } } } };

	ParameterCombinator parameterCombinator;
	parameterCombinator.combine(paramCombs, dontCares);

	return parameterCombinator;
}

ParameterCombinator sineSweepCombinations()
{
	parameterCombinations_t paramCombs;

	// List of parameters to test
	paramCombs["signal"]  = { "sine" };
	paramCombs["freq"]    = { 440 };
	paramCombs["steps"]   = { 3 };
	paramCombs["hopA"]    = { 256 };
	paramCombs["algo"]    = { "pv" };
	paramCombs["magTol"]  = { 1e-6 };
	paramCombs["buflen"]  = { 1024 };
	paramCombs["numSamp"] = { 1024*120 };
	paramCombs["data"]    = { "input" };

	// List of parameters that don't affect the algorithm
	dontCares_t dontCares = { {"algo", { {"se", {"magTol"} }, {"pv", {"magTol"} } } } };

	ParameterCombinator parameterCombinator;
	parameterCombinator.combine(paramCombs, dontCares);

	ParameterCombinator expectedDataCombinator = generateExpectedDataSetFromTrainingDataSet(paramCombs);

	// Add the two combinators as they are, no recombination
	dontCares.clear();
	parameterCombinator.addCombinations(parameterCombinator, expectedDataCombinator, dontCares);

	return parameterCombinator;
}

ParameterCombinator trainEngineSine()
{
	parameterCombinations_t paramCombs;

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
	dontCares_t dontCares = { {"algo", { {"se", {"magTol"} }, {"pv", {"magTol"} } } } };

	ParameterCombinator parameterCombinator;
	parameterCombinator.combine(paramCombs, dontCares);

	ParameterCombinator trainingSet = generateExpectedDataSetFromTrainingDataSet(paramCombs);

	return parameterCombinator;
}

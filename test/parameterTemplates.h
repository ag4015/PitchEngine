#pragma once
#include "ParameterCombinator.h"

using namespace parameterCombinator;

ParameterCombinator generateExpectedDataSetFromTrainingDataSet(ParameterCombinator& trainingData)
{
	//dontCares_t             dontCares;

	ParameterCombinator expectedDataSet = trainingData;

	//parameterCombinations_t trainingCombs;
	//const parameterCombinations_t* paramCombs  = trainingData.getParameterCombinations();

	//for (const auto& freq : paramCombs->at("freq"))
	//{
	//	for (auto& steps : paramCombs->at("steps"))
	//	{
	//		trainingCombs["freq"].push_back(static_cast<int>(std::get<int>(freq) * POW(2, (std::get<int>(steps)/ SEMITONES_PER_OCTAVE ))));
	//	}
	//}

	//// List of parameters that don't affect the algorithm
	//std::string dontCareKey = "steps";

	//expectedDataSet.addParametersWithoutRecombining(trainingCombs, dontCares, dontCareKey, parameterTypeMap);

	//return expectedDataSet;
	return trainingData;
}

ParameterCombinator generateInputFileCombinations()
{
	parameterCombinations_t paramCombs;

	// List of parameters to test
	paramCombs["inputFile"] = { "sine_short" };
	paramCombs["steps"]     = { 3, 12 };
	paramCombs["hopA"]      = { 256 };
	paramCombs["algo"]      = { "pv" };
	paramCombs["magTol"]    = { 1e-6 };
	paramCombs["buflen"]    = { 1024 };

	// List of parameters that don't affect the algorithm
	dontCares_t dontCares = { {"algo", { {"se", {"magTol"} }, {"pv", {"magTol"} } } } };

	ParameterCombinator parameterCombinator;
	parameterCombinator.combine(paramCombs, dontCares);

	return parameterCombinator;
}

ParameterCombinator sineSweepCombinations()
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
	paramCombs["data"]      = { "input" };

	// List of parameters that don't affect the algorithm
	dontCares_t dontCares = { {"algo", { {"se", {"magTol"} }, {"pv", {"magTol"} } } } };

	ParameterCombinator parameterCombinator;
	parameterCombinator.combine(paramCombs, dontCares);

	ParameterCombinator expectedData = generateExpectedDataSetFromTrainingDataSet(parameterCombinator);

	return expectedData;
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

	ParameterCombinator trainingSet = generateExpectedDataSetFromTrainingDataSet(parameterCombinator);

	return parameterCombinator;
}

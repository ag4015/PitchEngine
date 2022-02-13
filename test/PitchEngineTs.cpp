
#include "PitchEngineTs.h"
#include "wavio.h"
#include "stdint.h"
#include "CQPVEngine.h"
#include "PVDREngine.h"
#include "PVEngine.h"
#include "PitchEngine.h"
#include "logger.h"
#include "DumperContainer.h"
#include "TimerContainer.h"
#include "ProgressBarContainer.h"
#include <time.h>
#include <cmath>
#ifdef WIN32
#include <filesystem>
#else
#include <experimental/filesystem>
#endif
#include <memory>
#include <thread>
#include <queue>
#ifdef USE_MULTITHREADING
#define NUM_THREADS 7
#else
#define NUM_THREADS 1
#endif

#define INGAIN     1
#define OUTGAIN    1

DumperContainer* DumperContainer::instance = 0;
TimerContainer* TimerContainer::instance = 0;
ProgressBarContainer* ProgressBarContainer::instance = 0;

int PitchEngineTs()
{
	parameterCombinations_t paramCombs;

	paramCombs["inputFile"] = { "sine_short" };
	paramCombs["steps"]     = { 0, 3 };
	paramCombs["hopA"]      = { 256, 512, 1024 };
	paramCombs["algo"]      = { "pv" };
	paramCombs["magTol"]    = { 1e-4 };
	paramCombs["buflen"]    = { 1024, 2048, 4096 };

	paramCombs = generateParameterCombinations(paramCombs);
	runTest(paramCombs, INPUT_AUDIO_DIR, OUTPUT_AUDIO_DIR);

	std::vector<std::string> failedTests = getFailedTests(paramCombs, TEST_AUDIO_DIR, OUTPUT_AUDIO_DIR);

	for (auto& failedTest : failedTests)
	{
		std::cout << "Test: " << failedTest << " failed." << std::endl;
	}

	return failedTests.size() != 0;

}

std::string constructVariationName(parameterInstanceMap_t& paramInstance)
{
	std::string variationName;
	for (auto [paramName, paramValue] : paramInstance) {

		if (paramName == "inputFile")
		{
			variationName += std::get<std::string>(paramValue) + "_";
		}
		else if (paramName == "algo")
		{
			variationName += paramName + "_" + std::get<std::string>(paramValue) + "_";
		} 
		else if (paramName == "magTol")
		{
			std::stringstream ss;
			ss << std::get<my_float>(paramValue) << std::scientific;
			variationName += paramName + "_" + ss.str() + "_";
		}
		else
		{
			variationName += paramName + "_" + std::to_string(std::get<int>(paramValue)) + "_";
		}
	}

	// Remove trailing "_"
	size_t lastIndex = variationName.find_last_of("_");
	variationName = variationName.substr(0, lastIndex);

	return variationName;

}

void runTest(parameterCombinations_t& paramCombs, std::string inputFileDir, std::string outputFileDir)
{

	std::queue<std::thread> threadQ;

#ifdef USE_PROGRESS_BAR and NUM_THREADS > 1
	threadQ.push(std::thread{ printProgress });
#endif

	for (size_t paramIdx = 0; paramIdx < paramCombs["buflen"].size(); paramIdx++)
	{

		std::string outputFilePath{ outputFileDir };
		std::string inputFilePath{ inputFileDir };

		// Select the parameters to use for this iteration of the test
		parameterInstanceMap_t paramInstance;
		for (auto& [paramName, paramValues] : paramCombs) {
			paramInstance[paramName] = paramValues[paramIdx];
		}

		std::string inputFileName =  std::get<std::string>(paramInstance["inputFile"]);

		// Remove possible file extension ".wav"
		size_t lastIndex = inputFileName.find_last_of(".");
		if (lastIndex != std::string::npos) {
			inputFileName = inputFileName.substr(0, lastIndex);
		}

		outputFilePath += inputFileName;
		inputFilePath += inputFileName + ".wav";

		// Create directory for this test case
		outputFilePath += "/";

		std::filesystem::create_directory(outputFilePath);

		std::string variationName = constructVariationName(paramInstance);

		outputFilePath += variationName + ".wav";

		threadQ.push(std::thread{ runPitchEngine, inputFilePath, outputFilePath, paramInstance, variationName });
		if (threadQ.size() + 1 >= NUM_THREADS)
		{
			threadQ.front().join();
			threadQ.pop();
		}
	}
	while (threadQ.size() > 0)
	{
		threadQ.front().join();
		threadQ.pop();
	}
}

void runPitchEngine(std::string inputFilePath, std::string outputFilePath, parameterInstanceMap_t paramInstance, std::string variationName)
{
	PRINT_LOG("Test ", variationName);
	CREATE_TIMER("runPitchEngine", timeUnit::MILISECONDS);

	int audio_ptr   = 0;                       // Wav file sample pointer
	int buflen      = std::get<int>(paramInstance["buflen"]);
	int steps       = std::get<int>(paramInstance["steps"]);
	int hopA        = std::get<int>(paramInstance["hopA"]);
	//int hopA        = (1.0 - (ANALYSIS_FRAME_OVERLAP / 100.0)) * buflen;
	my_float magTol = std::get<my_float>(paramInstance["magTol"]);
	int sampleRate  = 44100;

	my_float shift = POW(2, (steps/12));
	int hopS       = static_cast<int>(ROUND(hopA * shift));
	int numFrames  = static_cast<int>(buflen / hopA);

	// Load contents of wave file
	std::vector<float> in_audio = readWav(inputFilePath);
	std::vector<float> out_audio(in_audio.size(), 0.0);

	INITIALIZE_DUMPERS(audio_ptr, buflen, numFrames, hopS, variationName, std::get<std::string>(paramInstance["inputFile"]));

	PROGRESS_BAR_CREATE(variationName, static_cast<int>(in_audio.size() / buflen));
	
	std::unique_ptr<PitchEngine> pe;
	std::string& algo = std::get<std::string>(paramInstance["algo"]);
	if (algo == "pv")
	{
		pe = std::make_unique<PVEngine>(steps, buflen, hopA);
	}
	else if (algo == "pvdr")
	{
		pe = std::make_unique<PVDREngine>(steps, buflen, hopA, magTol);
	}
	else if (algo == "cqpv")
	{
		pe = std::make_unique<CQPVEngine>(steps, buflen, hopA, sampleRate, magTol);
	}

	while(audio_ptr < (in_audio.size() - buflen))
	{
		for (int k = 0; k < buflen; k++)
		{
			pe->inbuffer_[k] = in_audio[audio_ptr + k] * INGAIN;
		}

		PROGRESS_BAR_PROGRESS(variationName);

		pe->process();

		for (int k = 0; k < buflen; k++)
		{
			out_audio[audio_ptr + k] = pe->outbuffer_[k] * OUTGAIN;

			// Avoid uint16_t overflow and clip the signal instead.
			if (std::abs(out_audio[audio_ptr + k]) > 1)
			{
				out_audio[audio_ptr + k] = (out_audio[audio_ptr + k] < 0) ? -1.0 : 1.0;
			}
		}
		audio_ptr += buflen;
	}

	PROGRESS_BAR_FINISH(variationName);

	writeWav(out_audio, inputFilePath, outputFilePath);

	DUMP_TIMINGS("timings.csv");

}

void initializeDumpers(int& audio_ptr, int buflen, int numFrames, int hopS, std::string& variationName, std::string& fileName)
{
	// Remove possible file extension ".wav"
	std::string debugPath{ DEBUG_DIR };
	debugPath += fileName;
	size_t lastIndex = debugPath.find_last_of(".");
	if (lastIndex != std::string::npos) {
		debugPath = debugPath.substr(0, lastIndex);
	}

	// Create directory for this test case
	debugPath += "/";
#ifdef WIN32
	std::filesystem::create_directory(debugPath);
#else
	std::experimental::filesystem::create_directory(debugPath);
#endif

	CREATE_DUMPER_C0NTAINER(debugPath);
	UPDATE_DUMPER_CONTAINER_PATH(debugPath + variationName + "/");
	//INIT_DUMPER("inframe.csv"   , audio_ptr, buflen, buflen, 40, -1);
	//INIT_DUMPER("outframe.csv"  , audio_ptr, buflen, buflen, 40, -1);
	//INIT_DUMPER("mag.csv"       , audio_ptr, buflen, buflen, -1, -1);
	//INIT_DUMPER("phi_a.csv"     , audio_ptr, buflen, buflen, -1, -1);
	//INIT_DUMPER("phi_s.csv"     , audio_ptr, buflen, buflen, -1, -1);
	//INIT_DUMPER("phi_sPrev.csv" , audio_ptr, buflen, buflen, -1, -1);
	//INIT_DUMPER("cpxIn.csv"     , audio_ptr, buflen, buflen,  40, -1);
	//INIT_DUMPER("cpxOut.csv"    , audio_ptr, buflen, buflen,  40, -1);
	//INIT_DUMPER("inbuffer.csv"  , audio_ptr, buflen, buflen, 40, -1);
	//INIT_DUMPER("outbuffer.csv" , audio_ptr, buflen, buflen, 10, -1);
	//INIT_DUMPER("inwin.csv"     , audio_ptr, buflen, buflen, 40, -1);
	//INIT_DUMPER("outwin.csv"    , audio_ptr, buflen, buflen, 40, -1);
	//INIT_DUMPER("delta_f.csv"   , audio_ptr, buflen, buflen, -1, -1);
	//INIT_DUMPER("delta_t.csv"   , audio_ptr, buflen, buflen, -1, -1);
	//INIT_DUMPER("vTime.csv"     , audio_ptr, numFrames*hopS*2, numFrames*hopS*2, 40, -1);
}

void CartesianRecurse(std::vector<std::vector<int64_t>> &accum, std::vector<int64_t> stack,
	std::vector<std::vector<int64_t>> sequences,int64_t index)
{
	std::vector<int64_t> sequence = sequences[index];
	for (int64_t i : sequence)
	{
		stack.push_back(i);
		if (index == 0) {
			accum.push_back(stack);
		}
		else {
			CartesianRecurse(accum, stack, sequences, index - 1);
		}
		stack.pop_back();
	}
}
std::vector<std::vector<int64_t>> CartesianProduct(std::vector<std::vector<int64_t>>& sequences)
{
	std::vector<std::vector<int64_t>> accum;
	std::vector<int64_t> stack;
	if (sequences.size() > 0) {
		CartesianRecurse(accum, stack, sequences, sequences.size() - 1);
	}
	return accum;
}

parameterCombinations_t generateParameterCombinations(parameterCombinations_t& paramCombs)
{
	// Convert parameterCombinations_t to a vector of vector of ints
	std::vector<std::vector<int64_t>> sequences;
	for (auto& param : paramCombs) {
		std::vector<int64_t> seq;
		if (param.first == "inputFile" || param.first == "algo")
		{
			for (auto& val : param.second) {
				std::string* strAddress = &std::get<std::string>(val);
				seq.push_back(reinterpret_cast<int64_t&>(strAddress));
			}
		}
		else
		{
			for (auto& val : param.second) {
				seq.push_back(reinterpret_cast<int64_t&>(val));
			}
		}
		sequences.push_back(seq);
	}

	std::vector<std::vector<int64_t>> res = CartesianProduct(sequences);
	// Eliminate duplicates
	std::set<std::vector<int64_t>> resSet;
	for (auto& v : res) {
		resSet.insert(v);
	}
	parameterCombinations_t newParamCombs;
	for (auto& set : resSet) {
		int i = 0;
		for (auto& param : paramCombs) {
			// Reverse the set and insert
			if (param.first == "magTol")
			{
				int64_t a = static_cast<int64_t>(set[set.size() - i - 1]);
				my_float b = reinterpret_cast<my_float&>(a);
				newParamCombs[param.first].push_back(b);
			}
			else if (param.first == "inputFile" || param.first == "algo")
			{
				int64_t a = static_cast<int64_t>(set[set.size() - i - 1]);
				std::string* b = reinterpret_cast<std::string*>(a);
				newParamCombs[param.first].push_back(*b);
			}
			else
			{
				auto a = static_cast<int>(set[set.size() - i - 1]);
				newParamCombs[param.first].push_back(a);
			}
			i++;
		}
	}
	return newParamCombs;
}

std::vector<std::string> getFailedTests(parameterCombinations_t& paramCombs, std::string testFileDir, std::string outputFileDir)
{
	std::vector<std::string> failedTests;
	for (size_t paramIdx = 0; paramIdx < paramCombs["buflen"].size(); paramIdx++)
	{

		std::string testFilePath{ testFileDir };
		std::string outputFilePath{ outputFileDir };

		parameterInstanceMap_t paramInstance;
		for (auto& [paramName, paramValues] : paramCombs) {
			paramInstance[paramName] = paramValues[paramIdx];
		}

		std::string inputFileName =  std::get<std::string>(paramInstance["inputFile"]);

		// Remove possible file extension ".wav"
		size_t lastIndex = inputFileName.find_last_of(".");
		if (lastIndex != std::string::npos) {
			inputFileName = inputFileName.substr(0, lastIndex);
		}

		std::string variationName = constructVariationName(paramInstance);

		outputFilePath += inputFileName;
		testFilePath   += inputFileName;
		outputFilePath += "/";
		testFilePath   += "/";
		outputFilePath += variationName + ".wav";
		testFilePath   += variationName + ".wav";

		if (!std::filesystem::exists(outputFilePath) || !std::filesystem::exists(testFilePath))
		{
			continue;
		}

		std::vector<float> output_audio = readWav(outputFilePath);
		std::vector<float> test_audio = readWav(testFilePath);

		if (output_audio.size() != test_audio.size())
		{
			failedTests.push_back(variationName);
			continue;
		}
		bool failed = false;
		for (int i = 0; i < output_audio.size() && !failed; i++)
		{
			if (output_audio[i] != test_audio[i])
			{
				failedTests.push_back(variationName);
				failed = true;
			}
		}
	}
	return failedTests;
}

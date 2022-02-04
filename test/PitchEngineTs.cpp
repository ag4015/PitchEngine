
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
#ifdef USE_MULTITHREADING
#include <thread>
#endif

#define INGAIN     1
#define OUTGAIN    1

DumperContainer* DumperContainer::instance = 0;
TimerContainer* TimerContainer::instance = 0;
ProgressBarContainer* ProgressBarContainer::instance = 0;

int PitchEngineTs()
{
	parameterCombinations_t paramCombs;

	paramCombs["inputFile"] = { "sine_short"};
	paramCombs["steps"]     = { 12, 13 };
	paramCombs["algo"]      = { "pv" };
	paramCombs["magTol"]    = { 1e-4 };
	paramCombs["buflen"]    = { 4096 };

	paramCombs = generateParameterCombinations(paramCombs);
	runTest(paramCombs, INPUT_AUDIO_DIR, OUTPUT_AUDIO_DIR);

	int result = 0;

	return result;

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

#ifdef USE_MULTITHREADING
	std::vector<std::thread> vecThread;
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
#ifdef WIN32
		std::filesystem::create_directory(outputFilePath);
#else
		std::experimental::filesystem::create_directory(outputFilePath);
#endif

		std::string variationName = constructVariationName(paramInstance);

		outputFilePath += variationName + ".wav";

#ifdef USE_MULTITHREADING
		vecThread.push_back(std::thread{ runPitchEngine, inputFilePath, outputFilePath, paramInstance, variationName });
	}
#ifdef USE_PROGRESS_BAR
	vecThread.push_back(std::thread{ printProgress });
#endif
	for (auto& thread : vecThread) {
		thread.join();
	}
#else
		runPitchEngine(inputFilePath, outputFilePath, paramInstance, variationName);
	}
#endif
}

void runPitchEngine(std::string inputFilePath, std::string outputFilePath, parameterInstanceMap_t paramInstance, std::string variationName)
{
	PRINT_LOG("Test ", variationName);
	CREATE_TIMER("runPitchEngine", timeUnit::MILISECONDS);

	int audio_ptr   = 0;                       // Wav file sample pointer
	int buflen      = std::get<int>(paramInstance["buflen"]);
	int steps       = std::get<int>(paramInstance["steps"]);
	int hopA        = (1.0 - (ANALYSIS_FRAME_OVERLAP / 100.0)) * buflen;
	my_float magTol = std::get<my_float>(paramInstance["magTol"]);
	int sampleRate  = 44100;
	int numSamp     = 0;

	my_float shift = POW(2, (steps/12));
	int hopS       = static_cast<int>(ROUND(hopA * shift));
	int numFrames  = static_cast<int>(buflen / hopA);

	// Load contents of wave file
	my_float* in_audio = readWav(numSamp, inputFilePath);                            // Get input audio from wav file and fetch the size

	my_float* out_audio = (my_float *)  calloc(numSamp, sizeof(my_float));

	PRINT_LOG("Input file: %s\n",  inputFilePath);
	PRINT_LOG("Output file: %s\n", outputFilePath);

	INITIALIZE_DUMPERS(audio_ptr, buflen, numFrames, hopS, variationName, std::get<std::string>(paramInstance["inputFile"]));

	PROGRESS_BAR_CREATE(variationName, static_cast<int>(numSamp / buflen));
	
	PRINT_LOG("Buffer length: %i.\n", buflen);

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

	while(audio_ptr < (numSamp - buflen))
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

	// Reconvert floating point audio to 16bit
	//for (int i = 0; i < numSamp; i++)
	//{
	//	out_audio[i] = out_audio[i] * MAXVAL16;
	//}

	// Save the processed audio to the output file
	writeWav(out_audio, inputFilePath, outputFilePath, numSamp);

	// Deallocate memory
	free(in_audio);
	free(out_audio);

	END_TIMER("runPitchEngine")
	DUMP_TIMINGS("timings.csv")

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

void printProgress()
{
	while (!ProgressBarContainer::getProgressBarContainer()->allFinished() ||
		ProgressBarContainer::getProgressBarContainer()->getProgressBarMap().size() == 0)
	{
		ProgressBarContainer::getProgressBarContainer()->print();
	}
}


#include "main.h"
#include "wavio.h"
#include "stdint.h"
#include "CQPVEngine.h"
#include "PVDREngine.h"
#include "PVEngine.h"
#include "PitchEngine.h"
#include "logger.h"
#include "DumperContainer.h"
#include "TimerContainer.h"
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

 //#define DEFAULT_INPUT_FILENAME "constant_guitar_short.wav"
#define DEFAULT_INPUT_FILENAME "sine_short.wav"
#define DEFAULT_OUTPUT_FILENAME "output.wav"

#define INGAIN     1
#define OUTGAIN    1

DumperContainer* DumperContainer::instance = 0;
TimerContainer* TimerContainer::instance = 0;


int main(int argc, char **argv)
{

	parameterCombinations_t paramCombs;
	my_float var          = 0;

	std::string inputFilePath{ INPUT_AUDIO_DIR DEFAULT_INPUT_FILENAME };
	std::string outputFilePath{ OUTPUT_AUDIO_DIR DEFAULT_INPUT_FILENAME };

	parse_arguments(argc, argv, inputFilePath, outputFilePath, &var);

	paramCombs["steps"]  = {   12,   12,   12,   12,   12,   12 };
	paramCombs["algo"]   = { PVDR, PVDR, PVDR, PVDR, PVDR, PVDR };
	paramCombs["hopS"]   = {  256,  256,  256,  256,  256,  256 };
	paramCombs["hopA"]   = {  256,  512,  256,  512,  256,  512 };
	paramCombs["buflen"] = { 1024, 1024, 2048, 2048, 4096, 4096 };

	//paramCombs["buflen"] = { 1024 };
	//paramCombs["hopA"]   = { 256  };
	//paramCombs["hopS"]   = { 512  };
	//paramCombs["algo"]   = { PV   };
	//paramCombs["steps"]  = { 2   };

	paramCombs = generateParameterCombinations(paramCombs);

	std::string originalOutputFilePath = outputFilePath;

#ifdef USE_MULTITHREADING
	std::vector<std::thread> vecThread;
#endif

	for (size_t paramIdx = 0; paramIdx < paramCombs["buflen"].size(); paramIdx++)
	{
		// Reset outputFilePath
		outputFilePath = originalOutputFilePath;

		// Select the parameters to use for this iteration of the test
		parameterInstanceMap_t paramInstance;
		for (auto& [paramName, paramValues] : paramCombs) {
			paramInstance[paramName] = paramValues[paramIdx];
		}
		std::string variationName;
		for (auto& [paramName, paramValue] : paramInstance) {
			if (paramName == "algo") {
				std::string algorithmName;
				switch (paramValue) {
				case(PV):
					algorithmName = "pv";
					break;
				case(PVDR):
					algorithmName = "pvdr";
					break;
				case(CQPV):
					algorithmName = "cqpv";
					break;
				}
				variationName += paramName + "_" + algorithmName + "_";
			} else {
				variationName += paramName + "_" + std::to_string(paramValue) + "_";
			}
		}

		// Remove trailing "_"
		size_t lastIndex = variationName.find_last_of("_");
		variationName = variationName.substr(0, lastIndex);

		// Remove possible file extension ".wav"
		lastIndex = outputFilePath.find_last_of(".");
		if (lastIndex != std::string::npos) {
			outputFilePath = outputFilePath.substr(0, lastIndex);
		}

		// Create directory for this test case
		outputFilePath += "/";
#ifdef WIN32
		std::filesystem::create_directory(outputFilePath);
#else
		std::experimental::filesystem::create_directory(outputFilePath);
#endif
		outputFilePath += variationName + ".wav";

#ifdef USE_MULTITHREADING
		vecThread.push_back(std::thread{ runTest, inputFilePath, outputFilePath, paramInstance, variationName });
	}
	for (auto& thread : vecThread) {
		thread.join();
	}
#else
		runTest(inputFilePath, outputFilePath, paramInstance, variationName);
	}
#endif
	return 0;
}

void runTest(std::string& inputFilePath, std::string& outputFilePath, parameterInstanceMap_t paramInstance, std::string& variationName)
{
	std::cout << "Test " << variationName << std::endl;
	CREATE_TIMER("runTest", timeUnit::MILISECONDS);

	int audio_ptr  = 0;                       // Wav file sample pointer
	int buflen     = paramInstance["buflen"];
	int steps      = paramInstance["steps"];
	int hopA       = paramInstance["hopA"];
	int sampleRate = 44100;
	int numSamp    = 0;

	my_float shift = POW(2, (steps/12));
	int hopS       = static_cast<int>(ROUND(HOPA * shift));
	int numFrames  = static_cast<int>(buflen / HOPA);

	// Load contents of wave file
	my_float* in_audio = readWav(numSamp, inputFilePath);                            // Get input audio from wav file and fetch the size

	my_float* out_audio = (my_float *)  calloc(numSamp, sizeof(my_float));

	PRINT_LOG("Input file: %s\n",  inputFilePath);
	PRINT_LOG("Output file: %s\n", outputFilePath);

	INITIALIZE_DUMPERS(audio_ptr, buflen, numFrames, hopS, variationName);
	
	PRINT_LOG("Buffer length: %i.\n", buflen);

	std::unique_ptr<PitchEngine> pe;
	switch (paramInstance["algo"])
	{
	case(PV):
		pe = std::make_unique<PVEngine>(steps, buflen, hopA);
		break;
	case(PVDR):
		pe = std::make_unique<PVDREngine>(steps, buflen, hopA);
		break;
	case(CQPV):
		pe = std::make_unique<CQPVEngine>(steps, buflen, hopA, sampleRate);
		break;
	}

	while(audio_ptr < (numSamp - buflen))
	{
		for (int k = 0; k < buflen; k++)
		{
			pe->inbuffer_[k] = in_audio[audio_ptr + k] * INGAIN;
		}

		printf("\r%i%%", 100 * audio_ptr/numSamp);

		CREATE_TIMER("process_buffer", timeUnit::MILISECONDS);
		pe->process();
		END_TIMER("process_buffer");

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

	// Reconvert floating point audio to 16bit
	//for (int i = 0; i < numSamp; i++)
	//{
	//	out_audio[i] = out_audio[i] * MAXVAL16;
	//}

	// Save the processed audio to the output file
	writeWav(out_audio, inputFilePath, outputFilePath, numSamp);

	// Deallocate memory
	END_TIMER("runTest")
	DUMP_TIMINGS("timings.csv")

	free(in_audio);
	free(out_audio);
}

void parse_arguments(int argc, char** argv, std::string&  inputFilePath, std::string& outputFilePath, my_float* var)
{
	if (argc > 1) {
		inputFilePath = argv[1];
	}
	if (argc > 2) {
		outputFilePath = argv[2];
	}
	if (argc > 3)
	{
#ifdef USE_DOUBLE
		*var = atof(argv[3]);
#else
		char* tmpPtr;
		*var = strtof(argv[3], &tmpPtr);
#endif
	}
}

void initializeDumpers(int& audio_ptr, int buflen, int numFrames, int hopS, std::string& variationName)
{
	// Remove possible file extension ".wav"
	std::string debugPath{ DEBUG_DIR DEFAULT_INPUT_FILENAME };
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
	//INIT_DUMPER("mag.csv"      , audio_ptr, buflen, buflen, -1, -1);
	//INIT_DUMPER("phi_a.csv"    , audio_ptr, buflen, buflen, -1, -1);
	//INIT_DUMPER("phi_s.csv"    , audio_ptr, buflen, buflen, -1, -1);
	//INIT_DUMPER("phi_sPrev.csv", audio_ptr, buflen, buflen, -1, -1);
	//INIT_DUMPER("cpxIn.csv"    , audio_ptr, buflen, buflen,  40, -1);
	//INIT_DUMPER("cpxOut.csv"   , audio_ptr, buflen, buflen,  40, -1);
	//INIT_DUMPER("inbuffer.csv" , audio_ptr, buflen, buflen, 40, -1);
	//INIT_DUMPER("outbuffer.csv", audio_ptr, buflen, buflen, 10, -1);
	//INIT_DUMPER("inwin.csv"    , audio_ptr, buflen, buflen, 40, -1);
	//INIT_DUMPER("outwin.csv"   , audio_ptr, buflen, buflen, 40, -1);
	//INIT_DUMPER("delta_f.csv"  , audio_ptr, buflen, buflen, -1, -1);
	//INIT_DUMPER("delta_t.csv"  , audio_ptr, buflen, buflen, -1, -1);
	//INIT_DUMPER("vTime.csv"    , audio_ptr, numFrames*hopS*2, numFrames*hopS*2, 40, -1);
}

void CartesianRecurse(std::vector<std::vector<int>> &accum, std::vector<int> stack,
	std::vector<std::vector<int>> sequences, int index)
{
	std::vector<int> sequence = sequences[index];
	for (int i : sequence)
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
std::vector<std::vector<int>> CartesianProduct(std::vector<std::vector<int>>& sequences)
{
	std::vector<std::vector<int>> accum;
	std::vector<int> stack;
	if (sequences.size() > 0) {
		CartesianRecurse(accum, stack, sequences, sequences.size() - 1);
	}
	return accum;
}

parameterCombinations_t generateParameterCombinations(parameterCombinations_t& paramCombs)
{
	// Convert parameterCombinations_t to a vector of vector of ints
	std::vector<std::vector<int>> sequences;
	for (auto& param : paramCombs) {
		std::vector<int> seq;
		for (auto val : param.second) {
			seq.push_back(val);
		}
		sequences.push_back(seq);
	}

	std::vector<std::vector<int>> res = CartesianProduct(sequences);
	// Eliminate duplicates
	std::set<std::vector<int>> resSet;
	for (auto& v : res) {
		resSet.insert(v);
	}
	parameterCombinations_t newParamCombs;
	for (auto& set : resSet) {
		int i = 0;
		for (auto& param : paramCombs) {
			// Reverse the set and insert
			newParamCombs[param.first].push_back(set[set.size() - i - 1]);
			i++;
		}
	}
	return newParamCombs;
}


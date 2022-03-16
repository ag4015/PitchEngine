
#include "PitchEngineTs.h"
#include "wavio.h"
#include "stdint.h"
#include "StrechEngine.h"
#include "CQPVEngine.h"
#include "PVDREngine.h"
#include "PVEngine.h"
#include "PitchEngine.h"
#include "logger.h"
#include "DumperContainer.h"
#include "TimerContainer.h"
#include "ProgressBarContainer.h"
#include "parameterValidation.h"
#include "../deps/waveform-generator/WaveformGenerator/waveform_generator.h"
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
#define NUM_THREADS 32
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
	dontCares_t dontCares;

	// List of parameters to test
	paramCombs["inputFile"] = { "signal_sine" };
	paramCombs["steps"]     = { 3 };
	paramCombs["hopA"]      = { 256 };
	paramCombs["algo"]      = { "pv", "pvdr" };
	paramCombs["magTol"]    = { 1e-6 };
	paramCombs["buflen"]    = { 1024 };

	// List of parameters that don't affect the algorithm
	dontCares["se"]   = { "magTol" };
	dontCares["pv"]   = { "magTol" };
	dontCares["pvdr"] = {};

	parameterInstanceSet_t paramInstanceSet = generateParameterInstanceSet(paramCombs, dontCares);

	auto t1 = std::chrono::high_resolution_clock::now();
	runTest(paramInstanceSet, INPUT_AUDIO_DIR, OUTPUT_AUDIO_DIR);
	auto t2 = std::chrono::high_resolution_clock::now();
	auto exTime = std::chrono::duration_cast<std::chrono::seconds>(t2 - t1);
	std::cout << "Test took " << exTime.count() << "s." << std::endl;

	std::vector<std::string> failedTests = getFailedTests(paramInstanceSet, TEST_AUDIO_DIR, OUTPUT_AUDIO_DIR);

	for (auto& failedTest : failedTests)
	{
		std::cout << "Test: " << failedTest << " failed." << std::endl;
	}

	return failedTests.size() != 0;

}

void runTest(parameterInstanceSet_t& paramInstanceSet, std::string inputFileDir, std::string outputFileDir)
{

	std::queue<std::thread> threadQ;

#ifdef USE_PROGRESS_BAR and NUM_THREADS > 1
	threadQ.push(std::thread{ printProgress });
#endif

	for (auto& paramInstance : paramInstanceSet)
	{

		std::string outputFilePath{ outputFileDir };
		std::string inputFilePath{ inputFileDir };

		std::string inputFileName =  std::get<std::string>(paramInstance.at("inputFile"));

		// Remove possible file extension ".wav"
		size_t lastIndex = inputFileName.find_last_of(".");
		if (lastIndex != std::string::npos) {
			inputFileName = inputFileName.substr(0, lastIndex);
		}

		outputFilePath += inputFileName;
		if (inputFileName.rfind("signal_", 0) != std::string::npos)
		{
			inputFilePath += GENERATED_INPUT_AUDIO_FOLDER_NAME;
		}
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

	int audio_ptr         = 0;                       // Wav file sample pointer
	int buflen            = std::get<int>(paramInstance["buflen"]);
	int steps             = std::get<int>(paramInstance["steps"]);
	int hopA              = std::get<int>(paramInstance["hopA"]);
	my_float magTol       = std::get<my_float>(paramInstance["magTol"]);
	std::string inputFile = std::get<std::string>(paramInstance.at("inputFile"));
	int sampleRate        = 44100;

	my_float shift    = POW(2, (steps/12));
	int hopS          = static_cast<int>(ROUND(hopA * shift));
	int numFrames     = static_cast<int>(buflen / hopA);
	int sigNumSamp    = sampleRate * 3;
	int bitsPerSample = 16;

	std::vector<float> in_audio(sigNumSamp);

	if (inputFile.rfind("signal_", 0) != std::string::npos)
	{
		int frequency = 440;
		int amplitude = 1;
		std::unique_ptr<WaveformGenerator> waveformGenerator;

		if (inputFile.find("sine") != std::string::npos)
		{
			waveformGenerator = std::make_unique<WafeformGenerator::SineWaveF>(amplitude, frequency, 0);
		}
		waveformGenerator->setStepSize(1.0 / sampleRate);
		for (int i = 0; i < in_audio.size(); i++)
		{
			in_audio[i] = waveformGenerator->generate();
		}
		writeWav(in_audio, inputFilePath, sampleRate, bitsPerSample);
	}
	else
	{
		in_audio = readWav(inputFilePath);
	}

	std::vector<float> out_audio(in_audio.size(), 0.0);

	INITIALIZE_DUMPERS(audio_ptr, buflen, numFrames, hopS, variationName, std::get<std::string>(paramInstance["inputFile"]));

	PROGRESS_BAR_CREATE(variationName, static_cast<int>(in_audio.size() / buflen));
	
	std::unique_ptr<PitchEngine> pe;
	std::string& algo = std::get<std::string>(paramInstance["algo"]);

	if (algo == "pv")
		pe = std::make_unique<PVEngine>(steps, buflen, hopA);
	else if (algo == "pvdr")
		pe = std::make_unique<PVDREngine>(steps, buflen, hopA, magTol);
	else if (algo == "cqpv")
		pe = std::make_unique<CQPVEngine>(steps, buflen, hopA, sampleRate, magTol);
	else if (algo == "se")
		pe = std::make_unique<StrechEngine>(steps, buflen, hopA);

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

	writeWav(out_audio, outputFilePath, sampleRate, bitsPerSample);

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
	//INIT_DUMPER("outframe.csv"  , audio_ptr, buflen, buflen, 40, -1);
	//INIT_DUMPER("inbuffer.csv"  , audio_ptr, buflen, buflen, 40, -1);
	//INIT_DUMPER("outbuffer.csv" , audio_ptr, buflen, buflen, 10, -1);
	//INIT_DUMPER("inwin.csv"     , audio_ptr, buflen, buflen, 40, -1);
	//INIT_DUMPER("outwin.csv"    , audio_ptr, buflen, buflen, 40, -1);
	//INIT_DUMPER("delta_f.csv"   , audio_ptr, buflen, buflen, -1, -1);
	//INIT_DUMPER("delta_t.csv"   , audio_ptr, buflen, buflen, -1, -1);
	//INIT_DUMPER("vTime.csv"     , audio_ptr, numFrames*hopS*2, numFrames*hopS*2, 40, -1);
}

std::vector<std::string> getFailedTests(parameterInstanceSet_t& paramInstanceSet, std::string testFileDir, std::string outputFileDir)
{

	std::vector<std::string> failedTests;

	for (auto& paramInstance : paramInstanceSet)
	{

		std::string testFilePath{ testFileDir };
		std::string outputFilePath{ outputFileDir };

		std::string inputFileName =  std::get<std::string>(paramInstance.at("inputFile"));

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



#include "main.h"
#include <time.h>
#include "wavio.h"
#include "audioUtils.h"
#include "stdint.h"
#include "CQPVEngine.h"
#include "PVDREngine.h"
#include "PVEngine.h"
#include "PitchEngine.h"
#include "logger.h"
#include "DumperContainer.h"
#include <cmath>
#include <chrono>

// #define DEFAULT_INPUT_FILENAME "constant_guitar_short.wav"
#define DEFAULT_INPUT_FILENAME "sine_short.wav"
#define DEFAULT_OUTPUT_FILENAME "output4.wav"

#define INGAIN     1
#define OUTGAIN    1
#define STEPS      12

DumperContainer* DumperContainer::instance = 0;

int main(int argc, char **argv)
{

	my_float avg_time     = 0;                      // Average time taken to compute a frame
	my_float elapsed_time = 0;
	uint32_t N         = 0;

	my_float var          = 0;

	// Local variable declaration
	std::string inputFilePath{ INPUT_AUDIO_DIR DEFAULT_INPUT_FILENAME };
	std::string outputFilePath{ OUTPUT_AUDIO_DIR DEFAULT_OUTPUT_FILENAME };
	my_float *coeffs         = 0;                 // Coefficients from the distortion polynomial
	uint32_t audio_ptr       = 0;                 // Wav file sample pointer
    uint32_t numSamp;                             // Total number of samples in wave file
	uint32_t sampleRate      = 44100;
	uint8_t frameNum         = 0;                 // Frame index. It's circular.
	uint32_t vTimeIdx        = 0;                 // Circular buffer for vTime
	my_float pOutBuffLastSample = 0;

	buffer_data_t bf;
	audio_data_t audat;

	parse_arguments(argc, argv, inputFilePath, outputFilePath, &var);

	PRINT_LOG("Input file: %s\n", inputFilePath);
	PRINT_LOG("Output file: %s\n", outputFilePath);

	// Load contents of wave file
	my_float* in_audio = readWav(numSamp, inputFilePath);                            // Get input audio from wav file and fetch the size

	parameterMap_t parameterMap;
	parameterCombinations_t paramCombs;

	parameterMap["buflen"] = { 1024, 2048, 4096 };
	parameterMap["hopA"]   = { 256, 512 };
	parameterMap["hopS"]   = { 123 };
	parameterMap["dummy"]  = { };

	generateParameterCombinations(parameterMap, paramCombs);

	for (auto& [paramName, paramValues] : parameterMap)
	{
		for (auto paramValue : paramValues)
		{
			//std::string variationName = "_" + paramName + "_" +
			//	std::to_string(std::get<uint32_t>(paramValue)) + "_";

			init_variables(&bf, &audat, numSamp, in_audio, sampleRate, STEPS, BUFLEN);

			//INITIALIZE_DUMPERS(audio_ptr, &bf, &audat, variationName);
			
			PRINT_LOG("Buffer length: %i.\n", bf.buflen);

			while(audio_ptr < (numSamp - bf.buflen))
			{
				for (uint16_t k = 0; k < bf.buflen; k++)
				{
					audat.inbuffer[k] = audat.in_audio[audio_ptr + k] * INGAIN;
				}

				elapsed_time = clock();

				printf("\r%i%%", 100 * audio_ptr/numSamp);

				auto initTime  = std::chrono::high_resolution_clock::now();

				process_buffer(&bf, &audat, frameNum, &vTimeIdx, &pOutBuffLastSample);

				auto finalTime = std::chrono::high_resolution_clock::now();
				auto exTime  = std::chrono::duration_cast<std::chrono::milliseconds>(finalTime - initTime);
				PRINT_LOG("Process buffer execution time: %s ms.\n", exTime.count());

				elapsed_time = clock() - elapsed_time;
				avg_time = avg_time + (elapsed_time - avg_time)/++N;

				for (uint16_t k = 0; k < bf.buflen; k++)
				{

					audat.out_audio[audio_ptr + k] = audat.outbuffer[k] * OUTGAIN;

					// Avoid uint16_t overflow and clip the signal instead.
					if (std::abs(audat.out_audio[audio_ptr + k]) > 1)
					{
						audat.out_audio[audio_ptr + k] = (audat.out_audio[audio_ptr + k] < 0) ? -1.0 : 1.0;
					}
				}
				audio_ptr += bf.buflen;
			}
			PRINT_LOG("It took an average of %f ms to process each frame.\n", 1000.0 * avg_time/CLOCKS_PER_SEC);

			// Reconvert floating point audio to 16bit
			//for (uint32_t i = 0; i < numSamp; i++)
			//{
			//	audat.out_audio[i] = audat.out_audio[i] * MAXVAL16;
			//}

			// Save the processed audio to the output file
			writeWav(audat.out_audio, inputFilePath, outputFilePath, numSamp);

			// Deallocate memory
			free_audio_data(&audat);
			free_buffer_data(&bf);
			free(coeffs);
		}
	}

	return 0;

}

void load_distortion_coefficients(my_float* coeffs, size_t* coeff_size)
{
	FILE* inFile = fopen("polyCoeff.csv", "r");
	my_float coeff;
	*coeff_size = 0;
	size_t n = 0;

	if(inFile == NULL)
	{
        PRINT_LOG("Error opening file\n");
        exit(1);
	}
#ifdef USE_DOUBLE
	while( fscanf(inFile, "%lf\n", &coeff) != EOF )
#else
	while( fscanf(inFile, "%f\n", &coeff) != EOF )
#endif
	coeffs = (my_float *) calloc(*coeff_size, sizeof(my_float));
	PRINT_LOG("Coefficients:\n");

#ifdef USE_DOUBLE
	while( fscanf(inFile, "%lf\n", &coeff) != EOF )
#else
	while( fscanf(inFile, "%f\n", &coeff) != EOF )
#endif
	{
		coeffs[n] = coeff;
		PRINT_LOG("%f\n", coeffs[n]);
		n++;
	}
	fclose(inFile);
}

void parse_arguments(int argc, char** argv, std::string&  inputFilePath, std::string& outputFilePath, my_float* var)
{
	if (argc > 1)
	{
		inputFilePath = argv[1];
	}
	if (argc > 2)
	{
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

void initializeDumpers(uint32_t& audio_ptr, buffer_data* bf, audio_data* audat, std::string& variationName)
{
	CREATE_DUMPER_C0NTAINER(DEBUG_DIR + variationName);
	INIT_DUMPER("mag.csv"      , audio_ptr, bf->buflen, bf->buflen, -1, -1);
	INIT_DUMPER("phi_a.csv"    , audio_ptr, bf->buflen, bf->buflen, -1, -1);
	INIT_DUMPER("phi_s.csv"    , audio_ptr, bf->buflen, bf->buflen, -1, -1);
	INIT_DUMPER("phi_sPrev.csv", audio_ptr, bf->buflen, bf->buflen, -1, -1);
	INIT_DUMPER("cpxIn.csv"    , audio_ptr, bf->buflen, bf->buflen,  5, -1);
	INIT_DUMPER("cpxOut.csv"   , audio_ptr, bf->buflen, bf->buflen,  5, -1);
	INIT_DUMPER("inbuffer.csv" , audio_ptr, bf->buflen, bf->buflen, 40, -1);
	INIT_DUMPER("outbuffer.csv", audio_ptr, bf->buflen, bf->buflen, 10, -1);
	INIT_DUMPER("inwin.csv"    , audio_ptr, bf->buflen, bf->buflen, 40, -1);
	INIT_DUMPER("outwin.csv"   , audio_ptr, bf->buflen, bf->buflen, 40, -1);
	INIT_DUMPER("delta_f.csv"  , audio_ptr, bf->buflen, bf->buflen, -1, -1);
	INIT_DUMPER("delta_t.csv"  , audio_ptr, bf->buflen, bf->buflen, -1, -1);
	INIT_DUMPER("vTime.csv"    , audio_ptr, audat->numFrames*bf->hopS*2, audat->numFrames*bf->hopS*2, 40, -1);
}

void generateParameterCombinations(const parameterMap_t & allParams, parameterCombinations_t & paramCombs)
{
	int numCombinations = 1;
	for (auto& it : allParams)
	{
		if (it.second.empty()) { continue; }
		numCombinations *= it.second.size();
	}
	for (auto& it : allParams)
	{
		if (it.second.empty()) { continue; }
		for (int i = 0; i < numCombinations / it.second.size(); i++)
		{
			paramCombs[it.first].insert(paramCombs[it.first].end(), it.second.begin(), it.second.end());
		}
	}

}


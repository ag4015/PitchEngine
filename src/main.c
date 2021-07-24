
#include "main.h"
#include <time.h>
#include "wavio.h"
#include "audioUtils.h"
#include "DSPConfig.h"
#include "stdint.h"

#define DEFAULT_INPUT_FILENAME "constant_guitar_short.wav"
//#define DEFAULT_INPUT_FILENAME "sine_short.wav"
#define DEFAULT_OUTPUT_FILENAME "output4.wav"

#define INGAIN     1
#define OUTGAIN    1
#define STEPS      12

int main(int argc, char **argv)
{

#ifdef DSPDEBUG
	my_float avg_time     = 0;                      // Average time taken to compute a frame
	my_float elapsed_time = 0;
	uint32_t N         = 0;
#endif
	my_float var          = 0;

	// Local variable declaration
	char* inputFilePath      = 0; 
	char* outputFilePath     = 0; 
	my_float *coeffs         = 0;                 // Coefficients from the distortion polynomial
	uint32_t audio_ptr       = 0;                 // Wav file sample pointer
	uint8_t frameNum         = 0;                 // Frame index. It's circular.
	uint32_t vTimeIdx        = 0;                 // Circular buffer for vTime
	uint32_t cleanIdx        = 0;                 // Circular buffer for reseting the vTime array
	my_float pOutBuffLastSample = 0;
    uint32_t numSamp;                             // Total number of samples in wave file

	buffer_data_t bf;
	audio_data_t audat;

	parse_arguments(argc, argv, &inputFilePath, &outputFilePath, &var);

	PRINT_LOG("Input file: %s\n", inputFilePath);
	PRINT_LOG("Output file: %s\n", outputFilePath);

	// Load contents of wave file
	my_float* in_audio = readWav(&numSamp, inputFilePath);                            // Get input audio from wav file and fetch the size

	init_variables(&bf, &audat, numSamp, in_audio, STEPS, BUFLEN);
	
	PRINT_LOG("Buffer length: %i.\n", bf.buflen);

	while(audio_ptr < (numSamp - bf.buflen))
	{
		for (int16_t k = 0; k < bf.buflen; k++)
		{
			audat.inbuffer[k] = audat.in_audio[audio_ptr + k] * INGAIN;
		}

#ifdef DSPDEBUG
		elapsed_time = clock();
#endif
		//printf("\r%i%%", 100 * audio_ptr/numSamp);

		auto initTime    = std::chrono::high_resolution_clock::now();
		//process_buffer(&bf, &audat, frameNum, audio_ptr, &vTimeIdx, &cleanIdx, &pOutBuffLastSample, var);
		auto finalTime   = std::chrono::high_resolution_clock::now();

#ifdef DSPDEBUG
		elapsed_time = clock() - elapsed_time;
		avg_time = avg_time + (elapsed_time - avg_time)/++N;
#endif

		for (int16_t k = 0; k < bf.buflen; k++)
		{

			audat.out_audio[audio_ptr + k] = audat.outbuffer[k] * OUTGAIN;

			// Avoid uint16_t overflow and clip the signal instead.
			if (abs(audat.out_audio[audio_ptr + k]) > 1)
			{
				audat.out_audio[audio_ptr + k] = (audat.out_audio[audio_ptr + k] < 0) ? -1 : 1;
			}
		}
		audio_ptr += bf.buflen;
	}
#ifdef DSPDEBUG
	PRINT_LOG("It took an average of %f ms to process each frame.\n", 1000.0 * avg_time/CLOCKS_PER_SEC);
#endif

	// Reconvert floating point audio to 16bit
	//for (uint32_t i = 0; i < numSamp; i++)
	//{
	//	audat.out_audio[i] = audat.out_audio[i] * MAXVAL16;
	//}

	// Save the processed audio to the output file
	writeWav(audat.out_audio, inputFilePath, outputFilePath);

	// Deallocate memory
	free_audio_data(&audat);
	free_buffer_data(&bf);
	free(coeffs);
	free(inputFilePath);
	free(outputFilePath);

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
	while( fscanf(inFile, "%f\n", &coeff) != EOF )
	coeffs = (my_float *) calloc(*coeff_size, sizeof(my_float));
	PRINT_LOG("Coefficients:\n");

	while( fscanf(inFile, "%f\n", &coeff) != EOF )
	{
		coeffs[n] = coeff;
		PRINT_LOG("%f\n", coeffs[n]);
		n++;
	}
	fclose(inFile);
}

void parse_arguments(int argc, char** argv, char** inputFilePath, char** outputFilePath, my_float* var)
{
	if (argc > 1)
	{
		*inputFilePath = (char*) malloc( sizeof( INPUT_AUDIO_DIR ) + (int)strlen(argv[1]) + 1);
		strcpy(*inputFilePath, INPUT_AUDIO_DIR);
		strcat(*inputFilePath, argv[1]);
	}
	else
	{
		*inputFilePath = (char*) malloc( sizeof( INPUT_AUDIO_DIR DEFAULT_INPUT_FILENAME) );
		strcpy(*inputFilePath, INPUT_AUDIO_DIR DEFAULT_INPUT_FILENAME);
	}
	if (argc > 2)
	{
		*outputFilePath = (char*) malloc( sizeof( OUTPUT_AUDIO_DIR ) + (int)strlen(argv[2]) + 1);
		strcpy(*outputFilePath, OUTPUT_AUDIO_DIR);
		strcat(*outputFilePath, argv[2]);
	}
	else
	{
		*outputFilePath = (char*) malloc( sizeof( OUTPUT_AUDIO_DIR DEFAULT_OUTPUT_FILENAME ));
		strcpy(*outputFilePath, OUTPUT_AUDIO_DIR DEFAULT_OUTPUT_FILENAME );
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


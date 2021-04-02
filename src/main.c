
#include "main.h"
#include <time.h>
#include "wavio.h"
#include "audioUtils.h"
#include "DSPConfig.h"
#include "stdint.h"

//#define DEFAULT_INPUT_FILENAME "constant_guitar_short.wav"
#define DEFAULT_INPUT_FILENAME "constant_guitar.wav"
#define DEFAULT_OUTPUT_FILENAME "output.wav"

#define INGAIN     1
#define OUTGAIN    1

int main(int argc, char **argv)
{

#ifdef DSPDEBUG
	float avg_time     = 0;                      // Average time taken to compute a frame
	float elapsed_time = 0;
	float var          = 0;
	uint32_t N         = 0;
#endif

	// Local variable declaration
	char* inputFilePath      = 0; 
	char* outputFilePath     = 0; 
	float *coeffs            = 0;                 // Coefficients from the distortion polynomial
	uint32_t audio_ptr       = 0;                 // Wav file sample pointer
	uint8_t frameNum         = 0;                 // Frame index. It's circular.
	uint32_t vTimeIdx        = 0;                 // Circular buffer for vTime
	uint32_t cleanIdx        = 0;                 // Circular buffer for reseting the vTime array
	float pOutBuffLastSample = 0;
    uint32_t numSamp;                             // Total number of samples in wave file

	buffer_data_t bf;
	audio_data_t audat;

	parse_arguments(argc, argv, &inputFilePath, &outputFilePath, var);

	PRINT_LOG2("Input file: %s\n", inputFilePath);
	PRINT_LOG2("Output file: %s\n", outputFilePath);

	// Load contents of wave file
	float* in_audio = readWav(&numSamp, inputFilePath);                            // Get input audio from wav file and fetch the size

	init_variables(&bf, &audat, numSamp, in_audio, 0);
	
	PRINT_LOG2("Buffer length: %i.\n", bf.buflen);

	// The first buffer isn't processed but it is stored
	for (uint8_t f = 0; f < audat.numFrames; f++)
	{
		for (uint32_t k = 0; k < bf.hopA; k++)
		{
			audat.inframe[f * bf.hopA + k] = audat.in_audio[audio_ptr + k + f*bf.hopA] * INGAIN;
		}
	}
	audio_ptr += bf.buflen;
	while(audio_ptr < (numSamp - bf.buflen))
	{
		for (int16_t k = 0; k < bf.buflen; k++)
		{
			audat.inbuffer[k] = audat.in_audio[audio_ptr + k] * INGAIN;
		}
#ifdef DSPDEBUG
		elapsed_time = clock();
#endif

		process_buffer(&bf, &audat, frameNum, audio_ptr, &vTimeIdx, &cleanIdx, pOutBuffLastSample, var);

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

	PRINT_LOG2("It took an average of %f ms to process each frame.\n", 1000.0 * avg_time/CLOCKS_PER_SEC);

	// Reconvert floating point audio to 16bit
	for (uint32_t i = 0; i < numSamp; i++)
	{
		audat.out_audio[i] = audat.out_audio[i] * MAXVAL16;
	}

	// Save the processed audio to the output file
	writeWav(audat.out_audio, inputFilePath, outputFilePath, numSamp);

	// Deallocate memory
	free_audio_data(&audat);
	free_buffer_data(&bf);
	free(coeffs);
	free(inputFilePath);
	free(outputFilePath);

	return 0;

}

void load_distortion_coefficients(float* coeffs, size_t* coeff_size)
{
	FILE* inFile = fopen("polyCoeff.csv", "r");
	float coeff;
	*coeff_size = 0;
	size_t n = 0;

	if(inFile == NULL)
	{
        PRINT_LOG1("Error opening file\n");
        exit(1);
	}
	while( fscanf(inFile, "%f\n", &coeff) != EOF )
	coeffs = (float *) calloc(*coeff_size, sizeof(float));
	PRINT_LOG1("Coefficients:\n");

	while( fscanf(inFile, "%f\n", &coeff) != EOF )
	{
		coeffs[n] = coeff;
		PRINT_LOG2("%f\n", coeffs[n]);
		n++;
	}
	fclose(inFile);
}

void parse_arguments(int argc, char** argv, char** inputFilePath, char** outputFilePath, float var)
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
		char* tmpPtr;
		var = strtof(argv[3], &tmpPtr);
	}
}



#include "main.h"
#include <time.h>
#include "wavio.h"
#include "audioUtils.h"

// Global variable declaration
float avg_time     = 0;                          // Average time taken to compute a frame
float elapsed_time = 0;                          // Time spent computing the current frame
float N            = 1;                          // Number of samples processed
int32_t count      = 0;
int32_t count2     = 0;
float var          = 0;

int main(int argc, char **argv)
{
	// Local variable declaration
	char* inputFilePath      = 0; 
	char* outputFilePath     = 0; 
	float *coeffs            = 0;                 // Coefficients from the distortion polynomial
	uint32_t audio_ptr       = 0;                 // Wav file sample pointer
	uint8_t frameNum         = 0;                 // Frame index. It's circular.
	uint32_t vTimeIdx        = 0;                 // Circular buffer for vTime
	float pOutBuffLastSample = 0;
	uint32_t cleanIdx        = 0;

	buffer_data_t bf;
	audio_data_t audat;

	parse_arguments(argc, argv, &inputFilePath, &outputFilePath, var);

	// Load contents of wave file
	printf("Input file: %s\n\n", inputFilePath);
	uint32_t numSamp;
	float* in_audio = readWav(&numSamp, inputFilePath);                            // Get input audio from wav file and fetch the size

	// Pitch variables
	uint8_t steps = 12;
	float shift = powf(2, steps/12);
	uint32_t hopS = (int32_t)roundf((float)(HOPA * shift));

	// Initialize structures
	initialize_audio_data(&audat, hopS, NUMFRAMES, numSamp, BUFLEN, in_audio);
	initialize_buffer_data(&bf, &audat, shift, hopS, steps, HOPA, BUFLEN);

	cleanIdx  = bf.hopS * NUMFRAMES;

	// Initialize input and output window functions
	for(uint16_t k = 0; k < BUFLEN; k++){
		audat.inwin[k]   =  HAMCONST * (1 - cos(2 * PI * k/BUFLEN))/sqrtf((BUFLEN/bf.hopA)/2);
		audat.outwin[k]  =  HAMCONST * (1 - cos(2 * PI * k/BUFLEN))/sqrtf((BUFLEN/bf.hopS)/2);
	}

	for (uint32_t i = 0; i < audat.numSamp; i++){
		audat.in_audio[i] = audat.in_audio[i] / MAXVAL16;
	}
	
	PRINT_LOG2("Buffer length: %i.\n", BUFLEN);

	// The first buffer isn't processed but it is stored
	for (uint8_t f = 0; f < NUMFRAMES; f++)
	{
		for (uint32_t k = 0; k < bf.hopA; k++)
		{
			audat.inframe[f * bf.hopA + k] = audat.in_audio[audio_ptr + k + f*bf.hopA] * INGAIN;
		}
	}
	audio_ptr += BUFLEN;
	while(audio_ptr < (audat.numSamp - BUFLEN))
	{
		for (int16_t k = 0; k < BUFLEN; k++)
		{
			audat.inbuffer[k] = audat.in_audio[audio_ptr + k] * INGAIN;
			audat.outbuffer[k] = audat.inbuffer[k];
		}
		process_buffer(&audat, &bf, frameNum, audio_ptr, vTimeIdx, &cleanIdx, pOutBuffLastSample);
		for (int16_t k = 0; k < BUFLEN; k++)
		{
			audat.out_audio[audio_ptr + k] = audat.outbuffer[k] * OUTGAIN;
			// Avoid uint16_t overflow and clip the signal instead.
			if (abs(audat.out_audio[audio_ptr + k]) > 1)
			{
				audat.out_audio[audio_ptr + k] = (audat.out_audio[audio_ptr + k] < 0) ? -1 : 1;
			}
		}
		audio_ptr += BUFLEN;
	}

	printf("It took an average time of %f ms to process each frame.\n", 1000*avg_time/CLOCKS_PER_SEC);

	// Reconvert floating point audio to 16bit
	for (uint32_t i = 0; i < audat.numSamp; i++)
	{
		audat.out_audio[i] = audat.out_audio[i] * MAXVAL16;
	}

	// Save the processed audio to the output file
	writeWav(audat.out_audio, inputFilePath, outputFilePath, audat.numSamp);

	// Deallocate memory
	free_audio_data(&audat);
	free_buffer_data(&bf);
	free(coeffs);
	free(inputFilePath);
	free(outputFilePath);

	return 0;

}

void process_buffer(audio_data_t* audat, buffer_data_t* bf, uint8_t frameNum,
	uint32_t audio_ptr, uint32_t vTimeIdx, uint32_t* cleanIdx, float pOutBuffLastSample)
{
	elapsed_time = clock();

	for (uint8_t f = 0; f < NUMFRAMES; f++)
	{
		
		//swap_ping_pong_buffer_data(&bf, &audat);
		// Update magnitude pointers
		bf->magPrev = bf->mag;
		bf->mag = (bf->mag == audat->mag_ping) ? audat->mag_pong : audat->mag_ping;

		// Update phase pointers
		bf->phi_sPrev = bf->phi_s;
		bf->phi_s = (bf->phi_s == audat->phi_ping) ? audat->phi_pong : audat->phi_ping;

		// Update time phase derivative pointers
		bf->delta_tPrev = bf->delta_t;
		bf->delta_t = (bf->delta_t == audat->delta_t_ping) ? audat->delta_t_pong : audat->delta_t_ping;

/************ ANALYSIS STAGE ***********************/

		// Using mag as output buffer, nothing to do with magnitude
		overlapAdd(audat->inbuffer, audat->inframe, bf->mag, bf->hopA, frameNum, NUMFRAMES);  
		COPY(bf->cpxIn[k].r, bf->mag[k] * audat->inwin[k], BUFLEN);
		COPY(bf->cpxIn[k].i, 0, BUFLEN);

/************ PROCESSING STAGE *********************/

		DUMP_ARRAY_COMPLEX(bf->cpxIn, BUFLEN, DEBUG_DIR "cpxInXXXXX.csv", count, 5, audio_ptr, -1);

		kiss_fft( bf->cfg , bf->cpxIn , bf->cpxOut );

		/* process_frame(bf->cpxOut, bf->mag, bf->magPrev, bf->phi_a, bf->phi_s, bf->phi_sPrev, bf->delta_t, bf->delta_tPrev, bf->delta_f, bf->hopA, bf->hopS, shift, BUFLEN, var); */

		// DUMP_ARRAY_COMPLEX(bf->cpxOut, BUFLEN, DEBUG_DIR "cpxOutXXXXX.csv", count, 40, audio_ptr,     -1);
		// DUMP_ARRAY(audat->inbuffer      , BUFLEN, DEBUG_DIR "inbuffer.csv"  , count, -1, audio_ptr, BUFLEN);
		// DUMP_ARRAY(audat->inwin         , BUFLEN, DEBUG_DIR "inwin.csv"     , count, -1, audio_ptr, BUFLEN);
		// DUMP_ARRAY(audat->outwin        , BUFLEN, DEBUG_DIR "outwin.csv"    , count, -1, audio_ptr, BUFLEN);
		// DUMP_ARRAY(bf->phi_a         , BUFLEN, DEBUG_DIR "phi_a.csv"     , count, -1, audio_ptr, BUFLEN);
		// DUMP_ARRAY(bf->phi_s         , BUFLEN, DEBUG_DIR "phi_s.csv"     , count, -1, audio_ptr, BUFLEN);

		kiss_fft( bf->cfgInv , bf->cpxOut , bf->cpxIn );

		DUMP_ARRAY_COMPLEX(bf->cpxIn, BUFLEN, DEBUG_DIR "cpxOutXXXXX.csv", count, 5, audio_ptr,     -1);

		COPY(bf->cpxOut[k].r, bf->cpxIn[k].r * audat->outwin[k]/BUFLEN, BUFLEN);


/************ SYNTHESIS STAGE ***********************/

		COPY(bf->mag[k], bf->cpxOut[k].r, BUFLEN);
		strechFrame(audat->vTime, bf->mag, cleanIdx, bf->hopS, frameNum, vTimeIdx, NUMFRAMES * bf->hopS * 2, BUFLEN);

		// DUMP_ARRAY(audat->vTime, NUMFRAMES*hopS*2, DEBUG_DIR "vTimeXXX.csv", count, 40, audio_ptr, -1);

		if ((++frameNum) >= NUMFRAMES) frameNum = 0;
		count++;

	}

/************* LINEAR INTERPOLATION *****************/

	interpolate(audat->outbuffer, audat->vTime, bf->steps, bf->shift, vTimeIdx, pOutBuffLastSample, bf->hopS, BUFLEN);

	// PRINT_LOG1("********* Buffer is out *********\n");

	DUMP_ARRAY(audat->outbuffer, BUFLEN, DEBUG_DIR "outXXX.csv", count2, 10, audio_ptr, -1);
	count2++;

	elapsed_time = clock() - elapsed_time;
	avg_time = avg_time + (elapsed_time - avg_time)/N;
	N++;
	vTimeIdx += NUMFRAMES * bf->hopS;
	if ((vTimeIdx) >= NUMFRAMES * bf->hopS * 2) vTimeIdx = 0;

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

void dumpFloatArray(float* buf, size_t size, const char* name, int count, int max, int auP, int auPMax)
{
		if ((count > max && max != -1) || (auP != auPMax && auPMax != -1)) { return; }

		int len = strlen(name) + 1; // Plus one for the \0
		char* fileName = (char*) malloc(sizeof(char)*len);
		strcpy(fileName, name);

		// Open file to append data. Clear if it's the first entry
		FILE *outfile;
		if (!count) { outfile = fopen(fileName, "w"); }
		else { outfile = fopen(fileName, "a"); }

		// Store in a format understandable by numpy in python
		for (size_t i = 0; i < size; i++){
			fprintf(outfile, "%f;", buf[i]);	
		}

		fprintf(outfile, "\n");
		free(fileName);
		fclose(outfile);
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
		*inputFilePath = (char*) malloc( sizeof( INPUT_AUDIO_DIR "sine_tester_short.wav") );
		strcpy(*inputFilePath, INPUT_AUDIO_DIR "sine_tester_short.wav");
	}
	if (argc > 2)
	{
		*outputFilePath = (char*) malloc( sizeof( OUTPUT_AUDIO_DIR ) + (int)strlen(argv[2]) + 1);
		strcpy(*outputFilePath, OUTPUT_AUDIO_DIR);
		strcat(*outputFilePath, argv[2]);
	}
	else
	{
		*outputFilePath = (char*) malloc( sizeof( OUTPUT_AUDIO_DIR "output.wav" ));
		strcpy(*outputFilePath, OUTPUT_AUDIO_DIR "output.wav" );
	}
	if (argc > 3)
	{
		char* tmpPtr;
		var = strtof(argv[3], &tmpPtr);
	}
}


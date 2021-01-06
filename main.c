
#include "main.h"
#include <time.h>
#include "audioUtils.h"
#include "wavio.h"

// Global variables declaration
float *inbuffer, *outbuffer;              // Input and output buffers
float *inframe, *outframe;                // Pointer to the current frame
float *inwin, *outwin;                    // Input and output windows
float *vTime;                             // Overlap-add signal
float *in_audio, *out_audio;              // Complete audio data from wav file for input and output
float *mag, *phase;                       // Frame magnitude and phase
float *phi_s;                             // Phase adjusted for synthesis stage
float *coeffs = 0;                        // Coefficients from the distortion polynomial
size_t coeff_size;                        // Number of coefficients pointed at by coeffs
int16_t *audio16;                         // 16 bit integer representation of the audio
unsigned long int audio_ptr = 0;          // Wav file sample pointer
unsigned long NUM_SAMP;                   // Total number of samples in wave file
float avg_time = 0;                       // Average time taken to compute a frame
float elapsed_time = 0;                   // Time spent computing the current frame
float N = 1;                              // Number of samples processed
uint8_t frameNum = 0;                     // Frame index. It's circular.
int vTimeIdx = 0;                         // Circular buffer for vTime
float steps;                              // Number of semitones by which to shift the signal
float shift;
int hopA;
int hopS;
float* vTimePtr;                          // Pointer to the ping-pong vTime array
int cleanIdx;
int count = 0;
int count2 = 0;
int sizeVTime;
float pOutBuffLastSample = 0; 

#ifdef PDEBUG
	float *previousPhase;
	float *deltaPhi;  
	float *deltaPhiPrime;
	float *deltaPhiPrimeMod;
	float *trueFreq;
	float *phaseCumulative;
#else
	float previousPhase;
	float deltaPhi;  
	float deltaPhiPrime;
	float deltaPhiPrimeMod;
	float trueFreq;
	float phaseCumulative = 0;
#endif

int main()
{
	// Variable declaration
	unsigned long sizeData;                                 //Size of audio data in bytes
	FILE* outfile;
	const char* filePath = "/mnt/c/Users/alexg/Google Drive/Projects/Denoiser/sine_tester_short.wav";

	// Pitch variables
	steps = 5;
	shift = pow(2, steps/12);

	hopA = HOPA;
	hopS = (int)round(hopA * shift);
	cleanIdx = hopS*NUMFRAMES;
  sizeVTime = NUMFRAMES * hopS * 2;

	PRINT_LOG2("Input file: %s\n\n", filePath);
	audio16 = readWav(&sizeData, filePath);                         // Get input audio from wav file and fetch the size
	NUM_SAMP = sizeData/sizeof(*audio16);                           // Number of samples

	// Allocate and zero fill arrays 
	inbuffer  = (float *) calloc(BUFLEN,          sizeof(float));   // Input array 
	outbuffer = (float *) calloc(BUFLEN,          sizeof(float));   // Output array 
	inframe   = (float *) calloc(BUFLEN,          sizeof(float));
	outframe  = (float *) calloc(sizeVTime,       sizeof(float));
	vTime     = (float *) calloc(sizeVTime,       sizeof(float));   // Overlap-add signal. Times two because of ping-pong.
	inwin     = (float *) calloc(BUFLEN,          sizeof(float));   // Input window
	outwin    = (float *) calloc(BUFLEN,          sizeof(float));   // Output window
	phase     = (float *) calloc(BUFLEN,          sizeof(float));   // Phase of current frame
	phi_s     = (float *) calloc(BUFLEN,          sizeof(float));   // Phase adjusted for synthesis stage
	mag       = (float *) calloc(BUFLEN,          sizeof(float));   // Magnitude of current frame
	cpxIn     = (kiss_fft_cpx*)calloc(BUFLEN,     sizeof(kiss_fft_cpx)); // Complex variable for FFT 
	cpxOut    = (kiss_fft_cpx*)calloc(BUFLEN,     sizeof(kiss_fft_cpx)); // Complex variable for FFT 
	in_audio  = (float *) calloc(NUM_SAMP,        sizeof(float));   // Total input audio
	out_audio = (float *) calloc(NUM_SAMP,        sizeof(float));   // Total output audio

	cfg    = kiss_fft_alloc( BUFLEN, 0, 0, 0);
	cfgInv = kiss_fft_alloc( BUFLEN, 1, 0, 0);

#ifdef PDEBUG
	previousPhase     = (float *) calloc(BUFLEN,  sizeof(float));   // Magnitude of current frame
	deltaPhi          = (float *) calloc(BUFLEN,  sizeof(float));   // Magnitude of current frame
	deltaPhiPrime     = (float *) calloc(BUFLEN,  sizeof(float));   // Magnitude of current frame
	deltaPhiPrimeMod  = (float *) calloc(BUFLEN,  sizeof(float));   // Magnitude of current frame
	trueFreq          = (float *) calloc(BUFLEN,  sizeof(float));   // Magnitude of current frame
	phaseCumulative   = (float *) calloc(BUFLEN,  sizeof(float));   // Magnitude of current frame
#endif

	
	// Initialize input and output window functions
	for(int k = 0; k < BUFLEN; k++){
		inwin[k]   =  HAMCONST * (1 - cos(2 * PI * k/BUFLEN))/sqrt((BUFLEN/hopA)/2);
		outwin[k]  =  HAMCONST * (1 - cos(2 * PI * k/BUFLEN))/sqrt((BUFLEN/hopS)/2);
		/* inwin[k] = 1; */
		/* outwin[k] = 1; */
		/* inwin[k]   =  HAMCONST * (1 - cos(2 * PI * k/BUFLEN)); */
		/* outwin[k]  =  HAMCONST * (1 - cos(2 * PI * k/BUFLEN)); */
	}
	
	// Clear freq.csv file of any content
	outfile = fopen("freq.csv", "w");
	fclose(outfile);

	// Convert 16bit audio to floating point
	for (unsigned long i = 0; i < NUM_SAMP; i++){
		in_audio[i] = ((float)audio16[i])/MAXVAL16;
	}

	printf("Buffer length: %i.\n", BUFLEN);

	// The first buffer isn't processed but it is stored
	for (uint8_t f = 0; f < NUMFRAMES; f++)
	{
		for (int k = 0; k < hopA; k++)
		{
			inframe[f * hopA + k] = in_audio[audio_ptr + k + f*hopA] * INGAIN;
		}
	}
	audio_ptr += BUFLEN;
	while(audio_ptr < (NUM_SAMP - BUFLEN))
	{
		for (int k = 0; k < BUFLEN; k++)
		{
			inbuffer[k] = in_audio[audio_ptr + k] * INGAIN;
		}
		process_buffer();
		for (int k = 0; k < BUFLEN; k++)
		{
			out_audio[audio_ptr + k] = outbuffer[k] * OUTGAIN;
			// Avoid uint16_t overflow and clip the signal instead.
			out_audio[audio_ptr + k] = (out_audio[audio_ptr + k] > 1) ? 1 : out_audio[audio_ptr + k];
		}
		audio_ptr += BUFLEN;
	}

	printf("It took an average time of %f ms to process each frame.\n", 1000*avg_time/CLOCKS_PER_SEC);

	// Reconvert floating point audio to 16bit
	for (unsigned long i = 0; i < NUM_SAMP; i++)
	{
		audio16[i] = (uint16_t)(out_audio[i]*MAXVAL16);
	}

	// Save the processed audio to output.wav
	writeWav(audio16, filePath);

	// Deallocate allocated memory
	free(audio16);
	free(inbuffer);
	free(outbuffer);
	free(inframe);
	free(outframe);
	free(vTime);
	free(inwin);
	free(outwin);
	free(phase);
	free(phi_s);
	free(mag);
	free(cpxIn);
	free(cpxOut);
	free(in_audio);
	free(out_audio);
	free(coeffs);
	kiss_fft_free(cfg);
	kiss_fft_free(cfgInv);

#ifdef PDEBUG
	free(previousPhase);
	free(deltaPhi);  
	free(deltaPhiPrime);
	free(deltaPhiPrimeMod);
	free(trueFreq);
	free(phaseCumulative);
#endif

	return 0;

}

void process_buffer()
{
	/* Variable declaration */
	elapsed_time = clock();

	for (uint8_t f = 0; f < NUMFRAMES; f++)
	{

/************ ANALYSIS STAGE ***********************/

		overlapAdd(inbuffer, inframe, mag, hopA, frameNum, NUMFRAMES);  
		COPY(cpxIn[k].r, mag[k] * inwin[k], BUFLEN);

/************ PROCESSING STAGE ***********************/

		DUMP_ARRAY_COMPLEX(cpxIn, BUFLEN, "debugData/cpxInXXX.csv", count, 40, audio_ptr, -1);

		kiss_fft( cfg , cpxIn , cpxOut );

#ifndef PDEBUG
    process_frame(cpxOut, mag, phase, phaseCumulative, hopA, hopS, BUFLEN);
#else
    process_frame( mag, cpxOut, phase, deltaPhi, previousPhase, deltaPhiPrime,
		               deltaPhiPrimeMod, trueFreq, phaseCumulative, hopA, hopS, BUFLEN);
#endif

		DUMP_ARRAY_COMPLEX(cpxOut  , BUFLEN, "debugData/cpxOutXXX.csv", count, 40, audio_ptr, -1);
		DUMP_ARRAY(inbuffer        , BUFLEN, "debugData/inbuffer.csv", count, -1, audio_ptr, BUFLEN);
		DUMP_ARRAY(inwin           , BUFLEN, "debugData/inwin.csv", count, -1, audio_ptr, BUFLEN);
		DUMP_ARRAY(outwin          , BUFLEN, "debugData/outwin.csv", count, -1, audio_ptr, BUFLEN);
		DUMP_ARRAY(previousPhase   , BUFLEN, "debugData/previousPhase.csv", count, -1, audio_ptr, BUFLEN);
		DUMP_ARRAY(phase           , BUFLEN, "debugData/phase.csv", count, -1, audio_ptr, BUFLEN);
		DUMP_ARRAY(deltaPhi        , BUFLEN, "debugData/deltaPhi.csv", count, -1, audio_ptr, BUFLEN);
		DUMP_ARRAY(deltaPhiPrime   , BUFLEN, "debugData/deltaPhiPrime.csv", count, -1, audio_ptr, BUFLEN);
		DUMP_ARRAY(deltaPhiPrimeMod, BUFLEN, "debugData/deltaPhiPrimeMod.csv", count, -1, audio_ptr, BUFLEN);
		DUMP_ARRAY(trueFreq        , BUFLEN, "debugData/trueFreq.csv", count, -1, audio_ptr, BUFLEN);
		DUMP_ARRAY(phaseCumulative , BUFLEN, "debugData/phaseCumulative.csv", count, -1, audio_ptr, BUFLEN);

		kiss_fft( cfgInv , cpxIn , cpxOut );

		COPY(cpxOut[k].r, cpxIn[k].r * outwin[k], BUFLEN);


/************ SYNTHESIS STAGE ***********************/

		COPY(mag[k], cpxOut[k].r, BUFLEN);
		PRINT_LOG1("*******************\n");
		PRINT_LOG2("vTimeIdx: %i\n", vTimeIdx);
		PRINT_LOG2("cleanIdx from %i", cleanIdx);
    strechFrame(vTime, mag, &cleanIdx, hopS, frameNum, vTimeIdx, sizeVTime, BUFLEN);
		PRINT_LOG2(" to %i\n", cleanIdx-1);
		/* PRINT_LOG3("t from %i to %i.\n", initT, t); */

		DUMP_ARRAY(vTime, NUMFRAMES*hopS*2, "debugData/vTimeXXX.csv", count, 40, audio_ptr, -1);

		if ((++frameNum) >= NUMFRAMES) frameNum = 0;
		count++;

	}
/************ LINEAR INTERPOLATION ***********************/

	interpolate(outbuffer, vTime, steps, shift, vTimeIdx, pOutBuffLastSample, hopS, BUFLEN);

	PRINT_LOG1("*******************\n");
	PRINT_LOG1("Buffer is out\n");
	PRINT_LOG1("*********************************\n");

	DUMP_ARRAY(outbuffer, BUFLEN, "debugData/outXXX.csv", count2, 10, audio_ptr, -1);
	count2++;

	elapsed_time = clock() - elapsed_time;
	avg_time = avg_time + (elapsed_time - avg_time)/N;
	N++;
	vTimeIdx += NUMFRAMES * hopS;
	if ((vTimeIdx) >= NUMFRAMES*hopS*2) vTimeIdx = 0;

}

float* load_distortion_coefficients(size_t* coeff_size)
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

	return coeffs;

}

void dumpFloatArray(float* buf, size_t size, const char* name, int count, int max, int auP, int auPMax)
{
		if ((count > max && max != -1) || (auP != auPMax && auPMax != -1))
		{
			return;
		}
		int len = strlen(name);
		char* fileName = (char*) malloc(sizeof(char)*len);
		strcpy(fileName, name);

		if (max != -1)
		{
			fileName[len - 7] = (int)(count/100) + 48;
			fileName[len - 6] = (int)(count/10) + 48;
			fileName[len - 5] = (int)(count % 10) + 48;
		}

		// Open file to append fft data
		FILE *outfile;
		outfile = fopen(fileName, "w");

		// Store in a format understandable by numpy in python
		for (int i = 0; i < size; i++){
			fprintf(outfile, "%f;", buf[i]);	
		}

		fprintf(outfile, "\n");
		free(fileName);
		fclose(outfile);
}


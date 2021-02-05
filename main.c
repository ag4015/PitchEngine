
#include "main.h"
#include <time.h>
#include "wavio.h"
#include "audioUtils.h"

// Global variables declaration
float *inbuffer, *outbuffer;              // Input and output buffers
float *inframe;                           // Pointer to the current frame
float *inwin, *outwin;                    // Input and output windows
float *vTime;                             // Overlap-add signal
float *in_audio, *out_audio;              // Complete audio data from wav file for input and output
float *mag, *magPrev;                     // Pointer to the magnitude of the current and previous frame
float *mag_ping, *mag_pong;               // Frame magnitude ping-pong buffers
float *phi_a;                             // Frame analysis phase
float *phi_s, *phi_sPrev;                 // Phase adjusted for synthesis stage of the current and previous frame
float *phi_ping, *phi_pong;               // Frame phase ping-pong buffers
float* delta_t, *delta_tPrev, *delta_f;   // Frame phase time and frequency derivatives
float *delta_t_ping, *delta_t_pong;       // Frame phase time derivative ping-pong buffers
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
float* vTimePtr;                          // Pointer to the ping-pong vTime array
float shift;
int hopA;
int hopS;
int cleanIdx;
int count = 0;
int count2 = 0;
int sizeVTime;
float pOutBuffLastSample = 0; 
kiss_fft_cpx *cpxIn, *cpxOut;             // Complex variable for FFT 
kiss_fft_cfg cfg;
kiss_fft_cfg cfgInv;
float var;

int main(int argc, char **argv)
{
	// Variable declaration
	unsigned long sizeData;                                 //Size of audio data in bytes
	char* inputFilePath; 
	char* outputFilePath; 
	var = 0;
	if (argc > 1)
	{
		inputFilePath = (char*) malloc( sizeof( "/mnt/c/Users/alexg/Google Drive/Projects/Denoiser/" ) + (int)strlen(argv[1]) + 1);
		strcpy(inputFilePath, "/mnt/c/Users/alexg/Google Drive/Projects/Denoiser/");
		strcat(inputFilePath, argv[1]);
	}
	else
	{
		inputFilePath = (char*) malloc( sizeof( "/mnt/c/Users/alexg/Google Drive/Projects/Denoiser/constant_guitar.wav" ) );
		strcpy(inputFilePath, "/mnt/c/Users/alexg/Google Drive/Projects/Denoiser/constant_guitar.wav");
	}
	if (argc > 2)
	{
		outputFilePath = (char*) malloc( sizeof( "/mnt/c/Users/alexg/Google Drive/Projects/Guitar Pedal/Software/Pedal/DSPSimulator/output/") + (int)strlen(argv[2]) + 1);
		strcpy(outputFilePath, "/mnt/c/Users/alexg/Google Drive/Projects/Guitar Pedal/Software/Pedal/DSPSimulator/output/");
		strcat(outputFilePath, argv[2]);
	}
	else
	{
		outputFilePath = (char*) malloc( sizeof( "/mnt/c/Users/alexg/Google Drive/Projects/Guitar Pedal/Software/Pedal/DSPSimulator/output/output.wav"));
		strcpy(outputFilePath, "/mnt/c/Users/alexg/Google Drive/Projects/Guitar Pedal/Software/Pedal/DSPSimulator/output/output.wav");
	}
	if (argc > 3)
	{
		var = atof(argv[3]);
	}

	// Pitch variables
	steps = 12;
	shift = pow(2, steps/12);

	hopA = HOPA;
	hopS = (int)round(hopA * shift);
	cleanIdx = hopS*NUMFRAMES;
	sizeVTime = NUMFRAMES * hopS * 2;

	PRINT_LOG2("Input file: %s\n\n", inputFilePath);
	audio16 = readWav(&sizeData, inputFilePath);                                  // Get input audio from wav file and fetch the size
	NUM_SAMP = sizeData/sizeof(*audio16);                                    // Number of samples

	// Allocate and zero fill arrays 
	inbuffer     = (float *)      calloc(BUFLEN,      sizeof(float));        // Input array 
	outbuffer    = (float *)      calloc(BUFLEN,      sizeof(float));        // Output array 
	inframe      = (float *)      calloc(BUFLEN,      sizeof(float));        // Input frame
	vTime        = (float *)      calloc(sizeVTime,   sizeof(float));        // Overlap-add signal. Times two because of ping-pong.
	inwin        = (float *)      calloc(BUFLEN,      sizeof(float));        // Input window
	outwin       = (float *)      calloc(BUFLEN,      sizeof(float));        // Output window
	phi_a        = (float *)      calloc(BUFLEN,      sizeof(float));        // Phase of current frame
	phi_ping     = (float *)      calloc(BUFLEN,      sizeof(float));        // Phase adjusted for synthesis stage
	phi_pong     = (float *)      calloc(BUFLEN,      sizeof(float));        // Phase adjusted for synthesis stage
	delta_t_ping = (float *)      calloc(BUFLEN,      sizeof(float));        // Backward phase derivative in time domain
	delta_t_pong = (float *)      calloc(BUFLEN,      sizeof(float));        // Backward phase derivative in time domain
	delta_f      = (float *)      calloc(BUFLEN,      sizeof(float));        // Centered phase derivative in frequency domain
	mag_ping     = (float *)      calloc(BUFLEN,      sizeof(float));        // Magnitude of frame
	mag_pong     = (float *)      calloc(BUFLEN,      sizeof(float));        // Magnitude of frame
	cpxIn        = (kiss_fft_cpx*)calloc(BUFLEN,      sizeof(kiss_fft_cpx)); // Complex variable for FFT 
	cpxOut       = (kiss_fft_cpx*)calloc(BUFLEN,      sizeof(kiss_fft_cpx)); // Complex variable for FFT 
	in_audio     = (float *)      calloc(NUM_SAMP,    sizeof(float));        // Total input audio
	out_audio    = (float *)      calloc(NUM_SAMP,    sizeof(float));        // Total output audio

	cfg    = kiss_fft_alloc( BUFLEN, 0, 0, 0);
	cfgInv = kiss_fft_alloc( BUFLEN, 1, 0, 0);

	// Initialize the magnitude pointers
	mag = mag_ping;
	magPrev = mag_pong;

	// Initialize the synthesis phase pointers
	phi_s = phi_ping;
	phi_sPrev = phi_pong;

	// Initialize the time phase derivative pointers
	delta_t = delta_t_ping;
	delta_tPrev = delta_t_pong;

	// Initialize input and output window functions
	for(int k = 0; k < BUFLEN; k++){
		inwin[k]   =  HAMCONST * (1 - cos(2 * PI * k/BUFLEN))/sqrt((BUFLEN/hopA)/2);
		outwin[k]  =  HAMCONST * (1 - cos(2 * PI * k/BUFLEN))/sqrt((BUFLEN/hopS)/2);
	}
	
	// Convert 16bit audio to floating point
	for (unsigned long i = 0; i < NUM_SAMP; i++){
		in_audio[i] = ((float)audio16[i])/MAXVAL16;
	}

	PRINT_LOG2("Buffer length: %i.\n", BUFLEN);

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
			if (abs(out_audio[audio_ptr + k]) > 1)
			{
				out_audio[audio_ptr + k] = (out_audio[audio_ptr + k] < 0) ? -1 : 1;
			}
		}
		audio_ptr += BUFLEN;
	}

	printf("It took an average time of %f ms to process each frame.\n", 1000*avg_time/CLOCKS_PER_SEC);

	// Reconvert floating point audio to 16bit
	for (unsigned long i = 0; i < NUM_SAMP; i++)
	{
		audio16[i] = (uint16_t)(out_audio[i]*MAXVAL16);
	}

	// Save the processed audio to the output file
	writeWav(audio16, inputFilePath, outputFilePath);

	// Deallocate allocated memory
	free(audio16);
	free(inbuffer);
	free(outbuffer);
	free(inframe);
	free(vTime);
	free(inwin);
	free(outwin);
	free(phi_a);
	free(delta_t_ping);
	free(delta_t_pong);
	free(delta_f);
	free(mag_ping);
	free(mag_pong);
	free(phi_ping);
	free(phi_pong);
	free(cpxIn);
	free(cpxOut);
	free(in_audio);
	free(out_audio);
	free(coeffs);
	free(inputFilePath);
	free(outputFilePath);
	kiss_fft_free(cfg);
	kiss_fft_free(cfgInv);

	return 0;

}

void process_buffer()
{
	// mag, magPrev, mag_ping, mag_pong, inbuffer, inframe, hopA, frameNum, numFrames, cpxIn, inwin

	elapsed_time = clock();

	for (uint8_t f = 0; f < NUMFRAMES; f++)
	{
		
		// Update magnitude pointers
		magPrev = mag;
		mag = (mag == mag_ping) ? mag_pong : mag_ping;

		// Update phase pointers
		phi_sPrev = phi_s;
		phi_s = (phi_s == phi_ping) ? phi_pong : phi_ping;

		// Update time phase derivative pointers
		delta_tPrev = delta_t;
		delta_t = (delta_t == delta_t_ping) ? delta_t_pong : delta_t_ping;

/************ ANALYSIS STAGE ***********************/

		// Using mag as output buffer, nothing to do with magnitude
		overlapAdd(inbuffer, inframe, mag, hopA, frameNum, NUMFRAMES);  
		COPY(cpxIn[k].r, mag[k] * inwin[k], BUFLEN);
		COPY(cpxIn[k].i, 0, BUFLEN);

/************ PROCESSING STAGE *********************/

		DUMP_ARRAY_COMPLEX(cpxIn, BUFLEN, "debugData/cpxInXXXXX.csv", count, 5, audio_ptr, -1);

		kiss_fft( cfg , cpxIn , cpxOut );

		process_frame(cpxOut, mag, magPrev, phi_a, phi_s, phi_sPrev, delta_t, delta_tPrev, delta_f, hopA, hopS, shift, BUFLEN, var);

		// DUMP_ARRAY_COMPLEX(cpxOut, BUFLEN, "debugData/cpxOutXXXXX.csv", count, 40, audio_ptr,     -1);
		// DUMP_ARRAY(inbuffer      , BUFLEN, "debugData/inbuffer.csv"  , count, -1, audio_ptr, BUFLEN);
		// DUMP_ARRAY(inwin         , BUFLEN, "debugData/inwin.csv"     , count, -1, audio_ptr, BUFLEN);
		// DUMP_ARRAY(outwin        , BUFLEN, "debugData/outwin.csv"    , count, -1, audio_ptr, BUFLEN);
		// DUMP_ARRAY(phi_a         , BUFLEN, "debugData/phi_a.csv"     , count, -1, audio_ptr, BUFLEN);
		// DUMP_ARRAY(phi_s         , BUFLEN, "debugData/phi_s.csv"     , count, -1, audio_ptr, BUFLEN);

		kiss_fft( cfgInv , cpxOut , cpxIn );

		DUMP_ARRAY_COMPLEX(cpxIn, BUFLEN, "debugData/cpxOutXXXXX.csv", count, 5, audio_ptr,     -1);

		COPY(cpxOut[k].r, cpxIn[k].r * outwin[k]/BUFLEN, BUFLEN);


/************ SYNTHESIS STAGE ***********************/

		COPY(mag[k], cpxOut[k].r, BUFLEN);
		strechFrame(vTime, mag, &cleanIdx, hopS, frameNum, vTimeIdx, sizeVTime, BUFLEN);

		// DUMP_ARRAY(vTime, NUMFRAMES*hopS*2, "debugData/vTimeXXX.csv", count, 40, audio_ptr, -1);

		if ((++frameNum) >= NUMFRAMES) frameNum = 0;
		count++;

	}

/************* LINEAR INTERPOLATION *****************/

	interpolate(outbuffer, vTime, steps, shift, vTimeIdx, pOutBuffLastSample, hopS, BUFLEN);

	// PRINT_LOG1("********* Buffer is out *********\n");

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
		if ((count > max && max != -1) || (auP != auPMax && auPMax != -1)) { return; }

		int len = strlen(name) + 1; // Plus one for the \0
		char* fileName = (char*) malloc(sizeof(char)*len);
		strcpy(fileName, name);

		// fileName[len - 9] = (int)(count/1000) % 1000 + 48;
		// fileName[len - 8] = (int)(count/100) % 100 + 48;
		// fileName[len - 7] = (int)((count/10) % 10) + 48;
		// fileName[len - 6] = (int)(count%10) + 48;

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



#include "main.h"

int main()
{
	// Variable declaration
	unsigned long sizeData;                                 //Size of audio data in bytes
	FILE* outfile;
	const char* filePath = "/mnt/c/Users/alexg/Google Drive/Projects/Denoiser/clean_guitar.wav";
	#if DISTORTION
	coeffs = load_distortion_coefficients(&coeff_size);
  printf("Number of coefficients: %ld\n", coeff_size);
	#endif

	printf("Input file: %s\n\n", filePath);
	audio16 = readWav(&sizeData, filePath);                  // Get input audio from wav file and fetch the size
	NUM_SAMP = sizeData/sizeof(*audio16);                    // Number of samples

	// Allocate and zero fill arrays 
	inbuffer  = (float *) calloc(BUFLEN,   sizeof(float));	 // Input array 
	outbuffer = (float *) calloc(BUFLEN,   sizeof(float));   // Output array 
	inframe   = (float *) calloc(BUFLEN,   sizeof(float));   // Array for processing
	outframe  = (float *) calloc(BUFLEN,   sizeof(float));   // Array for processing
	inwin     = (float *) calloc(BUFLEN,   sizeof(float));   // Input window
	outwin    = (float *) calloc(BUFLEN,   sizeof(float));   // Output window
	phase 		= (float *) calloc(BUFLEN,   sizeof(float));   // Phase of current frame
	phi_s 		= (float *) calloc(BUFLEN,   sizeof(float));   // Phase adjusted for synthesis stage
	mag		 		= (float *) calloc(BUFLEN,   sizeof(float));   // Magnitude of current frame
	cpx       = (complex*)calloc(BUFLEN,   sizeof(complex)); // Complex variable for FFT 
	in_audio  = (float *) calloc(NUM_SAMP, sizeof(float));   // Total input audio
	out_audio = (float *) calloc(NUM_SAMP, sizeof(float));   // Total output audio

	
	// Initialize input and output window functions
	for(int k = 0; k < BUFLEN; k++){
		inwin[k] = sqrt(1.0 - WINCONST * cos(PI * (2 * k + 1)/BUFLEN)/OVERSAMP);
		outwin[k] = inwin[k];
	}
	
	// Clear freq.csv file of any content
	outfile = fopen("freq.csv", "w");
	fclose(outfile);

	// Convert 16bit audio to floating point
	for (unsigned long i = 0; i < NUM_SAMP; i++){
		in_audio[i] = ((float)audio16[i])/MAXVAL16;
	}

	while(audio_ptr < (NUM_SAMP - BUFLEN))
	{
		process_buffer();
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
	free(inwin);
	free(outwin);
	free(phase);
	free(phi_s);
	free(mag);
	free(cpx);
	free(in_audio);
	free(out_audio);
	free(coeffs);

	return 0;

}

void process_buffer()
{
	/* Variable declaration */
	int k;

	for (k = 0; k < BUFLEN; k++)
	{
		elapsed_time = clock();

		inbuffer[k] = in_audio[audio_ptr + k];
		inframe[k] = inbuffer[k];
		cpx[k] = inframe[k];
		outframe[k] = creal(cpx[k]);
		outbuffer[k] = outframe[k];
		out_audio[audio_ptr + k] = outbuffer[k];

		#if FFT
		// Compute the FFT of the frame
		fft(cpx, BUFLEN, 0);

		/* process_frame(); */

		// Reconstruct the time domain signal using the IFFT 
		fft(cpx, BUFLEN, 1);
		#endif


		#if DISTORTION
		float distortedSample = 0;
		for (int n = coeff_size - 1; n > -1; n--)
		{
			distortedSample += pow(outbuffer[m], coeff_size - n - 1) * coeffs[n];
		}
		outbuffer[m] = distortedSample;
		#endif

		elapsed_time = clock() - elapsed_time;
		avg_time = avg_time + (elapsed_time - avg_time)/N;
		N++;
	}

}

float* load_distortion_coefficients(size_t* coeff_size)
{
	FILE* inFile = fopen("polyCoeff.csv", "r");
	float coeff;
	*coeff_size = 0;
	size_t n = 0;

	if(inFile == NULL)
	{
        printf("Error opening file\n");
        exit(1);
	}
	while( fscanf(inFile, "%f\n", &coeff) != EOF )
	{
		*coeff_size += 1;
	}

	rewind(inFile);
	coeffs = (float *) calloc(*coeff_size, sizeof(float));
  printf("Coefficients:\n");

	while( fscanf(inFile, "%f\n", &coeff) != EOF )
	{
		coeffs[n] = coeff;
		printf("%f\n", coeffs[n]);
		n++;
	}

	return coeffs;

}

void process_frame()
{
	complex current_phase;
	float		omega_bin;
	float		delta_t_a = FSAMP/BUFLEN; 	// Analysis stage
	float		delta_t_s = delta_t_a;    	// Synthesis stage. Assume for now it is the same as delta_t_a.
	float		delta_omega;
	float		delta_omega_wrapped;
	float		omega_true;
	for(uint16_t k = 0; k < BUFLEN; k++)
	{
		mag[k] = 20 * log10(cabs(cpx[k]));
		current_phase = carg(cpx[k]);
		omega_bin = k * (FSAMP/BUFLEN);
		// Here phase[k] contains the value of the previous frame
		delta_omega = (current_phase - phase[k])/delta_t_a - omega_bin; 
		delta_omega_wrapped = fmod(delta_omega + PI , 2 * PI) - PI;
		omega_true = omega_bin + delta_omega_wrapped;
		phi_s[k] = phi_s[k] + delta_t_s * omega_true;
		phase[k] = current_phase;
	}	
}



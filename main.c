
#include "main.h"

int main(){

	// Variable declaration
	unsigned long sizeData;                                //Size of audio data in bytes
	const char* filePath = "/mnt/c/Users/alexg/Google Drive/Projects/Denoiser/pulp_fiction.wav";
	printf("%s\n", filePath);
	audio16 = readWav(&sizeData, filePath);                // Get input audio from wav file and fetch the size
	cplx frame[FFTLEN];					                   // Array that stores the FFT of a single frame of audio
	NUM_SAMP = sizeData/sizeof(*audio16);                  // Number of samples
	/* NUM_FFTS = (unsigned long)(NUM_SAMP/FFTLEN); */
	uint16_t sample;
	float out_sample;

	// Initialize and zero fill arrays 
	inbuffer = (float *) calloc(CIRCBUF, sizeof(float)); 	// Input array 
	outbuffer = (float *) calloc(CIRCBUF, sizeof(float));   // Output array 
	inframe = (float *) calloc(FFTLEN, sizeof(float));      // Array for processing
	outframe = (float *) calloc(FFTLEN, sizeof(float));     // Array for processing
	inwin = (float *) calloc(FFTLEN, sizeof(float));        // Input window
	outwin = (float *) calloc(FFTLEN, sizeof(float));       // Output window
	cpx = (cplx *) calloc(FFTLEN, sizeof(cplx));		    // Complex variable for FFT 
	in_audio = (float *) calloc(NUM_SAMP, sizeof(float));   // Total audio
	out_audio = (float *) calloc(NUM_SAMP, sizeof(float));  // Total audio

	
	// Initialize input and output window functions
	for(int k = 0; k < FFTLEN; k++){
		inwin[k] = sqrt((1.0 - WINCONST * cos(PI * (2 * k + 1)/FFTLEN))/OVERSAMP);
		outwin[k] = inwin[k];
	}
	
	// Clear freq.csv file of any content
	FILE *outfile;
	outfile = fopen("freq.csv", "w");
	fclose(outfile);

	// Set interrupt timer for input/output simulation
	/* memset(&sa, 0, sizeof(sa)); */
	/* sa.sa_handler = &buffer_interrupt; */
	/* sigaction ( SIGALRM, &sa, NULL ); */
	/* timer.it_value.tv_sec = 3; */
	/* timer.it_value.tv_usec = 0; */
	/* timer.it_interval.tv_sec = 0; */
	/* timer.it_interval.tv_usec = 23; */
    /* setitimer ( ITIMER_REAL, &timer, NULL ) ; */

	// Convert 16bit audio to floating point
	for (unsigned long i = 0; i < NUM_SAMP; i++){
		in_audio[i] = ((float)audio16[i])/MAXVAL16;
	}

	while(audio_ptr < NUM_SAMP){
		process_frame();
	}
	printf("It took an average time of %f ms to process each frame.\n", 1000*avg_time/CLOCKS_PER_SEC);


	// Reconvert floating point audio to 16bit
	for (unsigned long i = 0; i < NUM_SAMP; i++){
		audio16[i] = (uint16_t)(out_audio[i]*MAXVAL16);
	}

	// Save the processed audio to output.wav
	writeWav(audio16, filePath);

	return 0;
}


// This gets called when the timer runs out.  Try not to do too much here;
// the recommended practice is to set a flag (of type sig_atomic_t), and have
// code elsewhere check that flag (e.g. in the main loop of your program)
void buffer_interrupt(int sig)
{
		float sample;
		float out_sample;
		sample = in_audio[audio_ptr];

		inbuffer[io_ptr] = sample;

		out_sample = outbuffer[io_ptr];

		out_audio[audio_ptr++] = out_sample;

		/* Update io_ptr and check for buffer wraparound  */
		if (++io_ptr >= CIRCBUF){
			/* printf("%f\r", ((float)audio_ptr)/NUM_SAMP); */
			io_ptr = 0;
		}
}

void process_frame(){

	/* Variable declaration */
	int k, m;
	int io_ptr0;


	/* wait until io_ptr is at the start of the current frame */
	while((io_ptr/FRAMEINC) != frame_ptr){

		float sample;
		float out_sample;
		sample = in_audio[audio_ptr];

		inbuffer[io_ptr] = sample;
		out_sample = outbuffer[io_ptr];
		out_audio[audio_ptr++] = out_sample;

		/* Update io_ptr and check for
		Buffer wraparound  */
		if (++io_ptr >= CIRCBUF){
			/* printf("%f\r", ((float)audio_ptr)/NUM_SAMP); */
			io_ptr = 0;
		}
	}

	elapsed_time = clock();

	/* then increment the framecount (wrapping if required) */
	if (++frame_ptr >= (CIRCBUF/FRAMEINC)) frame_ptr = 0;

	/* save a pointer to the position in the I/O buffers (inbuffer/outbuffer) where the
	data should be read (inbuffer) and saved (outbuffer) for the purpose of processing */
	io_ptr0 = frame_ptr * FRAMEINC;

	/* copy input data from inbuffer into inframe (starting from the pointer position) */
	m = io_ptr0;
	for(k = 0; k < FFTLEN; k++){

		inframe[k] = inbuffer[m] * inwin[k];
		cpx[k] = inframe[k];
		if (++m >= CIRCBUF) m = 0; /* wrap if required */

	}	

	// Compute the FFT of the frame
	fft(cpx, FFTLEN, 0);

	// Store the frame frequency transform on freq.csv
	/* show("FFT: \n", cpx); */

	// Reconstruct the time domain signal using the IFFT 
	fft(cpx, FFTLEN, 1);

	for(k = 0; k < FFTLEN; k++){
		outframe[k] = creal(cpx[k]);
	}

	/* multiply outframe by output window and overlap-add into output buffer */
	m = io_ptr0;
	for (k=0;k<(FFTLEN-FRAMEINC);k++){

		/* this loop adds into outbuffer */
		outbuffer[m] = outbuffer[m]+outframe[k]*outwin[k];
		if (++m >= CIRCBUF) m=0; /* wrap if required */
	}

	for(;k<FFTLEN;k++){
		outbuffer[m] = outframe[k] * outwin[k]; /* this loop over-writes outbuffer */
		m++;
	}
	elapsed_time = clock() - elapsed_time;
	avg_time = avg_time + (elapsed_time - avg_time)/N;
	N++;
}

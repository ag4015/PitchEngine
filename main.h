#ifndef MAIN_H
#define MAIN_H

#include "wavio.h"
#include "fft.h"
#include <math.h>
#include <signal.h>
#include <sys/time.h>
#include <time.h>

#define NFREQ (1 + FFTLEN/2)
#define OVERSAMP 4
#define FRAMEINC (FFTLEN/OVERSAMP)
#define CIRCBUF (FFTLEN + FRAMEINC)
#define MAXVAL16 32768
#define WINCONST 0.85185

// Global variables declaration
float *inbuffer, *outbuffer; 		// Input/output circular buffers
float *inframe, *outframe; 			// Input and output frames
float *inwin, *outwin;				// Input and output windows
float *in_audio, *out_audio;		// Complete audio data from wav file for input and output
uint16_t *audio16;
cplx *cpx;							// Complex variable for FFT 
volatile int io_ptr = 0;			// Input/output pointer for circular buffers
volatile int frame_ptr = 0;			// Frame pointer 
unsigned long int audio_ptr = 0;    // Wav file sample pointer
unsigned long NUM_SAMP;				// Total number of samples in wave file
struct sigaction sa;				// Set interrupt timer for input/output simulation
struct itimerval timer;				// Set interrupt timer for input/output simulation
float avg_time = 0;
float elapsed_time = 0;
float N = 1;

// Function declaration
void buffer_interrupt(int sig);
void sigalrm_handler(int sig);
void process_frame();

#endif // MAIN_H
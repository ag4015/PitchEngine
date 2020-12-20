#ifndef MAIN_H
#define MAIN_H

#include "wavio.h"
#include "fft.h"
#include <math.h>
#include <signal.h>
#include <sys/time.h>
#include <time.h>

#define NFREQ      (1 + BUFLEN/2)         // Number of unique frequency bins
#define NUMFRAMES   4                     // Number of frames that overlap in a buffer. 75% overlap for 4 frames.
// #define HOPA       (BUFLEN/NUMFRAMES)     // Hopcount or number of samples between frames for the analysis stage
// #define HOPS       (2*HOPA)               // Hopcount or number of samples between frames for the synthesis stage. 2*HOPA because we are doubling in frequency.
#define DTA        (BUFLEN/FSAMP)         // Delta t_a. Time between frames.
#define MAXVAL16   32768/1.5              // Maximum
#define WINCONST   0.85185                // Constant used for the hamming window
#define HAMCONST   0.53836                // Constant used for the hamming window
#define FFT        1                      // FFT = 1: Compute the FFT and IFFT of input audio
#define DISTORTION 0                      // DISTORTION = 1: Apply a polynomial function to the input audio
#define GAIN       1
// #define PDEBUG                            // Print debug information

#ifdef PDEBUG
       #define PRINT_LOG1(x)      printf(x)
       #define PRINT_LOG2(x,y)    printf(x,y)
       #define PRINT_LOG3(x,y,z)  printf(x,y,z)
#else
       #define PRINT_LOG1(x) 
       #define PRINT_LOG2(x,y) 
       #define PRINT_LOG3(x,y,z) 
#endif

// Global variables declaration
float *inbuffer, *outbuffer;              // Input and output buffers
float *inframe[NUMFRAMES];                // Pointer to the current frame
float *inwin, *outwin;                    // Input and output windows
float *vTime;                             // Overlap-add signal
float *in_audio, *out_audio;              // Complete audio data from wav file for input and output
float *mag, *phase;                       // Frame magnitude and phase
float *phi_s;                             // Phase adjusted for synthesis stage
float *coeffs = NULL;                     // Coefficients from the distortion polynomial
size_t coeff_size;                        // Number of coefficients pointed at by coeffs
int16_t *audio16;                         // 16 bit integer representation of the audio
complex *cpx;                             // Complex variable for FFT 
volatile int io_ptr = 0;                  // Input/output pointer for circular buffers
volatile int frame_ptr = 0;               // Frame pointer 
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

// Function declaration
void buffer_interrupt(int sig);
void sigalrm_handler(int sig);
void process_buffer();
void process_frame();
float* load_distortion_coefficients(size_t* coeff_size);

#endif // MAIN_H

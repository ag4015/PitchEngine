#ifndef MAIN_H
#define MAIN_H

#include <math.h>
#include <stdint.h>
#include <stddef.h>

#define BUFLEN     2048
#define PI         (float)3.1415926535 
#define NFREQ      (1 + BUFLEN/2)       // Number of unique frequency bins
#define HOPA       256
#define NUMFRAMES  (int)BUFLEN/HOPA     // Number of frames that overlap in a buffer. 75% overlap for 4 frames.
#define MAXVAL16   32768/1.5            // Maximum
#define WINCONST   0.85185              // Constant used for the hamming window
#define HAMCONST   0.53836              // Constant used for the hamming window
#define DISTORTION 0                    // DISTORTION = 1: Apply a polynomial function to the input audio
#define INGAIN     1
#define OUTGAIN    2.5
#define FSAMP      44100
// #define PDEBUG                          // Print debug information
#define SIMULATION                      // This is a simulation of the device

#define COPY(x,y,z) for(uint16_t k = 0; k < z; k++) { x = y; } 

#ifdef PDEBUG
       #define PRINT_LOG1(x)       printf(x)
       #define PRINT_LOG2(x,y)     printf(x,y)
       #define PRINT_LOG3(x,y,z)   printf(x,y,z)
       #define DUMP_ARRAY(a,b,c,d,e,f,g) dumpFloatArray(a,b,c,d,e,f,g)
       #define DUMP_ARRAY_COMPLEX(a,b,c,d,e,f,g) COPY(mag[k], a[k].r, b); dumpFloatArray(mag,b,c,d,e,f,g)
#else
       #define PRINT_LOG1(x) 
       #define PRINT_LOG2(x,y) 
       #define PRINT_LOG3(x,y,z) 
       #define DUMP_ARRAY(a,b,c,d,e,f,g)
       #define DUMP_ARRAY_COMPLEX(a,b,c,d,e,f,g)
#endif

// Function declaration
#pragma once
#ifdef __cplusplus
extern "C"
{
#endif

void buffer_interrupt(int sig);
void sigalrm_handler(int sig);
void process_buffer();
float* load_distortion_coefficients(size_t* coeff_size);
void dumpFloatArray(float* buf, size_t size, const char* name, int count, int max, int auP, int auPMax);

#ifdef __cplusplus
} // extern C
#endif

#endif // MAIN_H


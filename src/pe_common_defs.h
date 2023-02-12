#pragma once
#include "kiss_fft.h"

#define WINCONST               (my_float)0.5            // Constant used for the hamming window
#define HAMCONST               (my_float)0.53836        // Constant used for the hamming window
#define PI                     (my_float)3.1415926535   // Pi constant
#define MAXVAL16               (my_float)32768          // Maximum
#define SEMITONES_PER_OCTAVE   (my_float)12.0
#define ANALYSIS_FRAME_OVERLAP (my_float)75.0

typedef kiss_fft_cpx cpx;

#ifdef USE_DOUBLE
typedef double my_float;
#else
typedef float my_float;
#endif

#ifdef USE_DOUBLE
	#define POW(x,y) pow(x,static_cast<my_float>(y))
	#define ROUND(x) round(static_cast<my_float>(x))
	#define SQRT(x)  sqrt(x)
	#define COS(x)   cos(x)
#else
	#define POW(x,y) powf(x,static_cast<my_float>(y))
	#define ROUND(x) roundf(static_cast<my_float>(x))
	#define SQRT(x)  sqrtf(x)
	#define COS(x)   cosf(x)
#endif


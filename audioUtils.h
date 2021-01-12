
#ifndef AUDIOUTILS_H
#define AUDIOUTILS_H

#include <math.h>
#include "kissfft/kiss_fft.h"
#include "kissfft/_kiss_fft_guts.h"
#include "main.h"

kiss_fft_cpx *cpxIn, *cpxOut;                  // Complex variable for FFT 
kiss_fft_cfg cfg;
kiss_fft_cfg cfgInv;

#ifdef PDEBUG
void process_frame(float* mag, kiss_fft_cpx* cpxOut, float* phase, float* deltaPhi, float* previousPhase,
                   float* deltaPhiPrime, float* deltaPhiPrimeMod, float* trueFreq, float* phaseCumulative,
									 int hopA, int hopS, int bufLen);
#else

void process_frame(kiss_fft_cpx* input, float* mag, float* phase, float* phaseCumulative,
                   int hopA, int hopS, int bufLen);
#endif
float absc(kiss_fft_cpx *a);
float argc(kiss_fft_cpx *a);
void expc(kiss_fft_cpx *a, float mag, float phase);

void overlapAdd(float* input, float* frame, float* output, int hop, uint8_t frameNum, int numFrames);

void strechFrame(float* output, float* input, int* cleanIdx, int hop,
                 int frameNum, int outputIdx, int outputSize, int bufLen);

void interpolate(float* outbuffer, float* vTime, int steps, float shift,
		             int vTimeIdx, float pOutBuffLastSample, int hopS, int bufLen);

#endif // AUDIOUTILS_H


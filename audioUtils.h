
#ifndef AUDIOUTILS_H
#define AUDIOUTILS_H

#include <math.h>
#include "kissfft/kiss_fft.h"
#include "kissfft/_kiss_fft_guts.h"
#include "main.h"

extern "C"
{

void process_frame(kiss_fft_cpx* input, float* mag, float* magPrev, float* phi_a, float* phi_s,
                   float* delta_t_back, float* delta_f_cent, int hopA, int hopS, int bufLen);

void propagate_phase(float* delta_t, float* delta_f,float* mag, float* magPrev,float* phi_s, float tol, int bufLen);
float get_max(float* in, int size);
float absc(kiss_fft_cpx *a);
float argc(kiss_fft_cpx *a);
void expc(kiss_fft_cpx *a, float mag, float phase);

void overlapAdd(float* input, float* frame, float* output, int hop, uint8_t frameNum, int numFrames);

void strechFrame(float* output, float* input, int* cleanIdx, int hop,
                 int frameNum, int outputIdx, int outputSize, int bufLen);

void interpolate(float* outbuffer, float* vTime, int steps, float shift,
		             int vTimeIdx, float pOutBuffLastSample, int hopS, int bufLen);

}

#endif // AUDIOUTILS_H


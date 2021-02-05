
#ifndef AUDIOUTILS_H
#define AUDIOUTILS_H

#include <math.h>
#include "kissfft/kiss_fft.h"
#include "kissfft/_kiss_fft_guts.h"
#include "main.h"

extern "C"
{

void process_frame(kiss_fft_cpx* input, float* mag, float* magPrev, float* phi_a, float* phi_s, float* phi_sPrev,
                   float* delta_t, float* delta_tPrev, float* delta_f, int hopA, int hopS, float shift, int bufLen, float var);

void propagate_phase(float* delta_t, float* delta_tPrev, float* delta_f, float* mag, float* magPrev, float* phi_s, float* phi_sPrev, float hopA, float shift, int bufLen, float b_s, float abstol);
float absc(kiss_fft_cpx *a);
float argc(kiss_fft_cpx *a);
void expc(kiss_fft_cpx *a, float& mag, float& phase);

void overlapAdd(float* input, float* frame, float* output, int hop, uint8_t frameNum, int numFrames);

void strechFrame(float* output, float* input, int* cleanIdx, int hop,
                 int frameNum, int outputIdx, int outputSize, int bufLen);

void interpolate(float* outbuffer, float* vTime, int steps, float shift,
		             int vTimeIdx, float pOutBuffLastSample, int hopS, int bufLen);
float get_max(const float* in, int size);

} // extern C

#endif // AUDIOUTILS_H

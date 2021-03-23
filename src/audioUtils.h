
#ifndef AUDIOUTILS_H
#define AUDIOUTILS_H

#include <math.h>
#include <stdint.h>
#include "kissfft/kiss_fft.h"
#include "kissfft/_kiss_fft_guts.h"
#include "audioData.h"

#pragma once
#ifdef __cplusplus
extern "C"
{
#endif

void process_frame(buffer_data_t* bf, float var);
void propagate_phase(float* delta_t, float* delta_tPrev, float* delta_f, float* mag, float* magPrev, float* phi_s, float* phi_sPrev, float hopA, float shift, int bufLen, float b_s, float abstol);
float absc(kiss_fft_cpx *a);
float argc(kiss_fft_cpx *a);
void expc(kiss_fft_cpx *a, float* mag, float* phase);

void overlapAdd(float* input, float* frame, float* output, int hop, uint8_t frameNum, int numFrames);

void strechFrame(float* output, float* input, uint32_t* cleanIdx, uint32_t hop,
	uint8_t frameNum, uint32_t outputIdx, uint32_t outputSize, uint32_t bufLen);

void interpolate(float* outbuffer, float* vTime, uint8_t steps, float shift,
	uint32_t vTimeIdx, float pOutBuffLastSample, uint32_t hopS, uint32_t bufLen);
float get_max(const float* in, int size);


#ifdef __cplusplus
} // extern C
#endif

#endif // AUDIOUTILS_H

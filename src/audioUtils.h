
#pragma once

#include <math.h>
#include <stdint.h>
#include "kissfft/kiss_fft.h"
#include "kissfft/_kiss_fft_guts.h"

#ifdef __cplusplus
extern "C"
{
#endif

#include "audioData.h"

void process_buffer(buffer_data_t* bf, audio_data_t* audat, uint8_t frameNum,
	uint32_t audio_ptr, uint32_t* vTimeIdx, uint32_t* cleanIdx, float pOutBuffLastSample, float var);
void process_frame(buffer_data_t* bf, audio_data_t* audat, float var);
void propagate_phase(buffer_data_t* bf, audio_data_t* audat, float b_s, float abstol);
float absc(kiss_fft_cpx *a);
float argc(kiss_fft_cpx *a);
void expc(kiss_fft_cpx *a, float* mag, float* phase);

void overlapAdd(float* input, float* frame, float* output, int hop, uint8_t frameNum, int numFrames);

void strechFrame(float* output, float* input, uint32_t* cleanIdx, uint32_t hop,
	uint8_t frameNum, uint32_t outputIdx, uint32_t outputSize, uint32_t bufLen);

void interpolate(buffer_data_t* bf, audio_data_t* audat, uint32_t vTimeIdx, float pOutBuffLastSample);


#ifdef __cplusplus
} // extern C
#endif


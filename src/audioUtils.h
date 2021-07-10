
#pragma once

#include "DSPConfig.h"
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
	uint32_t audio_ptr, uint32_t* vTimeIdx, uint32_t* cleanIdx, my_float* pOutBuffLastSample);
void process_frame(buffer_data_t* bf, audio_data_t* audat);
void propagate_phase(buffer_data_t* bf, audio_data_t* audat, my_float b_s, my_float abstol);

void overlapAdd(my_float* input, my_float* frame, my_float* output, int hop, uint8_t frameNum, int numFrames);

void strechFrame(my_float* output, my_float* input, uint32_t* cleanIdx, uint32_t hop,
	uint8_t frameNum, uint32_t outputIdx, uint32_t outputSize, uint32_t bufLen);

void interpolate(buffer_data_t* bf, audio_data_t* audat, uint32_t vTimeIdx, my_float* pOutBuffLastSample);


#ifdef __cplusplus
} // extern C
#endif


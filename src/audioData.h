#pragma once

#include "DSPConfig.h"
#include "stdint.h"
#include "kiss_fft.h"

#define WINCONST   (my_float)0.5            // Constant used for the hamming window
#define HAMCONST   (my_float)0.53836        // Constant used for the hamming window
#define BUFLEN     (uint32_t) 1024          // Size of the buffer
#define HOPA       (uint32_t) 256           // Size of the frame in the analysis stage
//#define NUMFRAMES  (uint32_t) BUFLEN/HOPA     
#define PI         (my_float)3.1415926535        // Pi constant
#define MAXVAL16   (my_float)32768               // Maximum

typedef kiss_fft_cpx cpx;

typedef struct audio_data
{
	my_float *inbuffer, *outbuffer;              // Input and output buffers
	my_float *inframe, *outframe;                // Pointer to the current frame
	my_float *inwin, *outwin;                    // Input and output windows
	my_float *vTime;                             // Overlap-add signal
	my_float *in_audio, *out_audio;              // Complete audio data from wav file for input and output
    my_float *mag_ping, *mag_pong;               // Frame magnitude ping-pong buffers
    my_float *phi_ping, *phi_pong;               // Frame phase ping-pong buffers
	my_float *delta_t_ping, *delta_t_pong;       // Frame phase time derivative ping-pong buffers
	uint32_t cleanIdx;                           // Circular index where vTime is reset.
	uint32_t numFrames;                          // Number of frames
	uint32_t sampleRate;                         // Sample rate of input audio
} audio_data_t;

typedef struct buffer_data 
{
	cpx* cpxIn;
	cpx* cpxOut;
	my_float* mag;
	my_float* magPrev;
	my_float* phi_a;
	my_float* phi_aPrev;
	my_float* phi_s;
	my_float* phi_sPrev;
	my_float* delta_t;
	my_float* delta_tPrev;
	my_float* delta_f;
	my_float  shift;
	my_float  maxMagPrev;
	uint32_t steps;                            // Number of semitones by which to shift the signal
	uint32_t hopA;
	uint32_t hopS;
	uint32_t buflen;
	kiss_fft_cfg cfg;
	kiss_fft_cfg cfgInv;
} buffer_data_t;

#ifdef __cplusplus
extern "C"
{
#endif

void init_variables(buffer_data_t* bf, audio_data_t* audat, uint32_t numSamp, my_float* in_audio, uint32_t sampleRate, uint32_t steps, uint32_t buflen);
void swap_ping_pong_buffer_data(buffer_data_t* bf, audio_data_t* audat);
void initialize_audio_data(audio_data_t* audat, uint32_t hopS, uint32_t numFrames, uint32_t numSamp, uint32_t sampleRate, uint32_t bufLen, my_float* in_audio);
void initialize_buffer_data(buffer_data_t* bf, audio_data_t* audat, my_float shift, uint32_t hopS, uint32_t steps, uint32_t hopA, uint32_t bufLen);
audio_data_t alloc_audio_data(uint32_t vTimeSize, uint32_t numSamp, uint32_t bufLen);
buffer_data_t alloc_buffer_data(uint32_t bufLen);
void free_audio_data(audio_data_t* audat);
void free_buffer_data(buffer_data_t* bf);
void reset_buffer_data_arrays(buffer_data_t* bf);

#ifdef __cplusplus
}
#endif

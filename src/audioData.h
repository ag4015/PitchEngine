#pragma once
#include "stdint.h"
#include "kissfft/kiss_fft.h"

typedef struct audio_data
{
	float *inbuffer, *outbuffer;              // Input and output buffers
	float *inframe;                           // Pointer to the current frame
	float *inwin, *outwin;                    // Input and output windows
	float *vTime;                             // Overlap-add signal
	float *in_audio, *out_audio;              // Complete audio data from wav file for input and output
    float *mag_ping, *mag_pong;               // Frame magnitude ping-pong buffers
    float *phi_ping, *phi_pong;               // Frame phase ping-pong buffers
	float *delta_t_ping, *delta_t_pong;       // Frame phase time derivative ping-pong buffers
    uint32_t numSamp;                         // Total number of samples in wave file
} audio_data_t;

typedef struct buffer_data 
{
	kiss_fft_cpx* input;
	kiss_fft_cpx* cpxIn;
	kiss_fft_cpx* cpxOut;
	float* mag;
	float* magPrev;
	float* phi_a;
	float* phi_s;
	float* phi_sPrev;
	float* delta_t;
	float* delta_tPrev;
	float* delta_f;
	float  shift;
	uint8_t steps;                            // Number of semitones by which to shift the signal
	uint32_t hopA;
	uint32_t hopS;
	uint32_t buflen;
	kiss_fft_cfg cfg;
	kiss_fft_cfg cfgInv;
} buffer_data_t;

void swap_ping_pong_buffer_data(buffer_data_t* bf, audio_data_t* audat);
void initialize_audio_data(audio_data_t* audat, uint32_t hopS, uint8_t numFrames, uint32_t numSamp, uint32_t bufLen, float* in_audio);
void initialize_buffer_data(buffer_data_t* bf, audio_data_t* audat, float shift, uint32_t hopS, uint8_t steps, uint32_t hopA, uint32_t bufLen);
audio_data_t alloc_audio_data(uint32_t vTimeSize, uint32_t numSamp, uint32_t bufLen);
buffer_data_t alloc_buffer_data(uint32_t bufLen);
void free_audio_data(audio_data_t* audat);
void free_buffer_data(buffer_data_t* bf);

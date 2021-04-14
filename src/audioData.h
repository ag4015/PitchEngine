#pragma once
#include "stdint.h"
#include "kissfft/kiss_fft.h"

#define WINCONST   0.5                        // Constant used for the hamming window
#define HAMCONST   (float)0.53836             // Constant used for the hamming window
#define BUFLEN     (uint32_t) 1024            // Size of the buffer
#define HOPA       (uint32_t) 256             // Size of the frame in the analysis stage
#define NUMFRAMES  (uint32_t) BUFLEN/HOPA     // Number of frames that overlap in a buffer. 75% overlap for 4 frames.
#define PI         (float)3.1415926535        // Pi constant
#define MAXVAL16   (float)32768               // Maximum

typedef kiss_fft_cpx cpx;

typedef struct audio_data
{
	float *inbuffer, *outbuffer;              // Input and output buffers
	float *inframe, *outframe;                // Pointer to the current frame
	float *inwin, *outwin;                    // Input and output windows
	float *vTime;                             // Overlap-add signal
	float *in_audio, *out_audio;              // Complete audio data from wav file for input and output
    float *mag_ping, *mag_pong;               // Frame magnitude ping-pong buffers
    float *phi_ping, *phi_pong;               // Frame phase ping-pong buffers
	float *delta_t_ping, *delta_t_pong;       // Frame phase time derivative ping-pong buffers
	uint32_t cleanIdx;                        // Circular index where vTime is reset.
	uint8_t numFrames;                        // Number of frames
} audio_data_t;

typedef struct buffer_data 
{
	cpx* cpxIn;
	cpx* cpxOut;
	float* mag;
	float* magPrev;
	float* phi_a;
	float* phi_aPrev;
	float* phi_s;
	float* phi_sPrev;
	float* delta_t;
	float* delta_tPrev;
	float* delta_f;
	float  shift;
	float  maxMagPrev;
	uint8_t steps;                            // Number of semitones by which to shift the signal
	uint32_t hopA;
	uint32_t hopS;
	uint32_t buflen;
	kiss_fft_cfg cfg;
	kiss_fft_cfg cfgInv;
} buffer_data_t;

void init_variables(buffer_data_t* bf, audio_data_t* audat, uint32_t numSamp, float* in_audio, uint8_t steps);
void swap_ping_pong_buffer_data(buffer_data_t* bf, audio_data_t* audat);
void initialize_audio_data(audio_data_t* audat, uint32_t hopS, uint8_t numFrames, uint32_t numSamp, uint32_t bufLen, float* in_audio);
void initialize_buffer_data(buffer_data_t* bf, audio_data_t* audat, float shift, uint32_t hopS, uint8_t steps, uint32_t hopA, uint32_t bufLen);
audio_data_t alloc_audio_data(uint32_t vTimeSize, uint32_t numSamp, uint32_t bufLen);
buffer_data_t alloc_buffer_data(uint32_t bufLen);
void free_audio_data(audio_data_t* audat);
void free_buffer_data(buffer_data_t* bf);
void reset_buffer_data_arrays(buffer_data_t* bf);

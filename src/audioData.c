
#include "audioData.h"
#include "DSPConfig.h"

void init_variables(buffer_data_t* bf, audio_data_t* audat, uint32_t numSamp, my_float* in_audio, uint32_t sampleRate, uint8_t steps, uint32_t buflen)
{
	// Pitch variables
	my_float shift = powf(2, ((my_float)steps)/12);
	uint32_t hopS = (int32_t)roundf(HOPA * shift);
	uint32_t numFrames = (uint32_t) (BUFLEN / HOPA); // Number of frames that overlap in a buffer. 75% overlap for 4 frames.

	// Initialize structures
	initialize_audio_data(audat, hopS, numFrames, numSamp, sampleRate, buflen, in_audio);
	initialize_buffer_data(bf, audat, shift, hopS, steps, HOPA, buflen);

	// Initialize input and output window functions
	for(uint32_t k = 0; k < buflen; k++)
	{
		audat->inwin[k] = WINCONST * (1 - cos(2 * PI * k / buflen));
		audat->outwin[k] = WINCONST * (1 - cos(2 * PI * k / buflen));
	}
}

void swap_ping_pong_buffer_data(buffer_data_t* bf, audio_data_t* audat)
{
	// Update magnitude pointers
	bf->magPrev = bf->mag;
	bf->mag = (bf->mag == audat->mag_ping) ? audat->mag_pong : audat->mag_ping;

	// Phase pointers are not updated
	bf->phi_sPrev = bf->phi_s;
	bf->phi_s = (bf->phi_s == audat->phi_ping) ? audat->phi_pong : audat->phi_ping;

	// Update time phase derivative pointers
	bf->delta_tPrev = bf->delta_t;
	bf->delta_t = (bf->delta_t == audat->delta_t_ping) ? audat->delta_t_pong : audat->delta_t_ping;
}
void initialize_audio_data(audio_data_t* audat, uint32_t hopS, uint8_t numFrames, uint32_t numSamp, uint32_t sampleRate, uint32_t bufLen, my_float* in_audio)
{
	// Allocate and zero fill arrays 
	*audat = alloc_audio_data(hopS * numFrames * 2, numSamp, bufLen);
	audat->in_audio = in_audio;
	audat->numFrames = numFrames;
	audat->cleanIdx = hopS * numFrames;
	audat->sampleRate = sampleRate;
}

void initialize_buffer_data(buffer_data_t* bf, audio_data_t* audat, my_float shift, uint32_t hopS, uint8_t steps, uint32_t hopA, uint32_t bufLen)
{
	*bf = alloc_buffer_data(bufLen);

	bf->hopA = hopA;
	bf->hopS = hopS;
	bf->shift = shift;
	bf->steps = steps;
	bf->buflen = bufLen;
	bf->maxMagPrev = 0;

	// Initialize the magnitude pointers
	bf->mag = audat->mag_ping;
	bf->magPrev = audat->mag_pong;

	// Initialize the synthesis phase pointers
	bf->phi_s = audat->phi_ping;
	bf->phi_sPrev = audat->phi_pong;

	// Initialize the time phase derivative pointers
	bf->delta_t = audat->delta_t_ping;
	bf->delta_tPrev = audat->delta_t_pong;
}

audio_data_t alloc_audio_data(uint32_t vTimeSize, uint32_t numSamp, uint32_t bufLen)
{
	audio_data_t audat;
	audat.vTime        = (my_float *)  calloc(vTimeSize, sizeof(my_float));
	audat.inbuffer     = (my_float *)  calloc(bufLen,    sizeof(my_float));
	audat.outbuffer    = (my_float *)  calloc(bufLen,    sizeof(my_float));
	audat.inframe      = (my_float *)  calloc(bufLen,    sizeof(my_float));
	audat.outframe     = (my_float *)  calloc(bufLen,    sizeof(my_float));
	audat.inwin        = (my_float *)  calloc(bufLen,    sizeof(my_float));
	audat.outwin       = (my_float *)  calloc(bufLen,    sizeof(my_float));
	audat.phi_ping     = (my_float *)  calloc(bufLen,    sizeof(my_float));
	audat.phi_pong     = (my_float *)  calloc(bufLen,    sizeof(my_float));
	audat.delta_t_ping = (my_float *)  calloc(bufLen,    sizeof(my_float));
	audat.delta_t_pong = (my_float *)  calloc(bufLen,    sizeof(my_float));
	audat.mag_ping     = (my_float *)  calloc(bufLen,    sizeof(my_float));
	audat.mag_pong     = (my_float *)  calloc(bufLen,    sizeof(my_float));
	audat.out_audio    = (my_float *)  calloc(numSamp,   sizeof(my_float));
	return audat;
}

buffer_data_t alloc_buffer_data(uint32_t bufLen)
{
	buffer_data_t bf;
	bf.phi_a           = (my_float *)    calloc(bufLen, sizeof(my_float));
	bf.phi_aPrev       = (my_float *)    calloc(bufLen, sizeof(my_float));
	bf.delta_f         = (my_float *)    calloc(bufLen, sizeof(my_float));
	bf.cpxIn           = (kiss_fft_cpx*) calloc(bufLen, sizeof(kiss_fft_cpx));
	bf.cpxOut          = (kiss_fft_cpx*) calloc(bufLen, sizeof(kiss_fft_cpx));
	bf.cfg             = kiss_fft_alloc(bufLen, 0, 0, 0);
	bf.cfgInv          = kiss_fft_alloc(bufLen, 1, 0, 0);
	return bf;
}

void free_audio_data(audio_data_t* audat)
{
	free(audat->inbuffer);
	free(audat->outbuffer);
	free(audat->inframe);
	free(audat->outframe);
	free(audat->vTime);
	free(audat->inwin);
	free(audat->outwin);
	free(audat->delta_t_ping);
	free(audat->delta_t_pong);
	free(audat->mag_ping);
	free(audat->mag_pong);
	free(audat->phi_ping);
	free(audat->phi_pong);
	free(audat->in_audio);
	free(audat->out_audio);
}

void free_buffer_data(buffer_data_t* bf)
{
	free(bf->phi_a);
	free(bf->phi_aPrev);
	free(bf->delta_f);
	free(bf->cpxIn);
	free(bf->cpxOut);
	kiss_fft_free(bf->cfg);
	kiss_fft_free(bf->cfgInv);
}

void reset_buffer_data_arrays(buffer_data_t* bf)
{
	for (uint32_t k = 0; k < bf->buflen; k++)
	{
		bf->cpxIn[k].r = 0;
		bf->cpxIn[k].i = 0;
		bf->cpxOut[k].r = 0;
		bf->cpxOut[k].i = 0;
		bf->mag[k] = 0;
		bf->magPrev[k] = 0;
		bf->phi_a[k] = 0;
		bf->phi_aPrev[k] = 0;
		bf->phi_s[k] = 0;
		bf->phi_sPrev[k] = 0;
		bf->delta_t[k] = 0;
		bf->delta_tPrev[k] = 0;
		bf->delta_f[k] = 0;
	}
}

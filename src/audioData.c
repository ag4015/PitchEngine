
#include "audioData.h"
#include "DSPConfig.h"

void init_variables(buffer_data_t* bf, audio_data_t* audat, uint32_t numSamp, float* in_audio, uint8_t steps)
{
	// Pitch variables
	float shift = powf(2, ((float)steps)/12);
	uint32_t hopS = (int32_t)roundf(HOPA * shift);

	// Initialize structures
	initialize_audio_data(audat, hopS, NUMFRAMES, numSamp, BUFLEN, in_audio);
	initialize_buffer_data(bf, audat, shift, hopS, steps, HOPA, BUFLEN);

	// Initialize input and output window functions
	for(uint16_t k = 0; k < BUFLEN; k++){
		audat->inwin[k] = WINCONST * (1 - cos(2 * PI * k / BUFLEN));
		audat->outwin[k] = WINCONST * (1 - cos(2 * PI * k / BUFLEN));
		//audat->inwin[k] = 0.5;
		//audat->outwin[k] = 0.5;
	}

	//for (uint32_t i = 0; i < numSamp; i++){
	//	audat->in_audio[i] = audat->in_audio[i] / MAXVAL16;
	//}
}

void swap_ping_pong_buffer_data(buffer_data_t* bf, audio_data_t* audat)
{
	// Update magnitude pointers
	bf->magPrev = bf->mag;
	bf->mag = (bf->mag == audat->mag_ping) ? audat->mag_pong : audat->mag_ping;

	// Phase pointers are not updated
	//bf->phi_sPrev = bf->phi_s;
	//bf->phi_s = (bf->phi_s == audat->phi_ping) ? audat->phi_pong : audat->phi_ping;

	// Update time phase derivative pointers
	bf->delta_tPrev = bf->delta_t;
	bf->delta_t = (bf->delta_t == audat->delta_t_ping) ? audat->delta_t_pong : audat->delta_t_ping;
}
void initialize_audio_data(audio_data_t* audat, uint32_t hopS, uint8_t numFrames, uint32_t numSamp, uint32_t bufLen, float* in_audio)
{
	// Allocate and zero fill arrays 
	*audat = alloc_audio_data(hopS * numFrames * 2, numSamp, bufLen);
	audat->in_audio = in_audio;
	audat->numFrames = numFrames;
	audat->cleanIdx = hopS * numFrames;
}

void initialize_buffer_data(buffer_data_t* bf, audio_data_t* audat, float shift, uint32_t hopS, uint8_t steps, uint32_t hopA, uint32_t bufLen)
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
	audat.vTime        = (float *)  calloc(vTimeSize, sizeof(float));
	audat.inbuffer     = (float *)  calloc(bufLen,    sizeof(float));
	audat.outbuffer    = (float *)  calloc(bufLen,    sizeof(float));
	audat.inframe      = (float *)  calloc(bufLen,    sizeof(float));
	audat.outframe     = (float *)  calloc(bufLen,    sizeof(float));
	audat.inwin        = (float *)  calloc(bufLen,    sizeof(float));
	audat.outwin       = (float *)  calloc(bufLen,    sizeof(float));
	audat.phi_ping     = (float *)  calloc(bufLen,    sizeof(float));
	audat.phi_pong     = (float *)  calloc(bufLen,    sizeof(float));
	audat.delta_t_ping = (float *)  calloc(bufLen,    sizeof(float));
	audat.delta_t_pong = (float *)  calloc(bufLen,    sizeof(float));
	audat.mag_ping     = (float *)  calloc(bufLen,    sizeof(float));
	audat.mag_pong     = (float *)  calloc(bufLen,    sizeof(float));
	audat.out_audio    = (float *)  calloc(numSamp,   sizeof(float));
	return audat;
}

buffer_data_t alloc_buffer_data(uint32_t bufLen)
{
	buffer_data_t bf;
	bf.phi_a           = (float *)       calloc(bufLen, sizeof(float));
	bf.phi_aPrev       = (float *)       calloc(bufLen, sizeof(float));
	bf.delta_f         = (float *)       calloc(bufLen, sizeof(float));
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

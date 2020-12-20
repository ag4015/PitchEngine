
#include "main.h"
#include <string.h>

int main()
{
	// Variable declaration
	unsigned long sizeData;                                 //Size of audio data in bytes
	FILE* outfile;
	const char* filePath = "/mnt/c/Users/alexg/Google Drive/Projects/Denoiser/clean_guitar.wav";

	#if DISTORTION
	coeffs = load_distortion_coefficients(&coeff_size);
  PRINT_LOG2("Number of coefficients: %ld\n", coeff_size);
	#endif

	// Pitch variables
	steps = 12;
	shift = pow(2, steps/12);
	hopA = (int)round(BUFLEN/NUMFRAMES);
	hopS = (int)round(hopA * shift);
	cleanIdx = hopS*4;

	PRINT_LOG2("Input file: %s\n\n", filePath);
	audio16 = readWav(&sizeData, filePath);                  // Get input audio from wav file and fetch the size
	NUM_SAMP = sizeData/sizeof(*audio16);                    // Number of samples

	// Allocate and zero fill arrays 
	inbuffer  = (float *) calloc(BUFLEN,   sizeof(float));	 // Input array 
	outbuffer = (float *) calloc(BUFLEN,   sizeof(float));   // Output array 
	for (uint8_t f = 0; f < NUMFRAMES; f++)
	{
		inframe[f]  = (float *) calloc(hopA,     sizeof(float));
		outframe[f] = (float *) calloc(hopS,     sizeof(float));
	}
	vTime     = (float *) calloc(2*hopS*NUMFRAMES,sizeof(float)); // Overlap-add signal. Times two because of ping-pong.
	inwin     = (float *) calloc(BUFLEN,          sizeof(float));   // Input window
	outwin    = (float *) calloc(BUFLEN,          sizeof(float));   // Output window
	phase     = (float *) calloc(BUFLEN,          sizeof(float));   // Phase of current frame
	phi_s     = (float *) calloc(BUFLEN,          sizeof(float));   // Phase adjusted for synthesis stage
	mag       = (float *) calloc(BUFLEN,          sizeof(float));   // Magnitude of current frame
	cpx       = (complex*)calloc(BUFLEN,          sizeof(complex)); // Complex variable for FFT 
	in_audio  = (float *) calloc(NUM_SAMP,        sizeof(float));   // Total input audio
	out_audio = (float *) calloc(NUM_SAMP,        sizeof(float));   // Total output audio

	previousPhase     = (float *) calloc(BUFLEN,          sizeof(float));   // Magnitude of current frame
	deltaPhi          = (float *) calloc(BUFLEN,          sizeof(float));   // Magnitude of current frame
	deltaPhiPrime     = (float *) calloc(BUFLEN,          sizeof(float));   // Magnitude of current frame
	deltaPhiPrimeMod  = (float *) calloc(BUFLEN,          sizeof(float));   // Magnitude of current frame
	trueFreq          = (float *) calloc(BUFLEN,          sizeof(float));   // Magnitude of current frame
	phaseCumulative   = (float *) calloc(BUFLEN,          sizeof(float));   // Magnitude of current frame

	
	// Initialize input and output window functions
	for(int k = 0; k < BUFLEN; k++){
		inwin[k]   =  HAMCONST * (1 - cos(2 * PI * k/BUFLEN))/sqrt((BUFLEN/hopA)/2);
		outwin[k]  =  HAMCONST * (1 - cos(2 * PI * k/BUFLEN))/sqrt((BUFLEN/hopS)/2);
	}
	
	// Clear freq.csv file of any content
	outfile = fopen("freq.csv", "w");
	fclose(outfile);

	// Convert 16bit audio to floating point
	for (unsigned long i = 0; i < NUM_SAMP; i++){
		in_audio[i] = ((float)audio16[i])/MAXVAL16;
	}

	// The first buffer isn't processed but it is stored
	for (uint8_t f = 0; f < NUMFRAMES; f++)
	{
		for (int k = 0; k < hopA; k++)
		{
			inframe[f][k] = in_audio[audio_ptr + k + f*hopA];
		}
	}
	audio_ptr += BUFLEN;
	while(audio_ptr < (NUM_SAMP - BUFLEN))
	{
		for (int k = 0; k < BUFLEN; k++)
		{
			inbuffer[k] = in_audio[audio_ptr + k];
		}
		process_buffer();
		for (int k = 0; k < BUFLEN; k++)
		{
			out_audio[audio_ptr + k] = outbuffer[k];
		}
		audio_ptr += BUFLEN;
	}

	PRINT_LOG2("It took an average time of %f ms to process each frame.\n", 1000*avg_time/CLOCKS_PER_SEC);

	// Reconvert floating point audio to 16bit
	for (unsigned long i = 0; i < NUM_SAMP; i++)
	{
		audio16[i] = (uint16_t)(out_audio[i]*MAXVAL16);
	}

	// Save the processed audio to output.wav
	writeWav(audio16, filePath);

	// Deallocate allocated memory
	free(audio16);
	free(inbuffer);
	free(outbuffer);
	for (uint8_t f = 0; f < NUMFRAMES; f++)
	{
		free(inframe[f]);
		free(outframe[f]);
	}
	free(vTime);
	free(inwin);
	free(outwin);
	free(phase);
	free(phi_s);
	free(mag);
	free(cpx);
	free(in_audio);
	free(out_audio);
	free(coeffs);

	free(previousPhase);
	free(deltaPhi);  
	free(deltaPhiPrime);
	free(deltaPhiPrimeMod);
	free(trueFreq);
	free(phaseCumulative);

	return 0;

}

void process_buffer()
{
	/* Variable declaration */
	int k;
	elapsed_time = clock();

/************ ANALYSIS STAGE ***********************/

	for (uint8_t f = 0; f < NUMFRAMES; f++)
	{
		for (int k = 0; k < hopA; k++)
		{
			inframe[frameNum][k] = inbuffer[frameNum*hopA + k];
		}
		int frameNum2 = frameNum + 1;
		if (frameNum2 >= NUMFRAMES) frameNum2 = 0;
		for (uint8_t f2 = 0; f2 < NUMFRAMES; f2++)
		{
			for (int k = 0; k < hopA; k++)
			{
				cpx[k + f2*hopA] = inframe[frameNum2][k]/* * inwin[k]*/;
			}
			if (++frameNum2 >= NUMFRAMES) frameNum2 = 0;
		}
		COPY(cpx[k], cpx[k] * inwin[k], BUFLEN);

/************ PROCESSING STAGE ***********************/

		#if FFT
		// Compute the FFT of the frame
		fft(cpx, BUFLEN, 0);

		process_frame();

		// Reconstruct the time domain signal using the IFFT 
		fft(cpx, BUFLEN, 1);

		COPY(cpx[k], creal(cpx[k]) * outwin[k], BUFLEN);
		#endif
		#ifdef PDEBUG
		/* if (audio_ptr == 1024) */
		/* { */
		/* 	COPY(mag[k], creal(cpx[k]), BUFLEN); */
		/* 	dumpFloatArray(inbuffer, BUFLEN, "debugData/inbuffer.csv"); */
		/* 	dumpFloatArray(mag, BUFLEN, "debugData/cpx.csv"); */
		/* 	dumpFloatArray(inwin           , BUFLEN, "debugData/inwin.csv"); */
		/* 	dumpFloatArray(outwin           , BUFLEN, "debugData/outwin.csv"); */
		/* 	dumpFloatArray(previousPhase   , BUFLEN, "debugData/previousPhase.csv"); */
		/* 	dumpFloatArray(phase           , BUFLEN, "debugData/phase.csv"); */
		/* 	dumpFloatArray(deltaPhi        , BUFLEN, "debugData/deltaPhi.csv"); */
		/* 	dumpFloatArray(deltaPhiPrime   , BUFLEN, "debugData/deltaPhiPrime.csv"); */
		/* 	dumpFloatArray(deltaPhiPrimeMod, BUFLEN, "debugData/deltaPhiPrimeMod.csv"); */
		/* 	dumpFloatArray(trueFreq        , BUFLEN, "debugData/trueFreq.csv"); */
		/* 	dumpFloatArray(phaseCumulative , BUFLEN, "debugData/phaseCumulative.csv"); */
		/* } */
		#endif

/************ SYNTHESIS STAGE ***********************/

		PRINT_LOG1("*******************\n");
		PRINT_LOG2("vTimeIdx: %i\n", vTimeIdx);
		PRINT_LOG2("cleanIdx from %i", cleanIdx);
		/* COPY(cpx[k], 0.25, BUFLEN); */
		for (k = 0; k < hopS; k++)
		{
			vTime[cleanIdx] = 0;
			if (++cleanIdx >= NUMFRAMES*hopS*2) cleanIdx = 0;
		}
		PRINT_LOG2(" to %i\n", cleanIdx-1);

		// The indexing variable for vTime has to be circular.
		int t = vTimeIdx + hopS*f;
		if (t >= NUMFRAMES*hopS*2) t = 0;

		PRINT_LOG2("t from %i ", t);
		for (k = 0; k < BUFLEN; k++)
		{
			vTime[t] += creal(cpx[k]);
			if ((++t) >= (NUMFRAMES*hopS*2)) t = 0;
		}
		PRINT_LOG2("to %i\n", t);
		PRINT_LOG2("k from 0 to %i\n", k);
		#ifdef PDEBUG
		char* fileName = (char*) malloc(sizeof(char)*50);
		strcpy(fileName, "debugData/vTimeXXX.csv");
		if (count < 20)
		{
			fileName[15] = (int)(count/100) + 48;
			fileName[16] = (int)(count/10) + 48;
			fileName[17] = (int)(count % 10) + 48;
			count++;
			dumpFloatArray(vTime,NUMFRAMES*hopS*2, fileName);
		}
		free(fileName);
		#endif

/********************************************************/

		if ((++frameNum) >= NUMFRAMES) frameNum = 0;

	}
	if (steps == 12)
	{
		for (k = 0; k < BUFLEN; k++)
		{
			outbuffer[k] = vTime[vTimeIdx + k * 2] * GAIN;
		}
	}

/************ LINEAR INTERPOLATION ***********************/
	else
	{
		float tShift;
		float upper;
		float lower;
		/* PRINT_LOG2("k from %i", vTimeIdx); */
		for (k = vTimeIdx; k < vTimeIdx + BUFLEN; k++)
		{
			tShift = (k - vTimeIdx) * shift;
			lower = vTime[(int)(tShift + vTimeIdx)];
			if ((int)(round(tShift + vTimeIdx + 0.499999)) > NUMFRAMES*hopS*2)
			{
				PRINT_LOG2("index: %i\n", (int)(round(tShift + vTimeIdx + 0.499999)));
			}
			upper = vTime[(int)(round(tShift + vTimeIdx + 0.499999))];
			outbuffer[k - vTimeIdx] = lower * (1 - (tShift - (int)tShift)) + upper * (tShift - (int)tShift);
			outbuffer[k - vTimeIdx] *= GAIN;
		}
		if (vTimeIdx == NUMFRAMES * hopS)
		{
			outbuffer[k - vTimeIdx - 1] = lower + (lower - vTime[(int)(tShift + vTimeIdx - 1)]);
			outbuffer[k - vTimeIdx - 1] *= GAIN;
		}
		/* PRINT_LOG2(" to %i\n", k); */
		/* PRINT_LOG2("tShfit: %f\n", tShift); */
		/* PRINT_LOG2("lower: %f\n", lower); */
		/* PRINT_LOG2("upper: %f\n", upper); */

		PRINT_LOG1("*******************\n");
		PRINT_LOG1("Buffer is out\n");
		PRINT_LOG1("*********************************\n");
	}

	/* for (k = 0; k < BUFLEN; k++) */
	/* { */
	/* 	if (k % hopA == 0) */
	/* 	{ */
	/* 		outbuffer[k] = 0.9; */
	/* 	} */
	/* 	if (k % hopS == 0) */
	/* 	{ */
	/* 		outbuffer[k] = -0.9; */
	/* 	} */
	/* } */

	#if DISTORTION
	float distortedSample = 0;
	for (int n = coeff_size - 1; n > -1; n--)
	{
		distortedSample += pow(outbuffer[m], coeff_size - n - 1) * coeffs[n];
	}
	outbuffer[m] = distortedSample;
	#endif

	elapsed_time = clock() - elapsed_time;
	avg_time = avg_time + (elapsed_time - avg_time)/N;
	N++;
	vTimeIdx += NUMFRAMES * hopS;
	if ((vTimeIdx) >= NUMFRAMES*hopS*2) vTimeIdx = 0;
	char* fileName2 = (char*) malloc(sizeof(char)*50);
	strcpy(fileName2, "debugData/outXX.csv");
	if (count2 < 10)
	{
		fileName2[13] = (int)(count2/10) + 48;
		fileName2[14] = (int)(count2 % 10) + 48;
		dumpFloatArray(outbuffer,BUFLEN, fileName2);
		count2++;
	}
	for (k = 0; k < BUFLEN; k++)
	{
		outbuffer[k] = vTime[vTimeIdx + k * 2] * GAIN;
		/* outbuffer[k] = creal(cpx[k]) * GAIN; */
	}

/************ LINEAR INTERPOLATION ***********************/

	/* PRINT_LOG2("k from %i", vTimeIdx); */
	/* outbuffer[0] = vTime[vTimeIdx]; */
	/* float tShift; */
	/* float upper; */
	/* float lower; */
	/* for (k = vTimeIdx + 1; k < vTimeIdx + BUFLEN - 1; k++) */
	/* { */
	/* 	tShift = k * shift; */
	/* 	lower = vTime[(int)(tShift)]; */
	/* 	upper = vTime[(int)(tShift) + 1]; */
	/* 	outbuffer[k - vTimeIdx] = lower * (1 - (tShift - (int)tShift)) + upper * (tShift - (int)tShift); */
	/* 	outbuffer[k - vTimeIdx] *= GAIN; */
	/* } */
	/* outbuffer[BUFLEN - 1] = vTime[vTimeIdx + BUFLEN - 1]; */
	/* PRINT_LOG2(" to %i\n", k); */

	PRINT_LOG1("*******************\n");
	PRINT_LOG1("Buffer is out\n");
	PRINT_LOG1("*********************************\n");

	/* for (k = 0; k < BUFLEN; k++) */
	/* { */
	/* 	if (k % hopA == 0) */
	/* 	{ */
	/* 		outbuffer[k] = 0.9; */
	/* 	} */
	/* 	if (k % hopS == 0) */
	/* 	{ */
	/* 		outbuffer[k] = -0.9; */
	/* 	} */
	/* } */

	#if DISTORTION
	float distortedSample = 0;
	for (int n = coeff_size - 1; n > -1; n--)
	{
		distortedSample += pow(outbuffer[m], coeff_size - n - 1) * coeffs[n];
	}
	outbuffer[m] = distortedSample;
	#endif

	elapsed_time = clock() - elapsed_time;
	avg_time = avg_time + (elapsed_time - avg_time)/N;
	N++;
	vTimeIdx += NUMFRAMES * hopS;
	if ((vTimeIdx) >= NUMFRAMES*hopS*2) vTimeIdx = 0;

}

float* load_distortion_coefficients(size_t* coeff_size)
{
	FILE* inFile = fopen("polyCoeff.csv", "r");
	float coeff;
	*coeff_size = 0;
	size_t n = 0;

	if(inFile == NULL)
	{
        PRINT_LOG1("Error opening file\n");
        exit(1);
	}
	while( fscanf(inFile, "%f\n", &coeff) != EOF )
	coeffs = (float *) calloc(*coeff_size, sizeof(float));
  PRINT_LOG1("Coefficients:\n");

	while( fscanf(inFile, "%f\n", &coeff) != EOF )
	{
		coeffs[n] = coeff;
		PRINT_LOG2("%f\n", coeffs[n]);
		n++;
	}
	fclose(inFile);

	return coeffs;

}

void dumpFloatArray(float buf[], size_t size, const char* name) {

		// Open file to append fft data
		FILE *outfile;
		outfile = fopen(name, "w");

		// Store in a format understandable by numpy in python
		for (int i = 0; i < size; i++){
			fprintf(outfile, "%f;", buf[i]);	
		}

		fprintf(outfile, "\n");
		fclose(outfile);
}

/* void process_frame() */
/* { */
/* 	float current_phase; */
/* 	float		deltaPhi; */
/* 	float		deltaPhiPrime; */
/* 	float		deltaPhiPrimeMod; */
/* 	float   trueFreq; */
/*  */
/* 	for(uint16_t k = 0; k < BUFLEN; k++) */
/* 	{ */
/* 		mag[k] = cabs(cpx[k]); */
/*  */
/* 		current_phase = carg(cpx[k]); */
/* 		deltaPhi = current_phase - phase[k]; */
/* 		phase[k] = current_phase; */
/*  */
/* 		deltaPhiPrime = deltaPhi - (hopA * 2 * PI * k)/BUFLEN; */
/*  */
/* 		deltaPhiPrimeMod = fmod(deltaPhiPrime + PI , 2 * PI) - PI; */
/*  */
/* 		trueFreq = (2 * PI * k)/BUFLEN + deltaPhiPrimeMod/hopA; */
/*  */
/* 		phaseCumulative = phaseCumulative + hopS * trueFreq; */
/*  */
/* 		cpx[k] = mag[k]*cexp(I*phaseCumulative); */
/* 	}	 */
/* } */

void process_frame()
{
		COPY(mag[k], cabs(cpx[k]), BUFLEN);
		COPY(phase[k], carg(cpx[k]), BUFLEN);
		COPY(deltaPhi[k], phase[k] - previousPhase[k], BUFLEN);
		COPY(previousPhase[k], phase[k], BUFLEN);
		COPY(deltaPhiPrime[k], deltaPhi[k] - (hopA * 2 * PI * k)/BUFLEN, BUFLEN);
		COPY(deltaPhiPrimeMod[k], fmod(deltaPhiPrime[k] + PI , 2 * PI) - PI, BUFLEN);
		COPY(trueFreq[k], (2 * PI * k)/BUFLEN + deltaPhiPrimeMod[k]/hopA, BUFLEN);
		COPY(phaseCumulative[k], phaseCumulative[k] + hopS * trueFreq[k], BUFLEN);
		COPY(cpx[k], mag[k]*cexp(I*phaseCumulative[k]), BUFLEN);
}

/* void overlapAdd(float in[NUMFRAMES][], float out[], int bufLen, int hop) */
/* { */
/* } */



#ifndef FFT_H
#define FFT_H

#include <complex.h>
#include <stdbool.h>

#define PI 3.141592653589793
#define FSAMP 44100
#define FFTLEN 1024

typedef float complex cplx;

void fft2(cplx buf[], cplx out[], int n, int step, int isign);
void fft(cplx buf[], int n, bool inverse);
void show(const char * s, cplx buf[]);

#endif // FFT_H

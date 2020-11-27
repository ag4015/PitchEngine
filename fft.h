
#ifndef FFT_H
#define FFT_H

#include <complex.h>
#include <stdbool.h>

#define PI (float)3.141592653589793
#define FSAMP  44100
#define BUFLEN 1024

// typedef float complex cplx;

void fft2(complex buf[], complex out[], int n, int step, int isign);
void fft(complex buf[], int n, bool inverse);
void show(const char * s, complex buf[]);

#endif // FFT_H

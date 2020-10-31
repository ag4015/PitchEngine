
#include "fft.h"
#include <stdio.h>
#include <math.h>

void fft2(cplx buf[], cplx out[], int n, int step, int isign)
{
    if (step < n) {
        fft2(out, buf, n, step * 2, isign);
        fft2(out + step, buf + step, n, step * 2, isign);

        for (unsigned long i = 0; i < n; i += 2 * step) {
            cplx t = cexp(isign*I * PI * i / n) * out[i + step];
            buf[i / 2]     = out[i] + t;
            buf[(i + n)/2] = out[i] - t;
        }
    }
}

void fft(cplx buf[], int n, bool inverse)
{
    cplx out[n];
		int isign = -1;

		if (inverse){
			isign = 1;
		}

    fft2(buf, out, n, 1, isign);
		if (inverse){
    	for (unsigned long i = 0; i < n; i++) buf[i] = buf[i]/FFTLEN;
		}
}

void show(const char * s, cplx buf[]) {

		// Open file to append fft data
		FILE *outfile;
		outfile = fopen("freq.csv", "a");

		// Store in a format understandable by numpy in python
		for (int i = 0; i < FFTLEN; i++){
			if (cimagf(buf[i]) < 0){
				fprintf(outfile, "(%f%fj);", crealf(buf[i]), cimagf(buf[i]));	
			}
			else{
				fprintf(outfile, "(%f+%fj);", crealf(buf[i]), cimagf(buf[i]));	
			}
		}

		fprintf(outfile, "\n");
		fclose(outfile);
}

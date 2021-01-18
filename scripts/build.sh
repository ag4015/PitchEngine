# gcc -Wall -pedantic -std=c11 main.c kissfft/kiss_fft.c heap.c audioUtils.c -o main -lm -lgsl -lgslcblas -g
g++ -Wall -pedantic -std=c++17 main.c wavio.c kissfft/kiss_fft.c audioUtils.cpp -o build/main -lm -lgsl -lgslcblas -g

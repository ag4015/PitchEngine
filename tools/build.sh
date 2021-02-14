cd src; g++ -Wall -pedantic -std=c++17 main.c wavio.c kissfft/kiss_fft.c audioUtils.cpp -o ../build/main -lm -lgsl -lgslcblas -g

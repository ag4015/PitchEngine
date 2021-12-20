#ifndef DSP_CONFIG_H
#define DSP_CONFIG_H

#define DATA_DIR "C:/Users/alexg/source/repos/DSPSim/data//"
#define INPUT_AUDIO_DIR "C:/Users/alexg/source/repos/DSPSim/data/inputAudio//"
#define OUTPUT_AUDIO_DIR "C:/Users/alexg/source/repos/DSPSim/data/outputAudio//"
#define DEBUG_DIR "C:/Users/alexg/source/repos/DSPSim/data/debugData//"

#ifdef USE_DOUBLE
typedef double my_float;
#else
typedef float my_float;
#endif

#endif // DSP_CONFIG_H

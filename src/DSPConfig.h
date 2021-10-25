#ifndef DSP_CONFIG_H
#define DSP_CONFIG_H

/* #undef DEBUG_DUMP */
/* #undef DEBUG_LOG */
/* #undef SIMPLE_PV */
/* #undef CONSTANT_Q_T */
#define USE_DOUBLE
#define USE_MULTITHREADING
#define DEBUG_TIMING

#define RESOURCES_DIR "C:/Users/alexg/source/repos/DSPSim/resources/"
#define INPUT_AUDIO_DIR RESOURCES_DIR "inputAudio/"
#define OUTPUT_AUDIO_DIR RESOURCES_DIR "outputAudio/"
#define DEBUG_DIR RESOURCES_DIR "debugData/"

#ifdef USE_DOUBLE
typedef double my_float;
#else
typedef float my_float;
#endif

#endif // DSP_CONFIG_H

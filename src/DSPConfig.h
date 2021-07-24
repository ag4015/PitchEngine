#ifndef DSP_CONFIG_H
#define DSP_CONFIG_H

/* #undef DEBUG_DUMP */
#define DEBUG_LOG
/* #undef SIMPLE_PV */
/* #undef CONSTANT_Q_T */
/* #undef USE_DOUBLE */

// the configured options and settings for Tutorial
#define RESOURCES_DIR "/mnt/c/Users/alexg/source/repos/DSPSim/resources/"

#ifdef USE_DOUBLE
typedef double my_float;
#else
typedef float my_float;
#endif

#endif // DSP_CONFIG_H

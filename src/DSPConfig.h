#ifndef DSP_CONFIG_H
#define DSP_CONFIG_H
// the configured options and settings for Tutorial
#define RESOURCES_DIR "C:/Users/alexg/source/repos/DSPSim/resources/"

#ifdef USE_DOUBLE
typedef double my_float;
#else
typedef float my_float;
#endif

/* #undef DEBUG_DUMP */
/* #undef SIMPLE_PV */
/* #undef DEBUG_DUMP */
#define CONSTANT_Q_T

#endif // DSP_CONFIG_H

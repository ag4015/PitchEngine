#ifndef DSP_CONFIG_H
#define DSP_CONFIG_H

#define DATA_DIR "C:/Users/eeiglal/source/repos/PitchEngine/data//"
#define INPUT_AUDIO_DIR "C:/Users/eeiglal/source/repos/PitchEngine/data/inputAudio//"
#define OUTPUT_AUDIO_DIR "C:/Users/eeiglal/source/repos/PitchEngine/data/outputAudio//"
#define TEST_AUDIO_DIR "C:/Users/eeiglal/source/repos/PitchEngine/data/testAudio//"
#define DEBUG_DIR "C:/Users/eeiglal/source/repos/PitchEngine/data/debugData//"

#ifdef USE_DOUBLE
typedef double my_float;
#else
typedef float my_float;
#endif

#endif // DSP_CONFIG_H

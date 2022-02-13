#pragma once

#include <string>
#include <vector>

#ifdef USE_DOUBLE
typedef double my_float;
#else
typedef float my_float;
#endif

std::vector<float> readWav(std::string& filePath);
void    writeWav(std::vector<float>& audio, std::string& inputFilePath, std::string& outputFilePath);



#include "logger.h"
#include <fstream>

template<typename T>
void dump_array(T* buf, uint32_t size, std::string&& fileName, int count, int max, int auP, int auPMax)
{
	static_assert(std::is_floating_point<T>::value || isComplex<T>::value);

	if ((count > max && max != -1) ||
		(auP != auPMax && auPMax != -1) ||
		fileName.empty()) { return; }

	// Open file to append data. Clear if it's the first entry
	std::ofstream outFile(fileName, !count ? std::ios::out : std::ios::out | std::ios::app);
	
	// Store in a format understandable by numpy in python
	for (size_t i = 0; i < size; i++) {
		if constexpr (std::is_floating_point<T>::value) {
			outFile << buf[i] << ";";
		}
		else {
			outFile << buf[i].r << ";";
		}
	}
	outFile << std::endl;
	outFile.close();
}

void template_float_dump_array(my_float* buf, size_t size, const char* fileName, int count, int max, int auP, int auPMax)
{
	dump_array(buf, size, fileName, count, max, auP, auPMax);
}


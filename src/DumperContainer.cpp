
#include "DumperContainer.h"

void DumperContainer::createDumper(const std::string& name, uint32_t& audio_ptr,
	uint32_t bufferSize, uint32_t dumpSize, uint32_t countMax, uint32_t auPMax)
{
	std::string fileName = path_ + name;
	Dumper dumper(path_ + name, audio_ptr, dumpSize, bufferSize,countMax, auPMax);
	dumperMap_[fileName] = std::move(std::make_unique<Dumper>(Dumper(path_ + name, audio_ptr, bufferSize, dumpSize, countMax, auPMax)));
}


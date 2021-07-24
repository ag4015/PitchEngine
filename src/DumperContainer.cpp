
#include "DumperContainer.h"

void DumperContainer::createDumper(std::string& name, uint32_t& audio_ptr,
	uint32_t bufferSize, uint32_t dumpSize, uint32_t countMax, uint32_t auPMax)
{
	std::string fileName = path_ + name;
	Dumper dumper(path_ + name, audio_ptr, bufferSize, dumpSize, countMax, auPMax);
	//dumperMap_.emplace(std::make_pair(path_ + name, Dumper(path_ + name, audio_ptr, bufferSize, dumpSize, countMax, auPMax)));
	//dumperMap_.insert(std::make_pair(path_ + name, Dumper(path_ + name, audio_ptr, bufferSize, dumpSize, countMax, auPMax)));
	//dumperMap_.insert(path_ + name, Dumper(path_ + name, audio_ptr, bufferSize, dumpSize, countMax, auPMax));
	//dumperMap_.emplace(path_ + name, Dumper(path_ + name, audio_ptr, bufferSize, dumpSize, countMax, auPMax));
	//dumperMap_.emplace(std::make_pair(path_ + name, Dumper(path_ + name, audio_ptr, bufferSize, dumpSize, countMax, auPMax)));
	//dumperMap_.emplace(fileName, Dumper(path_ + name, audio_ptr, bufferSize, dumpSize, countMax, auPMax));
	//dumperMap_.emplace(fileName, dumper);
	//dumperMap_.insert(fileName, dumper);
	//dumperMap_[fileName] = Dumper(path_ + name, audio_ptr, bufferSize, dumpSize, countMax, auPMax);
	dumperMap_[fileName] = std::move(std::make_unique<Dumper>(Dumper(path_ + name, audio_ptr, bufferSize, dumpSize, countMax, auPMax)));
}

template<typename T>
void DumperContainer::dump(T* buf, std::string& name)
{
	auto dumper = dumperMap_.find(path_ + name);
	if (dumper != dumperMap_.end()) {
		dumper->second->dump(buf);
	}
}

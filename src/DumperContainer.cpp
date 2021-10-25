#include "DumperContainer.h"
#include <filesystem>
#include <mutex>

std::mutex dumperContainerMutex;

DumperContainer* DumperContainer::getDumperContainer(const std::string& path)
{
	std::lock_guard<std::mutex> dumperLock(dumperContainerMutex);
	if (!instance) {
		instance = new DumperContainer{ path };
	}
	return instance;
}

// Dumper for variables
void DumperContainer::createDumper(const std::string& variationName, const std::string& name, uint32_t& audio_ptr,
	uint32_t bufferSize, uint32_t dumpSize, uint32_t countMax, uint32_t auPMax)
{
	std::string folderDir = path_ + variationName + "/";
	std::filesystem::create_directory(folderDir);
	std::string fileName = folderDir + name;
	dumperMap_[fileName] = std::move(std::make_unique<Dumper>(Dumper(fileName, &audio_ptr, bufferSize, dumpSize, countMax, auPMax)));
}

// Dumper for timings
void DumperContainer::createDumper(const std::string& variationName, const std::string& name)
{
	std::string folderDir = path_ + variationName + "/";
	std::filesystem::create_directory(folderDir);
	std::string fileName = folderDir + name;
	dumperMap_[fileName] = std::move(std::make_unique<Dumper>(Dumper(fileName)));
}


#include "DumperContainer.h"
#include <filesystem>
#include <mutex>

std::mutex dumperContainerMutex;

DumperContainer::DumperContainer(std::string path)
{
	updatePath(path);
}

std::string& DumperContainer::getPath()
{
	return pathMap_[std::this_thread::get_id()];
}

DumperContainer* DumperContainer::getDumperContainer(const std::string& path)
{
	std::lock_guard<std::mutex> dumperLock(dumperContainerMutex);
	if (!instance) {
		instance = new DumperContainer{ path };
	}
	return instance;
}

void DumperContainer::updatePath(const std::string& path)
{
	pathMap_[std::this_thread::get_id()] = path;
}

// Dumper for variables
void DumperContainer::createDumper(const std::string& name, uint32_t& audio_ptr,
	uint32_t bufferSize, uint32_t dumpSize, uint32_t countMax, uint32_t auPMax)
{
	std::string& folderDir = getPath();
	std::filesystem::create_directory(folderDir);
	std::string fileName = folderDir + name;
	dumperMap_[fileName] = std::move(std::make_unique<Dumper>(Dumper(fileName, &audio_ptr, bufferSize, dumpSize, countMax, auPMax)));
}

// Dumper for timings
void DumperContainer::createDumper(const std::string& name)
{
	std::string& folderDir = getPath();
	std::filesystem::create_directory(folderDir);
	std::string fileName = folderDir + name;
	dumperMap_[fileName] = std::move(std::make_unique<Dumper>(Dumper(fileName)));
}


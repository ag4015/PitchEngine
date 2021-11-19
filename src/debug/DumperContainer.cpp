#include "DumperContainer.h"
#include <filesystem>
#include <mutex>
#include <chrono>
#include <thread>

std::mutex dumperContainerMutex;
//std::mutex createDumperMutex;

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
	//std::lock_guard<std::mutex> createDumper(createDumperMutex);
	std::string& folderDir = getPath();
	std::string folderDir2 = folderDir;
	//std::filesystem::remove_all(folderDir);
	//std::this_thread::sleep_for(std::chrono::seconds(5));
	std::filesystem::create_directory(folderDir2);
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


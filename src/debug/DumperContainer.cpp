#include "DumperContainer.h"
#ifdef WIN32
#include <filesystem>
#else
#include <experimental/filesystem>
#endif
#include <mutex>

std::mutex dumperContainerMutex;
std::mutex createDumperContainerMutex;
std::mutex getPathMutex;

DumperContainer::DumperContainer(std::string path)
{
	updatePath(path);
}

std::string& DumperContainer::getPath()
{
	std::lock_guard<std::mutex> getPathLock(getPathMutex);
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
void DumperContainer::createDumper(const std::string& name, int& audio_ptr,
	int bufferSize, int dumpSize, int countMax, int auPMax)
{
	std::string& folderDir = getPath();
	std::string folderDir2 = folderDir;
#ifdef WIN32
	std::filesystem::create_directory(folderDir2);
#else
	std::experimental::filesystem::create_directory(folderDir2);
#endif
	std::string fileName = folderDir + name;
	dumperMap_[fileName] = std::move(std::make_unique<Dumper>(Dumper(fileName, &audio_ptr, bufferSize, dumpSize, countMax, auPMax)));
}

// Dumper for timings
void DumperContainer::createDumper(const std::string& name)
{
	std::lock_guard<std::mutex> createDumperLock(createDumperContainerMutex);
	std::string& folderDir = getPath();
#ifdef WIN32
	std::filesystem::create_directory(folderDir);
#else
	std::experimental::filesystem::create_directory(folderDir);
#endif
	std::string fileName = folderDir + name;
	dumperMap_[fileName] = std::move(std::make_unique<Dumper>(Dumper(fileName)));
}


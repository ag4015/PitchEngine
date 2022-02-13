
#include "ProgressBarContainer.h"
#include <mutex>

std::mutex getProgressBarContainerMutex;
std::mutex getProgressBarMapMutex;
std::mutex createProgressBarMapMutex;
std::mutex printProgressBarMutex;
std::mutex allFinishedProgressBarMutex;
std::mutex getProgressBarNameVecMutex;

ProgressBarContainer* ProgressBarContainer::getProgressBarContainer()
{
	std::lock_guard<std::mutex> getProgressBarContainerLock(getProgressBarContainerMutex);
	if (!instance) {
		instance = new ProgressBarContainer();
	}
	return instance;
}

void ProgressBarContainer::createProgressBar(std::string& name, int numCycles)
{
	std::lock_guard<std::mutex> createProgressBarContainerLock(createProgressBarMapMutex);
	getProgressBarMap()[name] = std::move(std::make_unique<ProgressBar>(name,numCycles));
	getProgressBarNameVec().push_back(name);
}

progressBarMap_t& ProgressBarContainer::getProgressBarMap()
{
	std::lock_guard<std::mutex> getProgressBarLock(getProgressBarMapMutex);
	return progressBarMap_;
}

std::vector<std::string>& ProgressBarContainer::getProgressBarNameVec()
{
	std::lock_guard<std::mutex> getProgressBarNameVecLock(getProgressBarNameVecMutex);
	return progressBarNameVec_;
}

void ProgressBarContainer::progress(std::string& name)
{
	getProgressBarMap()[name]->progress();
}

bool ProgressBarContainer::allFinished()
{
	std::lock_guard<std::mutex> allFinishedProgressBar(allFinishedProgressBarMutex);
	for (auto& progressBar : getProgressBarMap())
	{
		if (!progressBar.second.get()->printedAfterFinished_)
		{
			return false;
		}
	}
	return true;
}

void ProgressBarContainer::print()
{
	std::lock_guard<std::mutex> printProgressBarLock(printProgressBarMutex);
	if (getProgressBarMap().size() == 0)
	{
		return;
	}
	for (size_t i = 0; i < getProgressBarMap().size() - 1; i++)
	{
		std::cout << "\033[1A";
	}
	int i = 0;
	for (auto& progressBarIter : getProgressBarMap())
	{
		ProgressBar* progressBar = progressBarIter.second.get();
		if (progressBar->first_ && i != 0) {
			progressBar->first_ = false;
			std::cout << std::endl;
		}
		std::cout << "\r";
#ifndef USE_MULTITHREADING
		if (!progressBar->isFinished() || !progressBar->printedAfterFinished_)
		{
			if (progressBar->isFinished())
			{
				progressBar->printedAfterFinished_ = true;
			}
#endif
			std::cout << progressBar->getOutputStream().str();
#ifndef USE_MULTITHREADING
		}
#endif
		if (i < getProgressBarMap().size() - 1 && !getProgressBarMap()[getProgressBarNameVec()[i + 1]]->first_)
		{
			std::cout << "\033[1B";
		}
		i++;
	}
}

void ProgressBarContainer::finish(std::string& name)
{
	getProgressBarMap()[name]->finish();
}

void printProgress()
{
#ifdef USE_MULTITHREADING
	while (!ProgressBarContainer::getProgressBarContainer()->allFinished() ||
		ProgressBarContainer::getProgressBarContainer()->getProgressBarMap().size() == 0)
	{
		ProgressBarContainer::getProgressBarContainer()->print();
	}
#else
	ProgressBarContainer::getProgressBarContainer()->print();
#endif
}

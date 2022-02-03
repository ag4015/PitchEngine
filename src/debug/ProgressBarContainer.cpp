
#include "ProgressBarContainer.h"
#include <mutex>

std::mutex getProgressBarContainerMutex;
std::mutex getProgressBarMapMutex;

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
	progressBarMap_[name] = std::move(std::make_unique<ProgressBar>(name,numCycles));
	progressBarNameVec_.push_back(name);
}

progressBarMap_t& ProgressBarContainer::getProgressBarMap()
{
	std::lock_guard<std::mutex> getProgressBarLock(getProgressBarMapMutex);
	return progressBarMap_;
}

void ProgressBarContainer::progress(std::string& name)
{
	getProgressBarMap()[name]->progress();
}

bool ProgressBarContainer::allFinished()
{
	for (size_t i = 0; i < progressBarMap_.size(); i++)
	{
		if (!getProgressBarMap()[progressBarNameVec_[i]]->printedAfterFinished_)
		{
			return false;
		}
	}
	return true;
}

void ProgressBarContainer::print()
{
	for (size_t i = 0; i < getProgressBarMap().size() - 1; i++)
	{
		std::cout << "\033[1A";
	}
	for (size_t i = 0; i < progressBarMap_.size(); i++)
	{
		ProgressBar* progressBar = getProgressBarMap()[progressBarNameVec_[i]].get();
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
		if (i < getProgressBarMap().size() - 1 && !getProgressBarMap()[progressBarNameVec_[i + 1]]->first_)
		{
			std::cout << "\033[1B";
		}
	}
}

void ProgressBarContainer::finish(std::string& name)
{
	progressBarMap_[name]->finish();
}


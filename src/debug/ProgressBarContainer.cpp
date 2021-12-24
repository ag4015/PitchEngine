
#include "ProgressBarContainer.h"
#include <mutex>
#include <chrono>
#include <thread>

std::mutex getProgressBarContainerMutex;
std::mutex progressBarContainerMutex;
std::mutex getProgressBarMapMutex;
std::mutex progressMutex;
std::mutex finishMutex;

ProgressBar::ProgressBar(std::string& name, int numCycles)
	: name_(name)
	, bar_(numCycles, name)
	, finished_(false)
	, outStream_()
	, first_(true)
{
}

std::stringstream& ProgressBar::getOutputStream()
{
	return outStream_;
}

void ProgressBar::progress()
{
	std::lock_guard<std::mutex> progressMutexLock(progressMutex);
	outStream_.clear();
	bar_.update(outStream_);
}

void ProgressBar::finish()
{
	std::lock_guard<std::mutex> finishMutexLock(finishMutex);
	finished_ = true;
	progress();
}

bool ProgressBar::isFinished()
{
	return finished_;
}

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
	std::unique_lock<std::mutex> progressBarLock(progressBarContainerMutex, std::try_to_lock);
	getProgressBarMap()[name]->progress();
	if (!progressBarLock.owns_lock()) {
		return;
	}

	//std::this_thread::sleep_for(std::chrono::milliseconds(30));
	for (size_t i = 0; i < getProgressBarMap().size() - 1; i++)
	{
		std::cout << "\033[1A";
	}
	for (size_t i = 0; i < progressBarMap_.size(); i++)
	{
		if (getProgressBarMap()[progressBarNameVec_[i]]->first_ && i != 0) {
			getProgressBarMap()[progressBarNameVec_[i]]->first_ = false;
			std::cout << std::endl;
		}
		std::cout << "\r";
		//if (!getProgressBarMap()[progressBarNameVec_[i]]->isFinished())
		//{
			std::cout << getProgressBarMap()[progressBarNameVec_[i]]->getOutputStream().str();
		//}
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

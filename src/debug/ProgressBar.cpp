
#include "ProgressBar.h"
#include <mutex>
#include <thread>

std::mutex progressBarContainerMutex;
std::mutex progressMutex;
std::mutex finishMutex;

ProgressBar::ProgressBar(std::string& name, int numCycles)
	: name_(name)
	, bar_(numCycles, name)
	, finished_(false)
	, outStream_()
	, first_(true)
	, printedAfterFinished_(false)
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


#include "TimerContainer.h"

TimerContainer* TimerContainer::getTimerContainer()
{
	if (!instance) {
		instance = new TimerContainer;
	}
	return instance;
}

void TimerContainer::updateTimer(std::string& name, uint64_t elapsedTimeUs, timeUnit unit)
{
	if (timerMap_.find(name) == timerMap_.end()) {
		timerMap_[name] = std::tuple<int, uint64_t, timeUnit>(0, 0, unit);
	}
	std::get<0>(timerMap_[name])++;
	std::get<1>(timerMap_[name]) += elapsedTimeUs;
}

void TimerContainer::dumpTimings(const std::string& variationName, std::string name)
{
	DumperContainer::getDumperContainer()->createDumper(variationName, name);
	DumperContainer::getDumperContainer()->dump(timerMap_, variationName + "/" + name);
}

#include "Timer.h"
#include "TimerContainer.h"


	Timer::Timer(std::string name, timeUnit unit)
		: name_(name), unit_(unit)
	{
		initTime_ = std::chrono::high_resolution_clock::now();
	}

	Timer::~Timer()
	{
		highResTime finalTime = std::chrono::high_resolution_clock::now();
		switch(unit_)
		{
			case timeUnit::MICROSECONDS:
			{
				auto exTime  = std::chrono::duration_cast<std::chrono::microseconds>(finalTime - initTime_);
				TimerContainer::getTimerContainer()->updateTimer(name_, exTime.count(), unit_);
				break;
			}
			case timeUnit::MILISECONDS:
			{
				auto exTime = std::chrono::duration_cast<std::chrono::milliseconds>(finalTime - initTime_);
				TimerContainer::getTimerContainer()->updateTimer(name_, exTime.count(), unit_);
				break;
			}
			case timeUnit::SECONDS:
			{
				auto exTime = std::chrono::duration_cast<std::chrono::seconds>(finalTime - initTime_);
				TimerContainer::getTimerContainer()->updateTimer(name_, exTime.count(), unit_);
				break;
			}
		}
	}


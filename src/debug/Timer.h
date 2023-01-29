#pragma once
#include <chrono>
#include <string>

#ifdef DEBUG_TIMING
       #define // CREATE_TIMER(a,b) Timer timer(a,b)
       #define // END_TIMER(a) timer.endMeasurement()
       #define // DUMP_TIMINGS(a) TimerContainer::getTimerContainer()->// DUMPTimings(a)
#else
       #define // CREATE_TIMER(a,b)
       #define // END_TIMER(a)
       #define // DUMP_TIMINGS(a)
#endif

enum class timeUnit
{
	MICROSECONDS,
	MILISECONDS,
	SECONDS
};

using highResTime = decltype(std::chrono::high_resolution_clock::now());

class Timer
{
	std::chrono::time_point< std::chrono::high_resolution_clock> initTime;
	std::string name_;
	int numIterations = 1;
	highResTime initTime_;
	timeUnit unit_;
public:
	Timer(std::string name, timeUnit unit);
	~Timer();
	void endMeasurement();
};


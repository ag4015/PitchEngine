#pragma once
#include <chrono>
#include <string>

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


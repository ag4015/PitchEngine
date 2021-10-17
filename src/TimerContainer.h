#pragma once
#include "DumperContainer.h"
#include "Timer.h"
#include <unordered_map>
#include <string>

class TimerContainer
{
private:
    static TimerContainer* instance;
    std::unordered_map<std::string, std::tuple<int, uint64_t, timeUnit> > timerMap_;

public:
    static TimerContainer* getTimerContainer();
    void updateTimer(std::string& name, uint64_t elapsedTimeUs, timeUnit unit);
    void dumpTimings(const std::string& variationName, std::string name);
};

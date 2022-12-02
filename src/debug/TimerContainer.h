#pragma once
#include "DumperContainer.h"

class TimerContainer
{
private:
    static TimerContainer* instance;
    std::unordered_map<std::string, std::tuple<int, uint64_t, timeUnit> > timerMap_;

public:
    static TimerContainer* getTimerContainer();
    void updateTimer(std::string& name, uint64_t elapsedTimeUs, timeUnit unit);
    void dumpTimings(std::string name);
};

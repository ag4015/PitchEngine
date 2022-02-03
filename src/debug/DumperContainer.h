#pragma once
#include "Dumper.h"
#include <unordered_map>
#include <memory>
#include <string>
#include <thread>

#if defined(DEBUG_DUMP) || defined(DEBUG_TIMING)
	   #define INITIALIZE_DUMPERS(a,b,c,d,e,f) initializeDumpers(a,b,c,d,e,f)
       #define CREATE_DUMPER_C0NTAINER(a) DumperContainer::getDumperContainer(a)
       #define UPDATE_DUMPER_CONTAINER_PATH(a) DumperContainer::getDumperContainer()->updatePath(a)
#else
	   #define INITIALIZE_DUMPERS(a,b,c,d,e,f)
       #define CREATE_DUMPER_C0NTAINER(a)
       #define UPDATE_DUMPER_CONTAINER_PATH(a)
#endif

#ifdef DEBUG_TIMING
       #define CREATE_TIMER(a,b) Timer timer(a,b);
       #define END_TIMER(a) timer.endMeasurement();
       #define DUMP_TIMINGS(a) TimerContainer::getTimerContainer()->dumpTimings(a);
#else
       #define CREATE_TIMER(a,b)
       #define END_TIMER(a)
       #define DUMP_TIMINGS(a)
#endif

#ifdef DEBUG_DUMP
       #define INIT_DUMPER(a,b,c,d,e,f,g) DumperContainer::getDumperContainer()->createDumper(a,b,c,d,e,f)
       #define DUMP_ARRAY(a,b) DumperContainer::getDumperContainer()->dump(a,b)
#else
       #define INIT_DUMPER(a,b,c,d,e,f)
       #define DUMP_ARRAY(a,b)
#endif

class DumperContainer
{
private:
    DumperContainer(std::string path);
    static DumperContainer* instance;
    std::unordered_map<std::string, std::unique_ptr<Dumper> > dumperMap_;
    std::string& getPath();

public:
    std::unordered_map<std::thread::id, std::string> pathMap_;
    static DumperContainer* getDumperContainer(const std::string& path = "");
    void updatePath(const std::string& path);
    void createDumper(const std::string& name, int& audio_ptr, int bufferSize,
        int dumpSize, int countMax, int auPMax);
    void createDumper(const std::string& name);
	template<typename T>
	void dump(T buf, const std::string& name)
	{
		auto dumper = dumperMap_.find(getPath() + name);
		if (dumper != dumperMap_.end()) {
			dumper->second->dump(buf);
		}
	}
};


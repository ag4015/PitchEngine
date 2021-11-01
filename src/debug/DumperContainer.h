#pragma once
#include "Dumper.h"
#include <unordered_map>
#include <memory>
#include <string>

#if defined(DEBUG_DUMP) || defined(DEBUG_TIMING)
	   #define INITIALIZE_DUMPERS(a,b,c,d) initializeDumpers(a,b,c,d)
       #define CREATE_DUMPER_C0NTAINER(a) DumperContainer::getDumperContainer(a)
       #define CREATE_TIMER(a,b) Timer timer(a,b);
       #define DUMP_TIMINGS(a,b) TimerContainer::getTimerContainer()->dumpTimings(a,b);
#else
	   #define INITIALIZE_DUMPERS(a,b,c,d)
       #define CREATE_DUMPER_C0NTAINER(a)
       #define CREATE_TIMER(a,b)
       #define DUMP_TIMINGS(a,b)
#endif

#ifdef DEBUG_DUMP
       #define INIT_DUMPER(a,b,c,d,e,f,g) DumperContainer::getDumperContainer()->createDumper(a,b,c,d,e,f,g)
       #define DUMP_ARRAY(a,b) DumperContainer::getDumperContainer()->dump(a,b)
#else
       #define INIT_DUMPER(a,b,c,d,e,f,g)
       #define DUMP_ARRAY(a,b)
#endif

class DumperContainer
{
private:
    DumperContainer(std::string path) : path_(path) {}
    static DumperContainer* instance;
    std::unordered_map<std::string, std::unique_ptr<Dumper> > dumperMap_;
    const std::string path_;

public:
    static DumperContainer* getDumperContainer(const std::string& path = "");
    void createDumper(const std::string& variationName, const std::string& name, uint32_t& audio_ptr, uint32_t bufferSize,
        uint32_t dumpSize, uint32_t countMax, uint32_t auPMax);
    void createDumper(const std::string& variationName, const std::string& name);
	template<typename T>
	void dump(T buf, const std::string& name)
	{
		auto dumper = dumperMap_.find(path_ + name);
		if (dumper != dumperMap_.end()) {
			dumper->second->dump(buf);
		}
	}
};


#pragma once
#include "DSPConfig.h"
#include "Dumper.h"
#include <unordered_map>
#include <memory>
#include <string>

#ifdef DEBUG_DUMP 
	   #define INITIALIZE_DUMPERS(a,b,c) initializeDumpers(a,b,c)
       #define CREATE_DUMPER_C0NTAINER(a) DumperContainer::getDumperContainer(a)
       #define INIT_DUMPER(a,b,c,d,e,f) DumperContainer::getDumperContainer()->createDumper(a,b,c,d,e,f)
       #define DUMP_ARRAY(a,b) DumperContainer::getDumperContainer()->dump(a,b)
#else
	   #define INITIALIZE_DUMPERS(a,b,c)
       #define CREATE_DUMPER_C0NTAINER(a)
       #define INIT_DUMPER(a,b,c,d,e,f)
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
    static DumperContainer* getDumperContainer(const std::string& path = "")
    {
        if (!instance) {
            instance = new DumperContainer{ path };
        }
		return instance;
    }
    void createDumper(const std::string& name, uint32_t& audio_ptr, uint32_t bufferSize,
        uint32_t dumpSize, uint32_t countMax, uint32_t auPMax);
	template<typename T>
	void dump(T buf, const std::string& name)
	{
		auto dumper = dumperMap_.find(path_ + name);
		if (dumper != dumperMap_.end()) {
			dumper->second->dump(buf);
		}
	}
};


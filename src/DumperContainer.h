#pragma once
#include "DSPConfig.h"
#include "Dumper.h"
#include <unordered_map>
#include <memory>
#include <string>

#ifdef DEBUG_DUMP
	   #define INITIALIZE_LOGS(a) intializeLogs(a)
       #define CREATE_DUMPER_C0NTAINER(a) dumperContainer::getDumperContainer(a)
       #define INIT_DUMPER(a,b,c,d,e,f) dumpercontainer::getDumperContainer()->createDumper(a,b,c,d,e,f)
       #define DUMP_ARRAY(a,b) dumpercontainer::getDumperContainer()->dump(a,b)
#else
	   #define INITIALIZE_LOGS(a)
       #define CREATE_DUMPER_C0NTAINER(a)
       #define INIT_DUMPER(a,b,c,d,e,f)
       #define DUMP_ARRAY(a,b)
#endif

class DumperContainer
{
private:
    DumperContainer(std::string path) : path_(path) {}
    static DumperContainer* instance;
    //std::unordered_map<std::string, Dumper> dumperMap_;
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
    void createDumper(std::string& name, uint32_t& audio_ptr, uint32_t bufferSize,
        uint32_t dumpSize, uint32_t countMax, uint32_t auPMax);
	template<typename T>
    void dump(T* buf, std::string& name);
};


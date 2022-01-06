#pragma once
#include "ProgressBar.h"
#include <unordered_map>
#include <memory>
#include <vector>

#ifdef USE_PROGRESS_BAR
	#define PROGRESS_BAR_PROGRESS(x) ProgressBarContainer::getProgressBarContainer()->progress(x)
	#define PROGRESS_BAR_FINISH(x) ProgressBarContainer::getProgressBarContainer()->finish(x)
	#define PROGRESS_BAR_CREATE(x,y) ProgressBarContainer::getProgressBarContainer()->createProgressBar(x,y)
#else
	#define PROGRESS_BAR_PROGRESS(x)
	#define PROGRESS_BAR_FINISH(x)
	#define PROGRESS_BAR_CREATE(x,y)
#endif

using progressBarMap_t = std::unordered_map<std::string, std::unique_ptr<ProgressBar> >;

class ProgressBarContainer
{
private:
    static ProgressBarContainer* instance;
    progressBarMap_t progressBarMap_;
    std::vector<std::string> progressBarNameVec_;
public:
    static ProgressBarContainer* getProgressBarContainer();
    void createProgressBar(std::string& name, int numCycles);
    void progress(std::string& name);
    void finish(std::string& name);
    bool allFinished();
    void print();
    progressBarMap_t& getProgressBarMap();

};


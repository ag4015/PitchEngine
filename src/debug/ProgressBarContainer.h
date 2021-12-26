#pragma once
#include "ProgressBar.h"
#include <unordered_map>
#include <memory>
#include <vector>

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

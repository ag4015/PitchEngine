#pragma once
#include "progressbar.hpp"
#include <unordered_map>
#include <memory>

class ProgressBar
{
private:
    std::string name_;
    progressbar bar_;
    bool finished_;
    std::stringstream outStream_;
public:
    ProgressBar(std::string& name, int numCycles);
    void progress();
    void finish();
    bool isFinished();
    std::stringstream& getOutputStream();
    bool first_;
};

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
    progressBarMap_t& getProgressBarMap();

};

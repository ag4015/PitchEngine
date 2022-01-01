
#include "progressbar.hpp"

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
    bool printedAfterFinished_;
};


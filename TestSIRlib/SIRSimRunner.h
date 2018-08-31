#include <string>
#include <cstdlib>
#include <stdexcept>
#include <vector>
#include <string>
#include <future>

#include <SIRlib.h>
#include <CSVExport.h>
#include <PyramidTimeSeries.h>
#include <TimeStatistic.h>
#include <TimeSeries.h>
#include <RNG.h>

using namespace SIRlib;

class SIRSimRunner {
public:
    enum class RunType {Serial, Parallel};

    SIRSimRunner(string fileName, int nTrajectories, double lambda, double gamma, \
           long nPeople, unsigned int ageMin, unsigned int ageMax,       \
           unsigned int ageBreak, unsigned int tMax, unsigned int deltaT,    \
           unsigned int pLength);

    // Alternate constructor without specification of nTrajectories
    SIRSimRunner(string fileName, /*int nTrajectories,*/ double lambda, double gamma, \
           long nPeople, unsigned int ageMin, unsigned int ageMax,           \
           unsigned int ageBreak, unsigned int tMax, unsigned int deltaT,        \
           unsigned int pLength) : \
        SIRSimRunner(fileName, 0, lambda, gamma, nPeople, ageMin, ageMax, ageBreak, tMax, deltaT, pLength) {};

    ~SIRSimRunner(void);

    template<RunType R>
    bool Run(void);

    std::vector<string> Write(void);

private:
    string fileName;
    int nTrajectories;
    double lambda;
    double gamma;

    long nPeople;
    unsigned int ageMin;
    unsigned int ageMax;

    unsigned int ageBreak;
    unsigned int tMax;
    unsigned int deltaT;

    unsigned int pLength;

    SIRSimulation **SIRsims;
};

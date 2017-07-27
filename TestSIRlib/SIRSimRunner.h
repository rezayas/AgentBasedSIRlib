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

    SIRSimRunner(string fileName, int nTrajectories, double λ, double Ɣ, \
           long nPeople, unsigned int ageMin, unsigned int ageMax,       \
           unsigned int ageBreak, unsigned int tMax, unsigned int Δt,    \
           unsigned int pLength);

    // Alternate constructor without specification of nTrajectories
    SIRSimRunner(string fileName, /*int nTrajectories,*/ double λ, double Ɣ, \
           long nPeople, unsigned int ageMin, unsigned int ageMax,           \
           unsigned int ageBreak, unsigned int tMax, unsigned int Δt,        \
           unsigned int pLength) : \
        SIRSimRunner(fileName, 0, λ, Ɣ, nPeople, ageMin, ageMax, ageBreak, tMax, Δt, pLength) {};

    ~SIRSimRunner(void);

    template<RunType R>
    bool Run(void);

    std::vector<string> Write(void);

private:
    string fileName;
    int nTrajectories;
    double λ;
    double Ɣ;

    long nPeople;
    unsigned int ageMin;
    unsigned int ageMax;

    unsigned int ageBreak;
    unsigned int tMax;
    unsigned int Δt;

    unsigned int pLength;

    SIRSimulation **SIRsims;
};

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

struct SIRTrajectoryResult {
    PrevalenceTimeSeries<int> *Susceptible;
    PrevalenceTimeSeries<int> *Infected;
    PrevalenceTimeSeries<int> *Recovered;

    IncidenceTimeSeries<int>  *Infections;
    IncidenceTimeSeries<int>  *Recoveries;

    TimeStatistic             *SusceptibleSx;
    TimeStatistic             *InfectedSx;
    TimeStatistic             *RecoveredSx;
    TimeStatistic             *InfectionsSx;
    TimeStatistic             *RecoveriesSx;

    PyramidTimeSeries         *SusceptiblePyr;
    PyramidTimeSeries         *InfectedPyr;
    PyramidTimeSeries         *RecoveredPyr;
    PyramidTimeSeries         *InfectionsPyr;
    PyramidTimeSeries         *RecoveriesPyr;

    PyramidData<double>       *CaseProfile;
};

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

    vector<SIRTrajectoryResult> GetTrajectoryResults(void);
    SIRTrajectoryResult GetTrajectoryResult(size_t);

    std::vector<string> Write(void);

private:
    SIRTrajectoryResult getTrajectoryResult(size_t);

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

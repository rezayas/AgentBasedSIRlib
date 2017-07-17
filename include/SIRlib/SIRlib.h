#include <string>
#include <memory>
#include <vector>

#include <PrevalenceTimeSeries.h>
#include <PrevalencePyramidTimeSeries.h>
#include <IncidenceTimeSeries.h>
#include <IncidencePyramidTimeSeries.h>
#include <TimeStatistic.h>
#include <DiscreteTimeStatistic.h>
#include <ContinuousTimeStatistic.h>

#include <Bernoulli.h>
#include <UniformDiscrete.h>

using namespace std;
using namespace SimulationLib;

namespace SIRlib {

enum class SIRData {
    Susceptible, Infected, Recovered, Infections, Recoveries;
};

class SIRSimulation {
public:
    SIRSimulation(double λ, double Ɣ, unsigned int nPeople, unsigned int ageMin, \
                  unsigned int ageMax, unsigned int tMax);
    ~SIRSimulation(void);

    bool Run(void);

    template <typename T>
    unique_pointer<T>        GetData(SIRData field);

private:
    double λ;               // Transmission parameter
    double Ɣ;               // Duration of infectiousness (years)
    unsigned int nPeople;   // Number of people at t0
    unsigned int ageMin;    // Minimum age of an individual in initial population
    unsigned int ageMax;    // Maximum age of an individual in initial population
    unsigned int tMax;      // Max value of 't' to run simulation to

    PrevalenceTimeSeries        *Susceptible;
    PrevalenceTimeSeries        *Infected;
    PrevalenceTimeSeries        *Recovered;
    IncidenceTimeSeries         *Infections;
    IncidenceTimeSeries         *Recoveries;

    ContinuousTimeStatistic     *SusceptibleSx;
    ContinuousTimeStatistic     *InfectedSx;
    ContinuousTimeStatistic     *RecoveredSx;
    DiscreteTimeStatistic       *InfectionsSx;
    DiscreteTimeStatistic       *RecoveriesSx;

    PrevalencePyramidTimeSeries *SusceptiblePyr;
    PrevalencePyramidTimeSeries *InfectedPyr;
    PrevalencePyramidTimeSeries *RecoveredPyr;
    IncidencePyramidTimeSeries  *InfectionsPyr;
    IncidencePyramidTimeSeries  *RecoveriesPyr;

    StatisticalDistributions::UniformDiscrete   *ageDist;
    StatisticalDistributions::Bernoulli         *sexDist;

    vector<Individual> Population;
    EventQueue EQ;
};

}

#pragma once

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

#include <RNG.h>
#include <Bernoulli.h>
#include <UniformDiscrete.h>
#include <Exponential.h>
#include <EventQueue.h>

#include "Individual.h"

using namespace std;
using namespace SimulationLib;
using namespace StatisticalDistributions;

namespace SIRlib {

enum class SIRData {
    Susceptible, Infected, Recovered, Infections, Recoveries
};

class SIRSimulation {
public:

    using uint    = unsigned int;
    using PeopleT = uint;
    using AgeT    = uint;
    using DayT    = double;

    using EQ = EventQueue<DayT, bool>;
    using EventFunc = EQ::EventFunc;

    // Creates a new SIRSimulation.
    //
    // λ:
    //   transmission parameter (double | > 0) unit: [cases/day]
    // Ɣ:
    //   duration of infectiousness. (double | > 0) double, unit: [day]
    // nPeople:
    //   number of people in the population (uint | > 0)
    // ageMin:
    //   minimum age of an individual (uint) unit: [years]
    // ageMax:
    //   maximum age of an individual (uint | >= ageMin) unit: [years]
    // ageBreak:
    //   interval between age breaks of population (uint | > 1, < (ageMax - ageMin)) unit: [years]
    // tMax:
    //   maximum length of time to run simulation to (uint | >= 1) unit: [days]
    // Δt:
    //   timestep (uint | >= 1, <= tMax) unit: [days]
    // pLength:
    //   length of one data-aggregation period (uint | > 0, < tMax) unit: [days]
    SIRSimulation(RNG *rng, double _λ, double _Ɣ, uint _nPeople, \
                  uint _ageMin, uint _ageMax, uint _ageBreak,    \
                  uint _tMax, uint _Δt,                          \
                  uint _pLength);

    // Currently buggy. Frees memory associated with the simulation
    ~SIRSimulation(void);

    // Runs the simulation, returning 'true' on success
    bool Run(void);

    // Allows access to data generated by the simulation. Supported data structures
    // are TimeSeries, TimeStatistics, and PyramidTimeSeries.
    template <typename T>
    T *GetData(SIRData field);

private:
    double λ;        // Transmission parameter
    double Ɣ;        // Duration of infectiousness (years)
    PeopleT nPeople; // Number of people at t0
    AgeT ageMin;     // Minimum age of an individual in initial population
    AgeT ageMax;     // Maximum age of an individual in initial population
    AgeT ageBreak;   // The uniform interval for the age breaks of the population
    DayT tMax;       // Max value of 't' to run simulation to
    DayT Δt;         // Timestep
    DayT pLength;    // Period length

    RNG *rng;

    // TimeSeries datastores
    PrevalenceTimeSeries<int>        *Susceptible;
    PrevalenceTimeSeries<int>        *Infected;
    PrevalenceTimeSeries<int>        *Recovered;
    IncidenceTimeSeries<int>         *Infections;
    IncidenceTimeSeries<int>         *Recoveries;

    // TimeStatistics datastores
    ContinuousTimeStatistic     *SusceptibleSx;
    ContinuousTimeStatistic     *InfectedSx;
    ContinuousTimeStatistic     *RecoveredSx;
    DiscreteTimeStatistic       *InfectionsSx;
    DiscreteTimeStatistic       *RecoveriesSx;

    // PyramidTimeSeries datastores
    PrevalencePyramidTimeSeries *SusceptiblePyr;
    PrevalencePyramidTimeSeries *InfectedPyr;
    PrevalencePyramidTimeSeries *RecoveredPyr;
    IncidencePyramidTimeSeries  *InfectionsPyr;
    IncidencePyramidTimeSeries  *RecoveriesPyr;

    // Age distribution and sex distribution
    StatisticalDistributions::UniformDiscrete   *ageDist;
    StatisticalDistributions::Bernoulli         *sexDist;
    StatisticalDistributions::Exponential       *timeToRecoveryDist;

    // Vector of Individuals who comprise the population
    vector<Individual> Population;

    // Pointer to the EventQueue holding scheduled events
    EQ *eq;

    // Increments the relevant TimeSeries and PyramidTimeSeries by 'increment'
    //   for an individual of properties specified by 'idv' at time 't'. Returns
    //   true on successful increment, false otherwise.
    bool IdvIncrement(DayT t, SIRData dtype, Individual idv, int increment);

    // ––- Event Generators: InfectionEvent, RecoveryEvent, and FOIEvent -––

    // Creates an event for an infection of individual 'individualIdx'
    // On execution of event, recovery of individual is scheduled according to
    //   function 'timeToRecovery' with input parameter 't' set to time of
    //   infection.
    EventFunc InfectionEvent(int individualIdx);

    // Creates an event for the recovery of individual 'individualIdx'.
    EventFunc RecoveryEvent(int individualIdx);

    // Creates an event for a Force-Of-Infection event.
    // An FOIEvent calculates the time-to-infection of each Individual in the
    //   population. If the time-to-infection is less than dt, the infection
    //   of the individual is scheduled on the EventQueue. Additionally,
    //   the FOIEvent schedules the next FOIEvent for time 't + dt'.
    EventFunc FOIUpdateEvent(void);

    // Calculates time to infection at time 't'
    DayT timeToInfection(DayT t);

    // Calculates time to recovery for infection occurring at time 't'
    DayT timeToRecovery(DayT t);
};

}

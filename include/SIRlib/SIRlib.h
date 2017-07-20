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

#include "../include/SIRlib/EventQueue.h"
#include "../include/SIRlib/Individual.h"

using namespace std;
using namespace SimulationLib;
using namespace StatisticalDistributions;


namespace SIRlib {

enum class SIRData {
    Susceptible, Infected, Recovered, Infections, Recoveries
};

class SIRSimulation {
    enum class Events {Infection, Recovery, FOIUpdate};
    using EQ = EventQueue<Events, double>;
    using UnaryFunction = function<double(double)>;

    double λ;               // Transmission parameter
    double Ɣ;               // Duration of infectiousness (years)
    uint nPeople;   // Number of people at t0
    uint ageMin;    // Minimum age of an individual in initial population
    uint ageMax;    // Maximum age of an individual in initial population
    uint tMax;      // Max value of 't' to run simulation to
    // uint ∆t;        // Timestep
    uint dt;        // Timestep
    uint pLength;   // Period length

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

    // Vector of Individuals who comprise the population
    vector<Individual> Population;

    // Pointer to the EventQueue holding scheduled events
    EQ *eq;

    // Increments the relevant TimeSeries and PyramidTimeSeries by 'increment'
    //   for an individual of properties specified by 'idv' at time 't'. Returns
    //   true on successful increment, false otherwise.
    bool IdvIncrement(double t, SIRData dtype, Individual idv, int increment);

    // ––Event Generators: InfectionEvent, RecoveryEvent, and FOIEvent––

    // Creates an event for an infection of individual 'individualIdx' with
    //   an associated unary function 'timeToRecovery' which must take a 'time'
    //   parameter and return the dt until recovery of that individual.
    // EQ::EventFunc<> InfectionEvent(uint individualIdx, UnaryFunction timeToRecovery);
    EQ::EventFunc<int> InfectionEvent(int individualIdx);

    // Creates an event for the recovery of individual 'individualIdx'.
    // EQ::EventFunc<> RecoveryEvent(uint individualIdx);
    EQ::EventFunc<int> RecoveryEvent(int individualIdx);

    // Creates an event for a Force-Of-Infection event.
    // An FOIEvent calculates the time-to-infection of each Individual in the
    //   population. If the time-to-infection is less than dt, the infection
    //   of the individual is scheduled on the EventQueue. Additionally,
    //   the FOIEvent schedules the next FOIEvent for time 't + dt'.
    // EQ::EventFunc<> FOIEvent(UnaryFunction timeToRecovery, UnaryFunction timeToInfection);
    EQ::EventFunc<int> FOIUpdateEvent(int individualIdx);

    double timeToInfection(double t);
    double timeToRecovery(double t);

public:
    SIRSimulation(double _λ, double _Ɣ, uint _nPeople, \
                  uint _ageMin, uint _ageMax,          \
                  uint _tMax, uint _dt,                \
                  uint _pLength);
    ~SIRSimulation(void);

    bool Run(void);

    template <typename T>
    unique_ptr<T> GetData(SIRData field);
};

}

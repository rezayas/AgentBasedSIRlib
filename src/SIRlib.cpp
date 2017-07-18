#include <cstdio>
#include <stdexcept>

#include "../include/SIRlib/SIRlib.h"
#include "../include/SIRlib/Individual.h"
#include "../include/SIRlib/EventQueue.h"

using namespace std;
using namespace SimulationLib;
using namespace SIRlib;

SIRSimulation::SIRSimulation(double _λ, double _Ɣ, unsigned int _nPeople, \
                             unsigned int _ageMin, unsigned int _ageMax, \
                             unsigned int _tMax, unsigned int _∆t, \
                             unsigned int _pLength)
{
    λ       = _λ;
    Ɣ       = _Ɣ;
    nPeople = _nPeople;
    ageMin  = _ageMin;
    ageMax  = _ageMax;
    tMax    = _tMax;
    ∆t      = _∆t;
    pLength = _pLength;

    if (λ <= 0)
        throw out_of_range("λ was <= 0");
    if (Ɣ <= 0)
        throw out_of_range("Ɣ was <= 0");
    if (nPeople < 1)
        throw out_of_range("'nPeople' < 1");
    if (!(ageMin <= ageMax))
        throw out_of_range("ageMin > ageMax");
    if (tMax < 1)
        throw out_of_range("tMax < 1");
    if (tMax % pLength != 0)
        printf("Warning: tmax % pLength != 0\n");
    if (pLength == 0)
        throw out_of_range("pLength == 0")
    if (pLength > tMax)
        throw out_of_range("pLength > tMax");
    if (∆t < 1)
        throw out_of_range("∆t < 1");
    if (tMax % ∆t != 0)
        printf("Warning: tMax / ∆t != 0\n");
    if (∆t > tMax)
        throw out_of_range("∆t > tMax");

    vector<double> defaultAgeBreaks{10, 20, 30, 40, 50, 60, 70, 80, 90};

    auto CTSx = ContinuousTimeStatistic;
    auto DTSx = DiscreteTimeStatistic;
    auto PTS  = PrevalenceTimeSeries;
    auto PPTS = PrevalencePyramidTimeSeries;
    auto ITS  = IncidenceTimeSeries;
    auto IPTS = IncidencePyramidTimeSeries;

    SusceptibleSx = new CTSx("Susceptible");
    InfectedSx    = new CTSx("Infected");
    RecoveredSx   = new CTSx("Recovered");
    InfectionsSx  = new DTSx("Infections");
    RecoveriesSx  = new DTSx("Recoveries");

    Susceptible = new PTS("Susceptible",   tMax, pLength, 1, SusceptibleSx);
    Infected    = new PTS("Infected" ,     tMax, pLength, 1, InfectedSx);
    Recovered   = new PTS("Recovered",     tMax, pLength, 1, RecoveredSx);
    Infections  = new ITS("Infections", 0, tMax, pLength, 1, InfectionsSx);
    Recoveries  = new ITS("Recoveries", 0, tMax, pLength, 1, RecoveriesSx);

    SusceptiblePyr = new PPTS("Susceptible", 0, tMax, pLength, 2, defaultAgeBreaks);
    InfectedPyr    = new PPTS("Infected",    0, tMax, pLength, 2, defaultAgeBreaks);
    RecoveredPyr   = new PPTS("Recovered",   0, tMax, pLength, 2, defaultAgeBreaks);
    InfectionsPyr  = new IPTS("Infections",   0, tMax, pLength, 2, defaultAgeBreaks);
    RecoveriesPyr  = new IPTS("Recoveries",   0, tMax, pLength, 2, defaultAgeBreaks);

    ageDist = new StatisticalDistributions::UniformDiscrete(ageMin, ageMax + 1);
    sexDist = new StatisticalDistributions::Bernoulli(0.5);
}

SIRSimulation::~SIRSimulation()
{
    delete Susceptible;
    delete Infected;
    delete Recovered;
    delete Infections;
    delete Recoveries;

    delete SusceptibleSx;
    delete InfectedSx;
    delete RecoveredSx;
    delete InfectionsSx;
    delete RecoveriesSx;

    delete SusceptiblePyr;
    delete InfectedPyr;
    delete RecoveredPyr;
    delete InfectionsPyr;
    delete RecoveriesPyr;
}

bool SIRSimulation::IdvIncrement(double t, SIRData dtype, Individual idv, int increment) {
    function<bool(int, int, double, int)> pts = nullptr;
    function<bool(double, int)>           ts  = nullptr;

    switch (dtype) {
        case SIRData::Susceptible: pts = SusceptiblePyr->UpdateByAge;
                                    ts = Susceptible->Record;
                                   break;
        case SIRData::Infected:    pts = InfectedPyr->UpdateByAge;
                                    ts = Infected->Record;
                                   break;
        case SIRData::Recovered:   pts = RecoveredPyr->UpdateByAge;
                                    ts = Recovered->Record;
                                   break;
        case SIRData::Infections:  pts = InfectionsPyr->UpdateByAge;
                                    ts = Infections->Record;
                                   break;
        case SIRData::Recoveries:  pts = RecoveriesPyr->UpdateByAge;
                                    ts = Recoveries->Record;
                                   break;
        default: break;
    }

    if (pts == nullptr || ts == nullptr)
        return false;

    if (!pts(t, sexN(idv), idv.age, increment))
        return false;

    if (!ts(t, increment))
        return false;

    return true;
}


template <typename ...Params>
using scheduler = function<void(int, Event, Params&&...params)>

EQ::EventFunc SIRSimulation::InfectionEvent(unsigned int individualIdx, \
                                              function<double(double)>timeToRecovery) {
    if (individualIdx >= nPeople)
        throw out_of_range("individualIdx >= nPeople");

    return [](double t, scheduler<...>Schedule) {
        Individual idv = Population.at(individualIdx);

        // Decrease susceptible quantity
        IdvIncrement(t, SIRData::Susceptible, idv, -1);

        // Increase infected quantity
        IdvIncrement(t, SIRData::Infected, idv, +1);
        IdvIncrement(t, SIRData::Infections, idv, +1);

        // Schedule recovery
        Schedule(t + timeToRecovery(t), Events::Recovery, individualIdx);

        Population[individualIdx] = changeHealthState(idv, HealthState::Infected);

        return success;
    };
}

EQ::EventFunc SIRSimulation::RecoveryEvent(unsigned int individualIdx) {
    if (individualIdx >= nPeople)
        throw out_of_range("individualIdx >= nPeople");

    return [](double t, scheduler<...>Schedule) {
        Individual idv = Population.at(individualIdx);

        IdvIncrement(t, SIRData::Infected, idv, -1);
        IdvIncrement(t, SIRData::Recovered, idv, +1);
        IdvIncrement(t, SIRData::Recoveries, idv, +1);

        Population[individualIdx] = changeHealthState(idv, HealthState::Recovered);

        return true;
    }
}

EQ::EventFunc SIRSimulation::FOIEvent(function<double(double)> timeToInfection, \
                                        function<double(double)> timeToRecovery) {
    if (timeToInfection == nullptr)
        throw invalid_argument("timeToInfection was == nullptr");
    if (timeToRecovery == nullptr)
        throw invalid_argument("timeToInfection was == nullptr");

    return [](double t, scheduler<...>Schedule) {
        int idvIndex = 0;

        // For each individual, schedule infection if timeToInfection(t) < ∆t
        for (auto individual : Population) {
            if (timeToInfection(t) < ∆t)
                Schedule(t + timeToInfection(t), Events::Infection, idvIndex, timeToRecovery);
            idvIndex += 1;
        }

        // Schedule next UpdateFOI
        return Schedule(t + ∆t, Events::FOIUpdate, timeToInfection, timeToRecovery);
    }
}



bool SIRSimulation::Run(void)
{
    // Create 'nPeople' susceptible individuals
    for (int i = 0; i < nPeople; i++)
        Population.push_back(newIndividual(rng, ageDist, sexDist, HealthState::Susceptible));

    // Schedule infection for the first individual
    EQ->schedule(0 + timeToInfection(0), Events::Infection, 0, timeToRecovery)

    // While there is an event on the calendar
    while(!EQ->empty()) {

        // Grab the next event
        auto e = EQ->top();

        // Run the event
        e.second(e.first, EQ->schedule);

        // Remove it from the event queue
        EQ->pop();
    }
}

unique_pointer<TimeSeries> SIRSimulation::GetData<TimeSeries>(SIRData field)
{
    switch(field) {
        case SIRData::Susceptible: return Susceptible;
        case SIRData::Infected:    return Infected;
        case SIRData::Recovered:   return Recovered;
        case SIRData::Infections:  return Infections;
        case SIRData::Recoveries:  return Recoveries;
        default:                   return nullptr;
    }
}

unique_pointer<TimeStatistics> SIRSimulation::GetData<TimeStatistics>(SIRData field)
{
    switch(field) {
        case SIRData::Susceptible: return SusceptibleSx;
        case SIRData::Infected:    return InfectedSx;
        case SIRData::Recovered:   return RecoveredSx;
        case SIRData::Infections:  return InfectionsSx;
        case SIRData::Recoveries:  return RecoveriesSx;
        default:                   return nullptr;
    }
}

unique_pointer<PyramidTimeSeries> SIRSimulation::GetData<PyramidTimeSeries>(SIRData field)
{
    switch(field) {
        case SIRData::Susceptible: return SusceptiblePyr;
        case SIRData::Infected:    return InfectedPyr;
        case SIRData::Recovered:   return RecoveredPyr;
        case SIRData::Infections:  return InfectionsPyr;
        case SIRData::Recoveries:  return RecoveriesPyr;
        default:                   return nullptr;
    }
}

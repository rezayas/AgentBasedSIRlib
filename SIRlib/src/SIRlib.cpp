#include <cstdio>
#include <stdexcept>

#include "../include/SIRlib/SIRlib.h"
#include "../include/SIRlib/Individual.h"
#include "../include/SIRlib/EventQueue.h"

using namespace std;
using namespace SimulationLib;
using namespace SIRlib;

using CTSx = ContinuousTimeStatistic;
using DTSx = DiscreteTimeStatistic;
using PTS  = PrevalenceTimeSeries<int>;
using PPTS = PrevalencePyramidTimeSeries;
using ITS  = IncidenceTimeSeries<int>;
using IPTS = IncidencePyramidTimeSeries;

using TS   = TimeSeries<int>;
using TSx  = TimeStatistic;
using PyTS = PyramidTimeSeries;

SIRSimulation::SIRSimulation(double _λ, double _Ɣ, uint _nPeople, \
                             uint _ageMin, uint _ageMax,          \
                             uint _tMax, uint _dt,                \
                             uint _pLength)
{
    λ       = _λ;
    Ɣ       = _Ɣ;
    nPeople = _nPeople;
    ageMin  = _ageMin;
    ageMax  = _ageMax;
    tMax    = _tMax;
    dt      = _dt;
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
        printf("Warning: tMax %% pLength != 0\n");
    if (pLength == 0)
        throw out_of_range("pLength == 0");
    if (pLength > tMax)
        throw out_of_range("pLength > tMax");
    if (dt < 1)
        throw out_of_range("dt < 1");
    if (tMax % dt != 0)
        printf("Warning: tMax %% dt != 0\n");
    if (dt > tMax)
        throw out_of_range("dt > tMax");

    rng = new RNG(time(NULL));

    vector<double> defaultAgeBreaks{10, 20, 30, 40, 50, 60, 70, 80, 90};

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

    map<Events, EQ::EventGenerator<int>> eventGenMap;
    eventGenMap[Events::Infection] = bind(&SIRSimulation::InfectionEvent, this, placeholders::_1);
    eventGenMap[Events::Recovery]  = bind(&SIRSimulation::RecoveryEvent, this, placeholders::_1);
    eventGenMap[Events::FOIUpdate] = bind(&SIRSimulation::FOIUpdateEvent, this, placeholders::_1);

    eq = new EQ(eventGenMap);
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
    switch (dtype) {
        case SIRData::Susceptible:
            return SusceptiblePyr->UpdateByAge(t, sexN(idv.sex), idv.age, increment)
                && Susceptible->Record(t, increment);

        case SIRData::Infected:
            return InfectedPyr->UpdateByAge(t, sexN(idv.sex), idv.age, increment)
                && Infected->Record(t, increment);

        case SIRData::Recovered:
            return RecoveredPyr->UpdateByAge(t, sexN(idv.sex), idv.age, increment)
                && Recovered->Record(t, increment);

        case SIRData::Infections:
            return InfectionsPyr->UpdateByAge(t, sexN(idv.sex), idv.age, increment)
                && Infections->Record(t, increment);

        case SIRData::Recoveries:
            return RecoveriesPyr->UpdateByAge(t, sexN(idv.sex), idv.age, increment)
                && Recoveries->Record(t, increment);

        default:
            return false;
    }
}

SIRSimulation::EQ::EventFunc<int> SIRSimulation::InfectionEvent(int individualIdx) {
    if (individualIdx >= nPeople)
        throw out_of_range("individualIdx >= nPeople");

    SIRSimulation::EQ::EventFunc<int> ef =
      [this,individualIdx](double t, function<void(double, Events, int)> Schedule) {
        Individual idv = Population.at(individualIdx);

        // Decrease susceptible quantity
        IdvIncrement(t, SIRData::Susceptible, idv, -1);

        // Increase infected quantity
        IdvIncrement(t, SIRData::Infected, idv, +1);
        IdvIncrement(t, SIRData::Infections, idv, +1);

        // Schedule recovery
        Schedule(t + timeToRecovery(t), Events::Recovery, individualIdx);

        Population[individualIdx] = changeHealthState(idv, HealthState::Infected);

        return true;
    };

    return ef;
}

SIRSimulation::EQ::EventFunc<int> SIRSimulation::RecoveryEvent(int individualIdx) {
    if (individualIdx >= nPeople)
        throw out_of_range("individualIdx >= nPeople");

    SIRSimulation::EQ::EventFunc<int> ef =
      [this, individualIdx](double t, function<void(double, Events, int)> Schedule) {
        Individual idv = Population.at(individualIdx);

        IdvIncrement(t, SIRData::Infected, idv, -1);
        IdvIncrement(t, SIRData::Recovered, idv, +1);
        IdvIncrement(t, SIRData::Recoveries, idv, +1);

        Population[individualIdx] = changeHealthState(idv, HealthState::Recovered);

        return true;
    };

    return ef;
}

SIRSimulation::EQ::EventFunc<int> SIRSimulation::FOIUpdateEvent(int individualIdx) {
    SIRSimulation::EQ::EventFunc<int> ef =
     [this, individualIdx](double t, function<void(double, Events, int)> Schedule) {
        int idvIndex = 0;

        // For each individual, schedule infection if timeToInfection(t) < dt
        for (auto individual : Population) {
            if (timeToInfection(t) < dt)
                Schedule(t + timeToInfection(t), Events::Infection, idvIndex);
            idvIndex += 1;
        }

        // Schedule next UpdateFOI
        Schedule(t + dt, Events::FOIUpdate, 0);

        return true;
    };

    return ef;
}

double SIRSimulation::timeToInfection(double t) {
    // Needs to be sampled from an exponential distribution

    double forceOfInfection;
    double I_t, N_t;

    I_t = 1;  // Needs to be replaced with query to Infected TS
    N_t = 10; // Same deal

    forceOfInfection = λ * (I_t / N_t);

    return forceOfInfection;
}

double SIRSimulation::timeToRecovery(double t) {
    // Needs to be sampled from an exponential distribution

    return Ɣ;
}

bool SIRSimulation::Run(void)
{
    TimeSeriesCSVExport<int>     exportSusceptible(fileName + string("-susceptible.csv"));
    TimeSeriesCSVExport<int>     exportInfected(fileName + string("-infected.csv"));
    TimeSeriesCSVExport<int>     exportRecovered(fileName + string("-recovered.csv"));
    TimeSeriesCSVExport<int>     exportInfections(fileName + string("-infections.csv"));
    TimeSeriesCSVExport<int>     exportRecoveries(fileName + string("-recoveries.csv"));

    PyramidTimeSeriesCSVExport     exportSusceptiblePyr(fileName + string("-pyramid-susceptible.csv"));
    PyramidTimeSeriesCSVExport     exportInfectedPyr(fileName + string("-pyramid-infected.csv"));
    PyramidTimeSeriesCSVExport     exportRecoveredPyr(fileName + string("-pyramid-recovred.csv"));
    PyramidTimeSeriesCSVExport     exportInfectionsPyr(fileName + string("-pyramid-infections.csv"));
    PyramidTimeSeriesCSVExport     exportRecoveriesPyr(fileName + string("-pyramid-recoveries.csv"));

    // Create 'nPeople' susceptible individuals
    for (int i = 0; i < nPeople; i++)
        Population.push_back(newIndividual(rng, ageDist, sexDist, HealthState::Susceptible));

    double timeOfFirstInfection = 0 + timeToInfection(0);
    double timeOfFirstFOI = timeOfFirstInfection + 0.001;

    // Schedule infection for the first individual (individualIdx = 0)
    eq->schedule(timeOfFirstInfection, Events::Infection, 0);

    // Question: When to run the first FOI event??
    // Assumption: Run it at the first dt after the first infection
    eq->schedule(timeOfFirstFOI, Events::FOIUpdate, 0);

    // While there is an event on the calendar
    while(!eq->empty()) {

        // Grab the next event
        auto e = eq->top();

        // Run the event
        e.second();

        // Remove it from the event queue
        eq->pop();
    }

    exportSusceptible.Add(Susceptible);
    exportInfected.Add(Infected);
    exportRecovered.Add(Recovered);
    exportInfections.Add(Infections);
    exportRecoveries.Add(Recoveries);

    exportSusceptiblePyr.Add(SusceptiblePyr);
    exportInfectedPyr.Add(InfectedPyr);
    exportRecoveredPyr.Add(RecoveredPyr);
    exportInfectionsPyr.Add(InfectionsPyr);
    exportRecoveriesPyr.Add(RecoveriesPyr);        

    exportSusceptible.Write();
    exportInfected.Write();
    exportRecovered.Write();
    exportInfections.Write();
    exportRecoveries.Write();

    exportSusceptiblePyr.Write();
    exportInfectedPyr.Write();
    exportRecoveredPyr.Write();
    exportInfectionsPyr.Write();
    exportRecoveriesPyr.Write();     

    return true;
}

template <>
unique_ptr<TS> SIRSimulation::GetData<TS>(SIRData field)
{
    switch(field) {
        case SIRData::Susceptible: return unique_ptr<TS>(Susceptible);
        case SIRData::Infected:    return unique_ptr<TS>(Infected);
        case SIRData::Recovered:   return unique_ptr<TS>(Recovered);
        case SIRData::Infections:  return unique_ptr<TS>(Infections);
        case SIRData::Recoveries:  return unique_ptr<TS>(Recoveries);
        default:                   return unique_ptr<TS>(nullptr);
    }
}

template <>
unique_ptr<TSx> SIRSimulation::GetData<TSx>(SIRData field)
{
    switch(field) {
        case SIRData::Susceptible: return unique_ptr<TSx>(SusceptibleSx);
        case SIRData::Infected:    return unique_ptr<TSx>(InfectedSx);
        case SIRData::Recovered:   return unique_ptr<TSx>(RecoveredSx);
        case SIRData::Infections:  return unique_ptr<TSx>(InfectionsSx);
        case SIRData::Recoveries:  return unique_ptr<TSx>(RecoveriesSx);
        default:                   return unique_ptr<TSx>(nullptr);
    }
}

template <>
unique_ptr<PyTS> SIRSimulation::GetData<PyTS>(SIRData field)
{
    switch(field) {
        case SIRData::Susceptible: return unique_ptr<PyTS>(SusceptiblePyr);
        case SIRData::Infected:    return unique_ptr<PyTS>(InfectedPyr);
        case SIRData::Recovered:   return unique_ptr<PyTS>(RecoveredPyr);
        case SIRData::Infections:  return unique_ptr<PyTS>(InfectionsPyr);
        case SIRData::Recoveries:  return unique_ptr<PyTS>(RecoveriesPyr);
        default:                   return unique_ptr<PyTS>(nullptr);
    }
}

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

using uint = unsigned int;
using EQ = EventQueue<double, bool>;
using EventFunc = EQ::EventFunc;
using SchedulerT = EQ::SchedulerT;

SIRSimulation::SIRSimulation(double _λ, double _Ɣ, uint _nPeople,        \
                             uint _ageMin, uint _ageMax, uint _ageBreak, \
                             uint _tMax, uint _dt,                       \
                             uint _pLength)
{
    λ        = _λ;
    Ɣ        = _Ɣ;
    nPeople  = _nPeople;
    ageMin   = _ageMin;
    ageMax   = _ageMax;
    ageBreak = _ageBreak;
    tMax     = _tMax;
    dt       = _dt;
    pLength  = _pLength;

    if (λ <= 0)
        throw out_of_range("λ was <= 0");
    if (Ɣ <= 0)
        throw out_of_range("Ɣ was <= 0");
    if (nPeople < 1)
        throw out_of_range("'nPeople' < 1");
    if (!(ageMin <= ageMax))
        throw out_of_range("ageMin > ageMax");
    if (ageBreak < 1)
        throw out_of_range("ageBreak < 1");
    if (ageBreak > (ageMax-ageMin))
        throw out_of_range("ageBreak > ageMax - ageMin");
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

    vector<double> ageBreaks;
    for (int i = ageMin + ageBreak; i < ageMax; i += ageBreak)
    {
        ageBreaks.push_back(i);
    }

    timeToRecoveryDist = new StatisticalDistributions::Exponential(1/Ɣ);

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

    SusceptiblePyr = new PPTS("Susceptible", 0, tMax, pLength, 2, ageBreaks);
    InfectedPyr    = new PPTS("Infected",    0, tMax, pLength, 2, ageBreaks);
    RecoveredPyr   = new PPTS("Recovered",   0, tMax, pLength, 2, ageBreaks);
    InfectionsPyr  = new IPTS("Infections",   0, tMax, pLength, 2, ageBreaks);
    RecoveriesPyr  = new IPTS("Recoveries",   0, tMax, pLength, 2, ageBreaks);

    ageDist = new StatisticalDistributions::UniformDiscrete(ageMin, ageMax + 1);
    sexDist = new StatisticalDistributions::Bernoulli(0.5);

    eq = new EQ{};
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

EventFunc SIRSimulation::InfectionEvent(int individualIdx) {
    if (individualIdx >= nPeople)
        throw out_of_range("individualIdx >= nPeople");

    printf("Infection: scheduled %d\n", individualIdx);

    EventFunc ef =
      [this,individualIdx](double t, SchedulerT Schedule) {
        printf("Infection: infecting %d\n", individualIdx);
        Individual idv = Population.at(individualIdx);

        // Decrease susceptible quantity
        IdvIncrement(t, SIRData::Susceptible, idv, -1);

        // Increase infected quantity
        IdvIncrement(t, SIRData::Infected, idv, +1);
        IdvIncrement(t, SIRData::Infections, idv, +1);

        // Make recovery event
        auto recoveryEvent =
          eq->MakeScheduledEvent(t + timeToRecovery(t), RecoveryEvent(individualIdx));

        // Schedule recovery
        Schedule(recoveryEvent);

        Population[individualIdx] = changeHealthState(idv, HealthState::Infected);

        return true;
    };

    return ef;
}

EventFunc SIRSimulation::RecoveryEvent(int individualIdx) {
    if (individualIdx >= nPeople)
        throw out_of_range("individualIdx >= nPeople");

    printf("Recovery: scheduled %d\n", individualIdx);

    EventFunc ef =
      [this, individualIdx](double t, SchedulerT Schedule) {
        printf("Recovery: recovered %d\n", individualIdx);
        Individual idv = Population.at(individualIdx);

        IdvIncrement(t, SIRData::Infected, idv, -1);
        IdvIncrement(t, SIRData::Recovered, idv, +1);
        IdvIncrement(t, SIRData::Recoveries, idv, +1);

        Population[individualIdx] = changeHealthState(idv, HealthState::Recovered);

        return true;
    };

    return ef;
}

EventFunc SIRSimulation::FOIUpdateEvent() {
    EventFunc ef =
     [this](double t, SchedulerT Schedule) {
        printf("FOI: updating FOI\n");
        int idvIndex = 0;

        // For each individual, schedule infection if timeToInfection(t) < dt
        double ttI;
        for (auto individual : Population) {
            if (individual.hs == HealthState::Susceptible &&
                (ttI = timeToInfection(t)) < dt)
                Schedule(eq->MakeScheduledEvent(t + ttI, InfectionEvent(idvIndex)));
            idvIndex += 1;
        }

        // Create next UpdateFOIEvent
        auto UpdateFOIEvent =
          eq->MakeScheduledEvent(t + dt, FOIUpdateEvent());

        // Schedule next UpdateFOI
        Schedule(UpdateFOIEvent);

        return true;
    };

    return ef;
}

double SIRSimulation::timeToInfection(double t) {
    // Needs to be sampled from an exponential distribution

    double forceOfInfection;
    double I_t, N_t;

    I_t = Infected->GetTotalAtTime(t);  // PROBABLY buggy!!!!!!!!

    if (I_t == 0)
        I_t = Infected->GetCurrentPrevalence();

    N_t = nPeople; // (N_t never changes)

    printf("I_t = %f\n", I_t);

    forceOfInfection = λ * (I_t / N_t);

    return StatisticalDistributions::Exponential(forceOfInfection).Sample(*rng);
}

double SIRSimulation::timeToRecovery(double t) {
    // Needs to be sampled from an exponential distribution

    return timeToRecoveryDist->Sample(*rng);
}

bool SIRSimulation::Run(void)
{
    // Create 'nPeople' susceptible individuals
    for (int i = 0; i < nPeople; i++) {
        auto idv = newIndividual(rng, ageDist, sexDist, HealthState::Susceptible);
        Population.push_back(idv);
        IdvIncrement(0, SIRData::Susceptible, idv, +1);
    }

    double timeOfFirstInfection = 0/* + timeToInfection(0)*/;
    double timeOfFirstFOI = timeOfFirstInfection + 0.001;

    auto firstInfection = eq->MakeScheduledEvent(timeOfFirstInfection,
                                                 InfectionEvent(0));
    auto firstFOI = eq->MakeScheduledEvent(timeOfFirstFOI,
                                           FOIUpdateEvent());

    eq->Schedule(firstInfection);
    eq->Schedule(firstFOI);

    // While there is an event on the calendar
    while(!eq->Empty()) {
        // printf("Running event!\n");

        // Grab the next event
        auto e = eq->Top();

        if (e.t >= tMax) {
            printf("Reached tMax\n");
            break;
        }

        // Run the event
        e.run();

        // Check that there are any infected people left
        // If not, break
        if (Infected->GetCurrentPrevalence() == 0) {
            printf("No more infected people\n");
            break;
        }

        // Remove it from the event queue
        eq->Pop();
    }

    Susceptible->Close();
    Infected->Close();
    Recovered->Close();
    Infections->Close();
    Recoveries->Close();

    SusceptiblePyr->Close();
    InfectedPyr->Close();
    RecoveredPyr->Close();
    InfectionsPyr->Close();
    RecoveriesPyr->Close();

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

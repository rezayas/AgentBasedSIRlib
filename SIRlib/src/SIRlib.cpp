#include <cstdio>
#include <stdexcept>

#include "../include/SIRlib/SIRlib.h"
#include "../include/SIRlib/Individual.h"
#include "../include/SIRlib/EventQueue.h"

using namespace std;
using namespace SimulationLib;
using namespace SIRlib;


// Alias for unsigned integers
using uint = unsigned int;

// Aliases for EventQueue
using EQ         = SIRSimulation::EQ;
using EventFunc  = EQ::EventFunc;
using SchedulerT = EQ::SchedulerT;

using PeopleT = SIRSimulation::PeopleT;
using AgeT    = SIRSimulation::AgeT;
using DayT    = SIRSimulation::DayT;

// Aliases for specialized data structures
using CTSx = ContinuousTimeStatistic;
using DTSx = DiscreteTimeStatistic;
using PTS  = PrevalenceTimeSeries<int>;
using PPTS = PrevalencePyramidTimeSeries;
using ITS  = IncidenceTimeSeries<int>;
using IPTS = IncidencePyramidTimeSeries;

// Aliases for unspecialized data structures
using TS   = TimeSeries<int>;
using TSx  = TimeStatistic;
using PyTS = PyramidTimeSeries;

SIRSimulation::SIRSimulation(double _λ, double _Ɣ, uint _nPeople,        \
                             uint _ageMin, uint _ageMax, uint _ageBreak, \
                             uint _tMax, uint _Δt,                       \
                             uint _pLength)
{
    λ        = _λ;
    Ɣ        = _Ɣ;
    nPeople  = (PeopleT)_nPeople;
    ageMin   = (AgeT)_ageMin;
    ageMax   = (AgeT)_ageMax;
    ageBreak = (AgeT)_ageBreak;
    tMax     = (DayT)_tMax;
    Δt       = (DayT)_Δt;
    pLength  = (DayT)_pLength;

    // Check to make sure parameters satisfy constraints
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
    if ((uint)tMax % (uint)pLength != 0)
        printf("Warning: tMax %% pLength != 0\n");
    if (pLength == 0)
        throw out_of_range("pLength == 0");
    if (pLength > tMax)
        throw out_of_range("pLength > tMax");
    if (Δt < 1)
        throw out_of_range("Δt < 1");
    if ((uint)tMax % (uint)Δt != 0)
        printf("Warning: tMax %% Δt != 0\n");
    if (Δt > tMax)
        throw out_of_range("Δt > tMax");

    // Seed RNG with current system time
    rng = new RNG(time(NULL));

    // Create age breaks from [0, ageMax) every 'ageBreak's
    vector<double> ageBreaks;
    for (int i = ageMin + ageBreak; i < ageMax; i += ageBreak)
        ageBreaks.push_back(i);

    // --- Instantiate data structures ---

    // Create statistics data structures
    SusceptibleSx = new CTSx("Susceptible");
    InfectedSx    = new CTSx("Infected");
    RecoveredSx   = new CTSx("Recovered");
    InfectionsSx  = new DTSx("Infections");
    RecoveriesSx  = new DTSx("Recoveries");

    // Create PrevalenceTimeSeries and IncidenceTimeSeries structures
    Susceptible = new PTS("Susceptible",   (uint)tMax, (uint)pLength, 1, SusceptibleSx);
    Infected    = new PTS("Infected" ,     (uint)tMax, (uint)pLength, 1, InfectedSx);
    Recovered   = new PTS("Recovered",     (uint)tMax, (uint)pLength, 1, RecoveredSx);
    Infections  = new ITS("Infections", 0, (uint)tMax, (uint)pLength, 1, InfectionsSx);
    Recoveries  = new ITS("Recoveries", 0, (uint)tMax, (uint)pLength, 1, RecoveriesSx);

    // Create PrevalencePyramidTimeSeries and IncidencePyramidTimeSeries structures
    SusceptiblePyr = new PPTS("Susceptible", 0, (uint)tMax, (uint)pLength, 2, ageBreaks);
    InfectedPyr    = new PPTS("Infected",    0, (uint)tMax, (uint)pLength, 2, ageBreaks);
    RecoveredPyr   = new PPTS("Recovered",   0, (uint)tMax, (uint)pLength, 2, ageBreaks);
    InfectionsPyr  = new IPTS("Infections",  0, (uint)tMax, (uint)pLength, 2, ageBreaks);
    RecoveriesPyr  = new IPTS("Recoveries",  0, (uint)tMax, (uint)pLength, 2, ageBreaks);

    // --- Instantiate statistical distributions ---

    // Distribution on time to recovery following infection
    timeToRecoveryDist = new StatisticalDistributions::Exponential(1/Ɣ);

    // Discrete uniform distribution on age from [ageMin, ageMax]
    ageDist = new StatisticalDistributions::UniformDiscrete(ageMin, ageMax + 1);

    // "Coin-flip" distribution on sex
    sexDist = new StatisticalDistributions::Bernoulli(0.5);

    // Create event queue
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

bool SIRSimulation::IdvIncrement(DayT t, SIRData dtype, Individual idv, int increment) {

    // Potential improvement would be to use function pointers instead of
    //   explicit calls to on each switch
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

        // Should never take this branch because the switch should cover every
        //   member of the SIRData enum
        default:
            return false;
    }
}

EventFunc SIRSimulation::InfectionEvent(int individualIdx) {
    if (individualIdx >= nPeople)
        throw out_of_range("individualIdx >= nPeople");

    printf("Infection: scheduled %d\n", individualIdx);

    EventFunc ef =
      [this,individualIdx](DayT t, SchedulerT Schedule) {

        printf("Infection: infecting %d\n", individualIdx);

        // Grab individual from population to use traits of individual
        Individual idv = Population.at(individualIdx);

        // Decrease susceptible quantity
        IdvIncrement(t, SIRData::Susceptible, idv, -1);

        // Increase infected quantity
        IdvIncrement(t, SIRData::Infected, idv, +1);
        IdvIncrement(t, SIRData::Infections, idv, +1);

        // Create recovery event
        auto recoveryEvent =
          eq->MakeScheduledEvent(t + timeToRecovery(t), RecoveryEvent(individualIdx));

        // Schedule recovery of individual
        Schedule(recoveryEvent);

        // Register individual as Infected
        Population[individualIdx] = changeHealthState(idv, HealthState::Infected);

        // Announce success
        return true;
    };

    return ef;
}

EventFunc SIRSimulation::RecoveryEvent(int individualIdx) {
    if (individualIdx >= nPeople)
        throw out_of_range("individualIdx >= nPeople");

    printf("Recovery: scheduled %d\n", individualIdx);

    EventFunc ef =
      [this, individualIdx](DayT t, SchedulerT Schedule) {

        printf("Recovery: recovered %d\n", individualIdx);

        // Grab individual to take advantage of their characteristics
        Individual idv = Population.at(individualIdx);

        // Reduce the number of Infectives
        IdvIncrement(t, SIRData::Infected, idv, -1);

        // Increase the number of Recovered population members, and increase
        //   the number of recoveries
        IdvIncrement(t, SIRData::Recovered, idv, +1);
        IdvIncrement(t, SIRData::Recoveries, idv, +1);

        // Register the Recovered status of the individual in the Population
        //   vector.
        Population[individualIdx] = changeHealthState(idv, HealthState::Recovered);

        // Announce success
        return true;
    };

    return ef;
}

EventFunc SIRSimulation::FOIUpdateEvent() {
    EventFunc ef =
     [this](DayT t, SchedulerT Schedule) {

        printf("FOI: updating FOI\n");

        // For each individual, schedule infection if timeToInfection(t) < Δt
        int idvIndex = 0;
        DayT ttI;

        // Iterate through each individual
        for (auto individual : Population) {

            // If they are susceptible, and timeToInfection(t) < Δt, schedule
            //   infection
            if (individual.hs == HealthState::Susceptible &&
                (ttI = timeToInfection(t)) < Δt)
                Schedule(eq->MakeScheduledEvent(t + ttI, InfectionEvent(idvIndex)));
            idvIndex += 1;
        }

        // Create next UpdateFOIEvent
        auto UpdateFOIEvent =
          eq->MakeScheduledEvent(t + Δt, FOIUpdateEvent());

        // Schedule next UpdateFOI
        Schedule(UpdateFOIEvent);

        // Announce success
        return true;
    };

    return ef;
}

DayT SIRSimulation::timeToInfection(DayT t) {
    // Needs to be sampled from an exponential distribution

    double forceOfInfection;

    auto I = [this] (DayT t) { return Infected->GetTotalAtTime(t); };
    auto N = [this] (DayT t) { return nPeople; };

    forceOfInfection = λ * (I(t) / N(t));

    // Return sample from distribution
    return (DayT)StatisticalDistributions::Exponential(forceOfInfection) \
             .Sample(*rng);
}

// Right now, actually doesn't depend on 't'.
DayT SIRSimulation::timeToRecovery(DayT t) {
    return (DayT)timeToRecoveryDist->Sample(*rng);
}

bool SIRSimulation::Run(void)
{
    // Create 'nPeople' susceptible individuals and increase the count of
    //   susceptibles
    for (PeopleT i = 0; i < nPeople; i++) {

        auto idv = newIndividual(rng, ageDist, sexDist, HealthState::Susceptible);

        Population.push_back(idv);
        IdvIncrement(0, SIRData::Susceptible, idv, +1);
    }

    printf("Reached\n");

    // Calculate time of first infection, and just after, the first FOIUpdate.
    // We assume that the individual with idx=0 is the first to be infected.
    DayT    timeOfFirstInfection   = 0;
    PeopleT firstInfectiousCaseIdx = 0;
    DayT    timeOfFirstFOI         = timeOfFirstInfection + 0.001;

    auto firstInfection =
      eq->MakeScheduledEvent(timeOfFirstInfection, InfectionEvent(firstInfectiousCaseIdx));
    auto firstFOI =
      eq->MakeScheduledEvent(timeOfFirstFOI, FOIUpdateEvent());


    printf("Reached 2\n");
    // Schedule these first two events
    eq->Schedule(firstInfection);
    eq->Schedule(firstFOI);

        printf("Reached 3\n");

    // While there is an event on the calendar
    while(!eq->Empty()) {

        // Grab the next event
        auto e = eq->Top();

        // Break if reached 'tMax'
        if (e.t >= tMax) {
            printf("Reached tMax\n");
            break;
        }

            printf("Reached 4\n");

        // Run the event
        e.run();

            printf("Reached 5\n");

        // Check that there are any infected people left
        // If not, break
        if (Infected->GetCurrentPrevalence() == 0) {
            printf("No more infected people; simulation done.\n");
            break;
        }

        // Remove event from the queue
        eq->Pop();
    }

    // Close data structures
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

// Specialization for TimeSeries
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

// Specialization for TimeStatistics
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

// Specialization for PyramidTimeSeries
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

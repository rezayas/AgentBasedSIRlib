#include <cstdio>
#include <stdexcept>

#include <EventQueue.h>

#include "../include/SIRlib/SIRlib.h"
#include "../include/SIRlib/Individual.h"

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

SIRSimulation::SIRSimulation(RNG *_rng, double _λ, double _Ɣ, uint _nPeople, \
                             uint _ageMin, uint _ageMax, uint _ageBreak,     \
                             uint _tMax, uint _Δt,                           \
                             uint _pLength)
{
    rng      = _rng;
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
    if (rng == nullptr)
        throw out_of_range("rng was = nullptr");
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
    if (ageBreak >= (ageMax-ageMin))
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

    // Create age breaks from [0, ageMax) every 'ageBreak's
    vector<double> ageBreaks;
    for (int age = ageMin + ageBreak; age < ageMax; age += ageBreak)
        ageBreaks.push_back((double)age);

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

    // Create PyramidData for storing age distribution of infected people
    TotalAgeCounts       = new PyramidData<int>(1, ageBreaks);
    InfectionsAgeCounts  = new PyramidData<int>(1, ageBreaks);
    InfectionsAgePercent = new PyramidData<double>(1, ageBreaks);

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

    delete InfectionsAgeCounts;
    delete InfectionsAgePercent;

    delete timeToRecoveryDist;
    delete ageDist;
    delete sexDist;

    delete eq;
}

bool SIRSimulation::IdvIncrement(DayT t, SIRData dtype, Individual idv, int increment) {
    int floor_t;
    floor_t = (int) t;

    // Potential improvement would be to use function pointers instead of
    //   explicit calls to on each switch
    switch (dtype) {
        case SIRData::Susceptible:
            return SusceptiblePyr->UpdateByAge(floor_t, sexN(idv.sex), idv.age, increment)
                && Susceptible->Record(floor_t, increment);

        case SIRData::Infected:
            return InfectedPyr->UpdateByAge(floor_t, sexN(idv.sex), idv.age, increment)
                && Infected->Record(floor_t, increment);

        case SIRData::Recovered:
            return RecoveredPyr->UpdateByAge(floor_t, sexN(idv.sex), idv.age, increment)
                && Recovered->Record(floor_t, increment);

        case SIRData::Infections:
            return InfectionsPyr->UpdateByAge(floor_t, sexN(idv.sex), idv.age, increment)
                && Infections->Record(floor_t, increment)
                && InfectionsAgeCounts->UpdateByAge(0, idv.age, increment);
                // ^ Increase number of infections in age group to calculate
                //     percentages later on

        case SIRData::Recoveries:
            return RecoveriesPyr->UpdateByAge(floor_t, sexN(idv.sex), idv.age, increment)
                && Recoveries->Record(floor_t, increment);

        // Should never take this branch because the switch should cover every
        //   member of the SIRData enum
        default:
            return false;
    }
}

EventFunc SIRSimulation::InfectionEvent(int individualIdx) {
    if (individualIdx >= nPeople)
        throw out_of_range("individualIdx >= nPeople");

    // printf("\t[unk] Infection: scheduled %d\n", individualIdx);

    EventFunc ef =
      [this,individualIdx](DayT t, SchedulerT Schedule) {

        // printf("[%f] Infection: infecting %d\n", t, individualIdx);

        // Grab individual from population to use traits of individual
        Individual idv = Population.at(individualIdx);

        // Decrease susceptible quantity, increase infected quantity
        IdvIncrement(t, SIRData::Susceptible, idv, -1);
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

    // printf("\t[unk] Recovery: scheduled %d\n", individualIdx);

    EventFunc ef =
      [this, individualIdx](DayT t, SchedulerT Schedule) {

        // printf("[%f] Recovery: recovered %d\n", t, individualIdx);

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

    auto N = [this] (DayT t) -> double { return nPeople; };

    forceOfInfection = λ * ((*Infected)(t) / N(t));

    // Return sample from distribution
    return (DayT)StatisticalDistributions::Exponential(forceOfInfection) \
             .Sample(*rng);
}

// Right now, actually doesn't depend on 't'.
DayT SIRSimulation::timeToRecovery(DayT t) {
    return (DayT)timeToRecoveryDist->Sample(*rng);
}

void SIRSimulation::CalculateInfectionAgePercent(void) {
    int nAgeBreaks;
    nAgeBreaks = (int) ceil((double)(ageMax-ageMin)/(double)ageBreak);

    for (int i = 0; i < nAgeBreaks; i++) {
        double percent = (double) InfectionsAgeCounts->GetTotalInAgeGroupAndCategory(i,0) / \
                         (double) TotalAgeCounts->GetTotalInAgeGroupAndCategory(i,0);
        InfectionsAgePercent->UpdateByIdx(0, i, percent);
    }
    return;
}

bool SIRSimulation::Run(void)
{
    // Create 'nPeople' susceptible individuals and increase the count of
    //   susceptibles
    for (PeopleT i = 0; i < nPeople; i++) {

        auto idv = newIndividual(rng, ageDist, sexDist, HealthState::Susceptible);

        Population.push_back(idv);
        IdvIncrement(0, SIRData::Susceptible, idv, +1);

        // Add person to total age count
        TotalAgeCounts->UpdateByAge(0, idv.age, +1);
    }


    // Calculate time of first infection, and just after, the first FOIUpdate.
    // We assume that the individual with idx=0 is the first to be infected.
    DayT    timeOfFirstInfection   = 0;
    PeopleT firstInfectiousCaseIdx = 0;
    DayT    timeOfFirstFOI         = timeOfFirstInfection + 0.001;

    auto firstInfection =
      eq->MakeScheduledEvent(timeOfFirstInfection, InfectionEvent(firstInfectiousCaseIdx));
    auto firstFOI =
      eq->MakeScheduledEvent(timeOfFirstFOI, FOIUpdateEvent());


    // Schedule these first two events
    eq->Schedule(firstInfection);
    eq->Schedule(firstFOI);


    // While there is an event on the calendar
    while(!eq->Empty()) {

        // Grab the next event
        auto e = eq->Top();

        // Break if reached 'tMax'
        if (e.t >= tMax)
            break;

        // Run the event
        e.run();

        // Check that there are any infected people left
        // If not, break
        if (Infected->GetCurrentPrevalence() == 0)
            break;

        // Remove event from the queue
        eq->Pop();
    }

    // Calculate the percent of age groups that were infected
    CalculateInfectionAgePercent();

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
TS *SIRSimulation::GetData<TS>(SIRData field)
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

// Specialization for TimeStatistics
template <>
TSx *SIRSimulation::GetData<TSx>(SIRData field)
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

// Specialization for PyramidTimeSeries
template <>
PyTS *SIRSimulation::GetData<PyTS>(SIRData field)
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

// Specialization for PyramidData
template <>
PyramidData<double> *SIRSimulation::GetData<PyramidData<double>>(SIRData field)
{
    switch(field) {
        case SIRData::Susceptible: return nullptr;
        case SIRData::Infected:    return nullptr;
        case SIRData::Recovered:   return nullptr;
        case SIRData::Infections:  return InfectionsAgePercent;
        case SIRData::Recoveries:  return nullptr;
        default:                   return nullptr;
    }
}

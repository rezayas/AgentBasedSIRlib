#include "SIRSimRunner.h"

SIRSimRunner::SIRSimRunner(string _fileName, int _nTrajectories, double _lambda, double _gamma,   \
               long _nPeople, unsigned int _ageMin, unsigned int _ageMax,    \
               unsigned int _ageBreak, unsigned int _tMax, unsigned int _deltaT, \
               unsigned int _pLength)
{
    fileName      = _fileName;
    nTrajectories = _nTrajectories;
    lambda             = _lambda;
    gamma             = _gamma;
    nPeople       = _nPeople;
    ageMin        = _ageMin;
    ageMax        = _ageMax;
    ageBreak      = _ageBreak;
    tMax          = _tMax;
    deltaT            = _deltaT;
    pLength       = _pLength;

    if (nTrajectories < 1)
        throw out_of_range("nTrajectories < 1");
}

SIRSimRunner::~SIRSimRunner(void) {
    delete [] SIRsims;
}

using RunType = SIRSimRunner::RunType;

template<>
bool SIRSimRunner::Run<RunType::Serial>(void) {
    bool succ = true;

    RNG  *masterRNG   = new RNG(42);
    RNG **servantRNGs = new RNG *[nTrajectories];
    for (int i = 0; i < nTrajectories; ++i)
        servantRNGs[i] = new RNG( masterRNG->mt_() );

    // Allocate array of SIRSimulation pointers, then instantiate SIRSimulations
    SIRsims = new SIRSimulation *[nTrajectories];
    for (int i = 0; i < nTrajectories; ++i)
        SIRsims[i] = new SIRSimulation(servantRNGs[i], lambda, gamma, nPeople, ageMin, ageMax, ageBreak, tMax, deltaT, pLength);

    // Run each SIRSimulation
    for (int i = 0; i < nTrajectories; ++i)
        succ &= SIRsims[i]->Run();

    // Note: freeing the RNGs means that Run() cannot be called again!
    delete masterRNG;
    delete [] servantRNGs;

    return succ;
}

template<>
bool SIRSimRunner::Run<RunType::Parallel>(void) {
    bool succ = true;

    // Create futures
    future<bool> *futures = new future<bool>[nTrajectories];

    // Create master RNG and seed servant RNGs with values from master RNG.
    RNG  *masterRNG   = new RNG(time(NULL));
    RNG **servantRNGs = new RNG *[nTrajectories];
    for (int i = 0; i < nTrajectories; ++i)
        servantRNGs[i] = new RNG( masterRNG->mt_() );

    // Allocate array of SIRSimulation pointers, then instantiate SIRSimulations
    SIRsims = new SIRSimulation *[nTrajectories];
    for (int i = 0; i < nTrajectories; ++i)
        SIRsims[i] = new SIRSimulation(servantRNGs[i], lambda, gamma, nPeople, ageMin, ageMax, ageBreak, tMax, deltaT, pLength);

    // Run each SIRSimulation
    for (int i = 0; i < nTrajectories; ++i)
        futures[i] = async(launch::async | launch::deferred, \
                           &SIRSimulation::Run,              \
                           SIRsims[i]);

    // Wait for all tasks to finish running and detect errors (barrier)
    for (int i = 0; i < nTrajectories; ++i)
        succ &= futures[i].get();

    // Note: freeing the RNGs means that Run() cannot be called again!
    delete    masterRNG;
    delete [] servantRNGs;
    delete [] futures;

    return succ;
}

std::vector<SIRTrajectoryResult>
SIRSimRunner::GetTrajectoryResults(void)
{
    std::vector<SIRTrajectoryResult> res;
    for (size_t i = 0; i < nTrajectories; ++i)
        res.push_back( getTrajectoryResult(i) );

    return res;
}

SIRTrajectoryResult
SIRSimRunner::GetTrajectoryResult(size_t i)
{
    if (i >= nTrajectories)
        throw std::out_of_range("i was too small or too big");

    return getTrajectoryResult(i);
}


std::vector<string> SIRSimRunner::Write(void) {
    bool succ = true;

    map<TimeStatType, string> columns {
        {TimeStatType::Sum,  "Total"},
        {TimeStatType::Mean, "Average"},
        {TimeStatType::Min,  "Minimum"},
        {TimeStatType::Max,  "Maximum"}
    };

    TimeSeriesExport<int> TSExSusceptible(fileName + string("-susceptible.csv"));
    TimeSeriesExport<int> TSExInfected   (fileName + string("-infected.csv"));
    TimeSeriesExport<int> TSExRecovered  (fileName + string("-recovered.csv"));
    TimeSeriesExport<int> TSExInfections (fileName + string("-infections.csv"));
    TimeSeriesExport<int> TSExRecoveries (fileName + string("-recoveries.csv"));

    TimeStatisticsExport TSxExSusceptible(fileName + string("-susceptible-statistics.csv"), columns);
    TimeStatisticsExport TSxExInfected   (fileName + string("-infected-statistics.csv"), columns);
    TimeStatisticsExport TSxExRecovered  (fileName + string("-recovered-statistics.csv"), columns);
    TimeStatisticsExport TSxExInfections (fileName + string("-infections-statistics.csv"), columns);
    TimeStatisticsExport TSxExRecoveries (fileName + string("-recoveries-statistics.csv"), columns);

    PyramidTimeSeriesExport PTSExSusceptible(fileName + string("-susceptible-pyramid.csv"));
    PyramidTimeSeriesExport PTSExInfected   (fileName + string("-infected-pyramid.csv"));
    PyramidTimeSeriesExport PTSExRecovered  (fileName + string("-recovered-pyramid.csv"));
    PyramidTimeSeriesExport PTSExInfections (fileName + string("-infections-pyramid.csv"));
    PyramidTimeSeriesExport PTSExRecoveries (fileName + string("-recoveries-pyramid.csv"));

    PyramidDataExport<double> PDExCaseProfile(fileName + string("-cases-by-age.csv"));

    std::vector<string> writes {
        fileName + string("-susceptible.csv"),
        fileName + string("-infected.csv"),
        fileName + string("-recovered.csv"),
        fileName + string("-infections.csv"),
        fileName + string("-recoveries.csv"),
        fileName + string("-susceptible-statistics.csv"),
        fileName + string("-infected-statistics.csv"),
        fileName + string("-recovered-statistics.csv"),
        fileName + string("-infections-statistics.csv"),
        fileName + string("-recoveries-statistics.csv"),
        fileName + string("-susceptible-pyramid.csv"),
        fileName + string("-infected-pyramid.csv"),
        fileName + string("-recovered-pyramid.csv"),
        fileName + string("-infections-pyramid.csv"),
        fileName + string("-recoveries-pyramid.csv"),
        fileName + string("-cases-by-age.csv")
    };

    // For each SIRSimulation, add its data to our exporters
    for (int i = 0; i < nTrajectories; ++i)
    {
        TimeSeries<int>     *Susceptible    = SIRsims[i]->GetData<TimeSeries<int>>(SIRData::Susceptible);
        TimeSeries<int>     *Infected       = SIRsims[i]->GetData<TimeSeries<int>>(SIRData::Infected);
        TimeSeries<int>     *Recovered      = SIRsims[i]->GetData<TimeSeries<int>>(SIRData::Recovered);
        TimeSeries<int>     *Infections     = SIRsims[i]->GetData<TimeSeries<int>>(SIRData::Infections);
        TimeSeries<int>     *Recoveries     = SIRsims[i]->GetData<TimeSeries<int>>(SIRData::Recoveries);

        TimeStatistic       *SusceptibleSx  = SIRsims[i]->GetData<TimeStatistic>(SIRData::Susceptible);
        TimeStatistic       *InfectedSx     = SIRsims[i]->GetData<TimeStatistic>(SIRData::Infected);
        TimeStatistic       *RecoveredSx    = SIRsims[i]->GetData<TimeStatistic>(SIRData::Recovered);
        TimeStatistic       *InfectionsSx   = SIRsims[i]->GetData<TimeStatistic>(SIRData::Infections);
        TimeStatistic       *RecoveriesSx   = SIRsims[i]->GetData<TimeStatistic>(SIRData::Recoveries);

        PyramidTimeSeries   *SusceptiblePyr = SIRsims[i]->GetData<PyramidTimeSeries>(SIRData::Susceptible);
        PyramidTimeSeries   *InfectedPyr    = SIRsims[i]->GetData<PyramidTimeSeries>(SIRData::Infected);
        PyramidTimeSeries   *RecoveredPyr   = SIRsims[i]->GetData<PyramidTimeSeries>(SIRData::Recovered);
        PyramidTimeSeries   *InfectionsPyr  = SIRsims[i]->GetData<PyramidTimeSeries>(SIRData::Infections);
        PyramidTimeSeries   *RecoveriesPyr  = SIRsims[i]->GetData<PyramidTimeSeries>(SIRData::Recoveries);

        PyramidData<double> *CaseProfile    = SIRsims[i]->GetData<PyramidData<double>>(SIRData::Infections);

        // Add
        succ &= TSExSusceptible.Add(Susceptible);
        succ &= TSExInfected.Add(Infected);
        succ &= TSExRecovered.Add(Recovered);
        succ &= TSExInfections.Add(Infections);
        succ &= TSExRecoveries.Add(Recoveries);

        succ &= TSxExSusceptible.Add(SusceptibleSx);
        succ &= TSxExInfected.Add(InfectedSx);
        succ &= TSxExRecovered.Add(RecoveredSx);
        succ &= TSxExInfections.Add(InfectionsSx);
        succ &= TSxExRecoveries.Add(RecoveriesSx);

        succ &= PTSExSusceptible.Add(SusceptiblePyr);
        succ &= PTSExInfected.Add(InfectedPyr);
        succ &= PTSExRecovered.Add(RecoveredPyr);
        succ &= PTSExInfections.Add(InfectionsPyr);
        succ &= PTSExRecoveries.Add(RecoveriesPyr);

        succ &= PDExCaseProfile.Add(CaseProfile);
    }

    // Write
    succ &= TSExSusceptible.Write();
    succ &= TSExInfected.Write();
    succ &= TSExRecovered.Write();
    succ &= TSExInfections.Write();
    succ &= TSExRecoveries.Write();

    succ &= TSxExSusceptible.Write();
    succ &= TSxExInfected.Write();
    succ &= TSxExRecovered.Write();
    succ &= TSxExInfections.Write();
    succ &= TSxExRecoveries.Write();

    succ &= PTSExSusceptible.Write();
    succ &= PTSExInfected.Write();
    succ &= PTSExRecovered.Write();
    succ &= PTSExInfections.Write();
    succ &= PTSExRecoveries.Write();

    succ &= PDExCaseProfile.Write();

    printf("Finished writing\n");

    return succ ? writes : std::vector<std::string>{};
}


SIRTrajectoryResult SIRSimRunner::getTrajectoryResult(size_t i)
{
    struct SIRTrajectoryResult res;

    res.Susceptible    = SIRsims[i]->GetData<PrevalenceTimeSeries<int>>(SIRData::Susceptible);
    res.Infected       = SIRsims[i]->GetData<PrevalenceTimeSeries<int>>(SIRData::Infected);
    res.Recovered      = SIRsims[i]->GetData<PrevalenceTimeSeries<int>>(SIRData::Recovered);

    res.Infections     = SIRsims[i]->GetData<IncidenceTimeSeries<int>>(SIRData::Infections);
    res.Recoveries     = SIRsims[i]->GetData<IncidenceTimeSeries<int>>(SIRData::Recoveries);

    res.SusceptibleSx  = SIRsims[i]->GetData<TimeStatistic>(SIRData::Susceptible);
    res.InfectedSx     = SIRsims[i]->GetData<TimeStatistic>(SIRData::Infected);
    res.RecoveredSx    = SIRsims[i]->GetData<TimeStatistic>(SIRData::Recovered);
    res.InfectionsSx   = SIRsims[i]->GetData<TimeStatistic>(SIRData::Infections);
    res.RecoveriesSx   = SIRsims[i]->GetData<TimeStatistic>(SIRData::Recoveries);

    res.SusceptiblePyr = SIRsims[i]->GetData<PyramidTimeSeries>(SIRData::Susceptible);
    res.InfectedPyr    = SIRsims[i]->GetData<PyramidTimeSeries>(SIRData::Infected);
    res.RecoveredPyr   = SIRsims[i]->GetData<PyramidTimeSeries>(SIRData::Recovered);
    res.InfectionsPyr  = SIRsims[i]->GetData<PyramidTimeSeries>(SIRData::Infections);
    res.RecoveriesPyr  = SIRsims[i]->GetData<PyramidTimeSeries>(SIRData::Recoveries);

    res.CaseProfile    = SIRsims[i]->GetData<PyramidData<double>>(SIRData::Infections);

    return res;
}

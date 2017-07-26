#include <string>
#include <cstdlib>

#include <SIRlib.h>
#include <CSVExport.h>
#include <PyramidTimeSeries.h>
#include <TimeStatistic.h>
#include <TimeSeries.h>
#include <RNG.h>

using namespace std;
using namespace SIRlib;

using uint = unsigned int;

// Parameters:
// 1: fileName:
//      Prefix of .csv file name (do not specify extension). Three files will
//        be created: [fileName]-births.csv, [fileName]-deaths.csv,
//        [filename]-population.csv
// 2: nTrajectories:
//      number of trajectories to run under the following parameters:
// 3: λ:
//      transmission parameter (unitless)
// 4: Ɣ:
//      duration of infectiousness, in years
// 5: nPeople:
//      number of people in the population
// 6: ageMin:
//      minimum age of an individual
// 7: ageMax:
//      maximum age of an individual
// 8: ageBreak:
//      interval between age breaks of population
// 9: tMax:
//      maximum length of time to run simulation to
// 10: dt:
//      timestep
// 11: pLength:
//      length of one data-aggregation period
int main(int argc, char const *argv[])
{
    int i;
    string fileName;
    int nTrajectories;
    RNG *rng;
    double λ, Ɣ;
    long nPeople;
    uint ageMin, ageMax, ageBreak, tMax, Δt, pLength;

    SIRSimulation *SIRsim;

    if (argc < 12) {
        printf("Error: too few arguments\n");
        exit(1);
    }

    i = 0;
    fileName      = string(argv[++i]);
    nTrajectories = atoi(argv[++i]);
    rng           = new RNG(time(NULL));
    λ             = stof(argv[++i], NULL);
    Ɣ             = stof(argv[++i], NULL);
    nPeople       = atol(argv[++i]);
    ageMin        = atoi(argv[++i]);
    ageMax        = atoi(argv[++i]);
    ageBreak      = atoi(argv[++i]);
    tMax          = atoi(argv[++i]);
    Δt            = atoi(argv[++i]);
    pLength       = atoi(argv[++i]);


    printf("Args:\n\tfileName=%s\n\tλ=%4.4f\n\tƔ=%4.4f\n\tnPeople=%ld\n\tageMin=%d\n\tageMax=%d\n\tageBreak=%d\n\ttMax=%d\n\tΔt=%d\n\tpLength=%d\n\n", fileName.c_str(), λ, Ɣ, nPeople, ageMin, ageMax, ageBreak, tMax, Δt, pLength);

    SIRsim = new SIRSimulation(rng, λ, Ɣ, nPeople, ageMin, ageMax, ageBreak, tMax, Δt, pLength);
    SIRsim->Run();

    map<TimeStatType, string> columns {
            {TimeStatType::Sum,  "Total"},
            {TimeStatType::Mean, "Average"},
            {TimeStatType::Min,  "Minimum"},
            {TimeStatType::Max,  "Maximum"}
    };

    TimeSeriesCSVExport<int> TSExSusceptible(fileName + string("-susceptible.csv"));
    TimeSeriesCSVExport<int> TSExInfected   (fileName + string("-infected.csv"));
    TimeSeriesCSVExport<int> TSExRecovered  (fileName + string("-recovered.csv"));
    TimeSeriesCSVExport<int> TSExInfections (fileName + string("-infections.csv"));
    TimeSeriesCSVExport<int> TSExRecoveries (fileName + string("-recoveries.csv"));

    TimeStatisticsCSVExport TSxExSusceptible(fileName + string("-susceptible-statistics.csv"), columns);
    TimeStatisticsCSVExport TSxExInfected   (fileName + string("-infected-statistics.csv"), columns);
    TimeStatisticsCSVExport TSxExRecovered  (fileName + string("-recovered-statistics.csv"), columns);
    TimeStatisticsCSVExport TSxExInfections (fileName + string("-infections-statistics.csv"), columns);
    TimeStatisticsCSVExport TSxExRecoveries (fileName + string("-recoveries-statistics.csv"), columns);

    PyramidTimeSeriesCSVExport PTSExSusceptible(fileName + string("-susceptible-pyramid.csv"));
    PyramidTimeSeriesCSVExport PTSExInfected   (fileName + string("-infected-pyramid.csv"));
    PyramidTimeSeriesCSVExport PTSExRecovered  (fileName + string("-recovered-pyramid.csv"));
    PyramidTimeSeriesCSVExport PTSExInfections (fileName + string("-infections-pyramid.csv"));
    PyramidTimeSeriesCSVExport PTSExRecoveries (fileName + string("-recoveries-pyramid.csv"));

    shared_ptr<TimeSeries<int>>   Susceptible    = SIRsim->GetData<TimeSeries<int>>(SIRData::Susceptible);
    shared_ptr<TimeSeries<int>>   Infected       = SIRsim->GetData<TimeSeries<int>>(SIRData::Infected);
    shared_ptr<TimeSeries<int>>   Recovered      = SIRsim->GetData<TimeSeries<int>>(SIRData::Recovered);
    shared_ptr<TimeSeries<int>>   Infections     = SIRsim->GetData<TimeSeries<int>>(SIRData::Infections);
    shared_ptr<TimeSeries<int>>   Recoveries     = SIRsim->GetData<TimeSeries<int>>(SIRData::Recoveries);

    shared_ptr<TimeStatistic>     SusceptibleSx  = SIRsim->GetData<TimeStatistic>(SIRData::Susceptible);
    shared_ptr<TimeStatistic>     InfectedSx     = SIRsim->GetData<TimeStatistic>(SIRData::Infected);
    shared_ptr<TimeStatistic>     RecoveredSx    = SIRsim->GetData<TimeStatistic>(SIRData::Recovered);
    shared_ptr<TimeStatistic>     InfectionsSx   = SIRsim->GetData<TimeStatistic>(SIRData::Infections);
    shared_ptr<TimeStatistic>     RecoveriesSx   = SIRsim->GetData<TimeStatistic>(SIRData::Recoveries);

    shared_ptr<PyramidTimeSeries> SusceptiblePyr = SIRsim->GetData<PyramidTimeSeries>(SIRData::Susceptible);
    shared_ptr<PyramidTimeSeries> InfectedPyr    = SIRsim->GetData<PyramidTimeSeries>(SIRData::Infected);
    shared_ptr<PyramidTimeSeries> RecoveredPyr   = SIRsim->GetData<PyramidTimeSeries>(SIRData::Recovered);
    shared_ptr<PyramidTimeSeries> InfectionsPyr  = SIRsim->GetData<PyramidTimeSeries>(SIRData::Infections);
    shared_ptr<PyramidTimeSeries> RecoveriesPyr  = SIRsim->GetData<PyramidTimeSeries>(SIRData::Recoveries);

    // Add
    TSExSusceptible.Add(Susceptible.get());
    TSExInfected.Add(Infected.get());
    TSExRecovered.Add(Recovered.get());
    TSExInfections.Add(Infections.get());
    TSExRecoveries.Add(Recoveries.get());

    TSxExSusceptible.Add(SusceptibleSx.get());
    TSxExInfected.Add(InfectedSx.get());
    TSxExRecovered.Add(RecoveredSx.get());
    TSxExInfections.Add(InfectionsSx.get());
    TSxExRecoveries.Add(RecoveriesSx.get());

    PTSExSusceptible.Add(SusceptiblePyr.get());
    PTSExInfected.Add(InfectedPyr.get());
    PTSExRecovered.Add(RecoveredPyr.get());
    PTSExInfections.Add(InfectionsPyr.get());
    PTSExRecoveries.Add(RecoveriesPyr.get());

    // Write
    TSExSusceptible.Write();
    TSExInfected.Write();
    TSExRecovered.Write();
    TSExInfections.Write();
    TSExRecoveries.Write();

    TSxExSusceptible.Write();
    TSxExInfected.Write();
    TSxExRecovered.Write();
    TSxExInfections.Write();
    TSxExRecoveries.Write();

    PTSExSusceptible.Write();
    PTSExInfected.Write();
    PTSExRecovered.Write();
    PTSExInfections.Write();
    PTSExRecoveries.Write();

    printf("Finished writing\n");


    delete SIRsim;
    return 0;
}

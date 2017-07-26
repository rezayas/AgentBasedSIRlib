#include <string>
#include <cstdlib>
#include <stdexcept>

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
// 3. λ:
//      transmission parameter (double | > 0) unit: [cases/day]
// 4. Ɣ:
//      duration of infectiousness. (double | > 0) double, unit: [day]
// 5. nPeople:
//      number of people in the population (uint | > 0)
// 6. ageMin:
//      minimum age of an individual (uint) unit: [years]
// 7. ageMax:
//      maximum age of an individual (uint | >= ageMin) unit: [years]
// 8. ageBreak:
//      interval between age breaks of population (uint | > 1, < (ageMax - ageMin)) unit: [years]
// 9. tMax:
//      maximum length of time to run simulation to (uint | >= 1) unit: [days]
//10. Δt:
//      timestep (uint | >= 1, <= tMax) unit: [days]
//11. pLength:
//      length of one data-aggregation period (uint | > 0, < tMax) unit: [days]
int main(int argc, char const *argv[])
{
    int i;
    string fileName;
    int nTrajectories;
    RNG *rng;
    double λ, Ɣ;
    long nPeople;
    uint ageMin, ageMax, ageBreak, tMax, Δt, pLength;

    SIRSimulation **SIRsims;

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

    printf("Args:\n\tfileName=%s\n\tnTrajectories=%d\n\tλ=%4.4f\n\tƔ=%4.4f\n\tnPeople=%ld\n\tageMin=%d\n\tageMax=%d\n\tageBreak=%d\n\ttMax=%d\n\tΔt=%d\n\tpLength=%d\n\n", fileName.c_str(), nTrajectories, λ, Ɣ, nPeople, ageMin, ageMax, ageBreak, tMax, Δt, pLength);

    if (nTrajectories < 1)
        throw out_of_range("nTrajectories < 1");

    // Allocate array of SIRSimulation pointers, then instantiate SIRSimulations
    SIRsims = new SIRSimulation *[nTrajectories];
    for (int i = 0; i < nTrajectories; ++i)
        SIRsims[i] = new SIRSimulation(rng, λ, Ɣ, nPeople, ageMin, ageMax, ageBreak, tMax, Δt, pLength);

    // Run each SIRSimulation
    for (int i = 0; i < nTrajectories; ++i) {
        printf("=== Simulation %d ===\n\n\n", i);
        SIRsims[i]->Run();
        printf("\n\n\n\n\n\n");
    }

    // For each SIRSimulation, add its data to our exporters
    for (int i = 0; i < nTrajectories; ++i)
    {
        TimeSeries<int>   *Susceptible    = SIRsims[i]->GetData<TimeSeries<int>>(SIRData::Susceptible);
        TimeSeries<int>   *Infected       = SIRsims[i]->GetData<TimeSeries<int>>(SIRData::Infected);
        TimeSeries<int>   *Recovered      = SIRsims[i]->GetData<TimeSeries<int>>(SIRData::Recovered);
        TimeSeries<int>   *Infections     = SIRsims[i]->GetData<TimeSeries<int>>(SIRData::Infections);
        TimeSeries<int>   *Recoveries     = SIRsims[i]->GetData<TimeSeries<int>>(SIRData::Recoveries);

        TimeStatistic     *SusceptibleSx  = SIRsims[i]->GetData<TimeStatistic>(SIRData::Susceptible);
        TimeStatistic     *InfectedSx     = SIRsims[i]->GetData<TimeStatistic>(SIRData::Infected);
        TimeStatistic     *RecoveredSx    = SIRsims[i]->GetData<TimeStatistic>(SIRData::Recovered);
        TimeStatistic     *InfectionsSx   = SIRsims[i]->GetData<TimeStatistic>(SIRData::Infections);
        TimeStatistic     *RecoveriesSx   = SIRsims[i]->GetData<TimeStatistic>(SIRData::Recoveries);

        PyramidTimeSeries *SusceptiblePyr = SIRsims[i]->GetData<PyramidTimeSeries>(SIRData::Susceptible);
        PyramidTimeSeries *InfectedPyr    = SIRsims[i]->GetData<PyramidTimeSeries>(SIRData::Infected);
        PyramidTimeSeries *RecoveredPyr   = SIRsims[i]->GetData<PyramidTimeSeries>(SIRData::Recovered);
        PyramidTimeSeries *InfectionsPyr  = SIRsims[i]->GetData<PyramidTimeSeries>(SIRData::Infections);
        PyramidTimeSeries *RecoveriesPyr  = SIRsims[i]->GetData<PyramidTimeSeries>(SIRData::Recoveries);

        // Add
        TSExSusceptible.Add(Susceptible);
        TSExInfected.Add(Infected);
        TSExRecovered.Add(Recovered);
        TSExInfections.Add(Infections);
        TSExRecoveries.Add(Recoveries);

        TSxExSusceptible.Add(SusceptibleSx);
        TSxExInfected.Add(InfectedSx);
        TSxExRecovered.Add(RecoveredSx);
        TSxExInfections.Add(InfectionsSx);
        TSxExRecoveries.Add(RecoveriesSx);

        PTSExSusceptible.Add(SusceptiblePyr);
        PTSExInfected.Add(InfectedPyr);
        PTSExRecovered.Add(RecoveredPyr);
        PTSExInfections.Add(InfectionsPyr);
        PTSExRecoveries.Add(RecoveriesPyr);
    }

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

    delete [] SIRsims;
    return 0;
}

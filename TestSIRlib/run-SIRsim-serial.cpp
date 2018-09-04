#include <string>
#include <cstdlib>
#include <stdexcept>

#include <SIRlib.h>
#include <CSVExport.h>
#include <PyramidTimeSeries.h>
#include <TimeStatistic.h>
#include <TimeSeries.h>
#include <RNG.h>

#include "SIRSimRunner.h"

using namespace std;
using namespace SIRlib;

using uint = unsigned int;

// exponential distribution vs SIR model
// ./executableName output01 10 20 25 10000 0 90 10 3650 1 30

// Parameters:
// 1: fileName:
//      Prefix of .csv file name (do not specify extension). Three files will
//        be created: [fileName]-births.csv, [fileName]-deaths.csv,
//        [filename]-population.csv
// 2: nTrajectories:
//      number of trajectories to run under the following parameters:
// 3. lambda:
//      transmission parameter (double | > 0) unit: [cases/day]
// 4. gamma:
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
//10. deltaT:
//      timestep (uint | >= 1, <= tMax) unit: [days]
//11. pLength:
//      length of one data-aggregation period (uint | > 0, < tMax) unit: [days]

using RunType = SIRSimRunner::RunType;

int main(int argc, char const *argv[])
{
    bool succ = true;

    int i;
    string fileName;
    int nTrajectories;
    double lambda, gamma;
    long nPeople;
    uint ageMin, ageMax, ageBreak, tMax, deltaT, pLength;

    if (argc < 12) {
        printf("Error: too few arguments\n");
        exit(1);
    }

    i = 0;
    fileName      = string(argv[++i]);
    nTrajectories = atoi(argv[++i]);
    lambda             = stof(argv[++i], NULL);
    gamma             = stof(argv[++i], NULL);
    nPeople       = atol(argv[++i]);
    ageMin        = atoi(argv[++i]);
    ageMax        = atoi(argv[++i]);
    ageBreak      = atoi(argv[++i]);
    tMax          = atoi(argv[++i]);
    deltaT            = atoi(argv[++i]);
    pLength       = atoi(argv[++i]);

    // Initialize simulation
    SIRSimRunner sim(fileName, nTrajectories, lambda, gamma, nPeople, ageMin, ageMax, \
                     ageBreak, tMax, deltaT, pLength);

    // Run simulation
    succ &= sim.Run<RunType::Serial>();

    // Write simulation results
    succ &= !sim.Write().empty();

    if (succ)
        printf("Simulation finished successfully\n");
    else
        printf("Simulation finished unsuccessfully!\n");

    return 0;
}

#include <string>
#include <cstdlib>
#include <stdexcept>

#include <SIRlib.h>
#include <CSVExport.h>
#include <PyramidTimeSeries.h>
#include <TimeStatistic.h>
#include <TimeSeries.h>
#include <RNG.h>
#include <Normal.h>

#include <Bound.h>
#include <Calibrate.h>
#include <PolyRegCal.hpp>

#include "SIRSimRunner.h"

using namespace SIRlib;
using namespace ComputationalLib;
using namespace SimulationLib;

using uint = unsigned int;

// Parameters:
// 1: fileName:
//      Prefix of .csv file name (do not specify extension). Three files will
//        be created: [fileName]-births.csv, [fileName]-deaths.csv,
//        [filename]-population.csv
// 2. lambda:
//      transmission parameter (double | > 0) unit: [cases/day]
// 3. gamma:
//      duration of infectiousness. (double | > 0) double, unit: [day]
// 4. nPeople:
//      number of people in the population (uint | > 0)
// 5. ageMin:
//      minimum age of an individual (uint) unit: [years]
// 6. ageMax:
//      maximum age of an individual (uint | >= ageMin) unit: [years]
// 7. ageBreak:
//      interval between age breaks of population (uint | > 1, < (ageMax - ageMin)) unit: [years]
// 8. tMax:
//      maximum length of time to run simulation to (uint | >= 1) unit: [days]
// 9. deltaT:
//      timestep (uint | >= 1, <= tMax) unit: [days]
//10. pLength:
//      length of one data-aggregation period (uint | > 0, < tMax) unit: [days]
using RunType = SIRSimRunner::RunType;

int main(int argc, char const *argv[])
{
    using Params = std::vector<SimulationLib::TimeSeries<int>::query_type>;
    if (argc < 10) {
        printf("Error: too few arguments\n");
        exit(1);
    }

    // Grab params from command line ////////////////////
    size_t i {0};
    auto fileName      = string(argv[++i]);
    auto lambda             = (double)stof(argv[++i], NULL);
    auto gamma             = (double)stof(argv[++i], NULL);
    auto nPeople       = atol(argv[++i]);
    auto ageMin        = atoi(argv[++i]);
    auto ageMax        = atoi(argv[++i]);
    auto ageBreak      = atoi(argv[++i]);
    auto tMax          = atoi(argv[++i]);
    auto deltaT            = atoi(argv[++i]);
    auto pLength       = atoi(argv[++i]);
    // End grab from command line ////////////////////////


    // Create historical data to compare against model data
    auto InfectedData = new PrevalenceTimeSeries<int>("Historical data", tMax, pLength, 1, nullptr);
    InfectedData->Record(0.0, 1);
    InfectedData->Record(1.0, 2);
    InfectedData->Record(2.0, 8);
    InfectedData->Record(3.0, 3);
    InfectedData->Record(4.0, 2);

    // Calculate likelihood on t=0,1,2,3,4
    Params Ps {std::make_tuple((double)0),
               std::make_tuple((double)1),
               std::make_tuple((double)2),
               std::make_tuple((double)3),
               std::make_tuple((double)4)};

    // Create a normal distribution with stDev=1 for each model point
    using DG = std::function<StatisticalDistributions::Normal(double,double)>;
    DG DistributionGenerator = [] (auto v, auto t) -> StatisticalDistributions::Normal {
        double standardDeviation {1.0};
        return StatisticalDistributions::Normal(v, standardDeviation);
    };

    // Create a lambda function for use with PolyRegCal routine. The
    // only parameters which can be varied by the iterative method
    // are lambda and gamma.
    using F = std::function<double(double,double)>;
    F f = [&] (double lambda, double gamma) -> double {
        bool succ {true};
        
        auto S = SIRSimRunner(fileName, 1, lambda, gamma, nPeople, ageMin, \
                              ageMax, ageBreak, tMax, deltaT, pLength);
        
        succ &= S.Run<RunType::Serial>();
        S.Write();

        auto Data = S.GetTrajectoryResult(0);
        auto InfectedModel = Data.Infected;
        auto Likelihood = CalculateLikelihood(*InfectedModel, *InfectedData, Ps, DistributionGenerator);

        return Likelihood;
    };

    using rNearlyZero  = std::ratio<1,100000>;
    using r10    = std::ratio<10,1>;
    using r100   = std::ratio<100,1>;
    using ZeroTo10 = Bound<double, rNearlyZero, r100>;
    using ZeroTo100 = Bound<double, rNearlyZero, r10>;
    using Xs = std::tuple<ZeroTo10, ZeroTo100>;

    Xs init {lambda, gamma};

    auto CalibrationResult = PolyRegCal(init, f);

    return 0;
}

#include <string>
#include <cstdlib>
#include <stdexcept>

#include <nlohmann/json.hpp>

#include <SIRlib.h>
#include <CSVExport.h>
#include <PyramidTimeSeries.h>
#include <TimeStatistic.h>
#include <TimeSeries.h>
#include <RNG.h>
#include <Binomial.h>

#include <Bound.h>
#include <Calibrate.h>
#include <PolyRegCal.hpp>

#include "SIRSimRunner.h"

using namespace SIRlib;
using namespace ComputationalLib;
using namespace SimulationLib;

using json = nlohmann::json;

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
//11. omega:
//      The penalty for regressing towards steeper slopes, used in the 
//      construction of the polynomial used in regression
//12. alpha:
//      The step size. It is multiplied by the slope to calculate
//      the distance to the next vector x_i
//13. epsilon
//      The minimum distance for each step. When a step becomes smaller 
//      than epsilon, the algorithm terminates
//14. b
//      A constant used to calculate Lambda, the learning weight.
//      As 'b' increases, lambda approaches 1. Therefore, the values of
//      'b' and 'maxIters' should be chosen such that "b is very
//      close to 1 as b approaches 'maxIters'??
//15. maxIters
//      algorithm stops when maxIters is reached
//16. timeSeriesFile:
//      Filename of the JSON file used to store the time series data
//      Format of the JSON file
//      {
//          "timeseries": [
//              {
//                  "time": 0.0,
//                  "increment": 1
//              },
//              {
//                  "time": 1.0,
//                  "increment": 2
//              }
//          ]
//      }


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
    auto fileName        = string(argv[++i]);
    auto lambda          = (double)stof(argv[++i], NULL);
    auto gamma           = (double)stof(argv[++i], NULL);
    auto nPeople         = atol(argv[++i]);
    auto ageMin          = atoi(argv[++i]);
    auto ageMax          = atoi(argv[++i]);
    auto ageBreak        = atoi(argv[++i]);
    auto tMax            = atoi(argv[++i]);
    auto deltaT          = atoi(argv[++i]);
    auto pLength         = atoi(argv[++i]);
    auto omega           = (double)stof(argv[++i], NULL);
    auto alpha           = (double)stof(argv[++i], NULL);
    auto epsilon         = (double)stof(argv[++i], NULL);
    auto b               = atoi(argv[++i]);
    auto maxIters        = atoi(argv[++i]);
    auto timeSeriesFile  = string(argv[++i]);
    // End grab from command line ////////////////////////

    // Read JSON input from file
    auto* file = new std::ifstream(timeSeriesFile);
    auto* timeSeriesJSON = new json();
    *file >> *timeSeriesJSON;
    auto timeseries = (*timeSeriesJSON)["timeseries"];

    // Create Historical Data
    Params parameters;
    auto InfectionsData = new IncidenceTimeSeries<int>("Historical data", 0, tMax, pLength, 1, nullptr);
    for (json::iterator it = timeseries.begin(); it != timeseries.end(); ++it) {
        auto unit = *it;
        InfectionsData->Record(unit["time"], unit["increment"]);
        parameters.push_back(std::make_tuple(unit["time"]));
    }

    // Free up memory
    delete timeSeriesJSON;
    delete file;

    // Create type of distribution generator, which will return a binomial
    // distribution. The binomial distribution will be a function of the
    // number of people were infected (in the model), divided by the number
    // of people who are in the simulation. In this case, the number of
    // people in the simulation is constant.
    using Binomial = StatisticalDistributions::Binomial;
    using DG = std::function<Binomial(double,double)>;

    DG distGen = [=] (auto v, auto t) -> Binomial {
        return {nPeople, v/nPeople};
    };

    // Create a lambda function for use with PolyRegCal routine. The
    // only parameters which can be varied by the iterative method
    // are lambda and gamma.
    using F = std::function<double(double,double)>;
    F f = [&] (double lambda, double gamma) -> double {
        
        auto S = SIRSimRunner(fileName, 
                              1, 
                              lambda, 
                              gamma, 
                              nPeople, 
                              ageMin,
                              ageMax, 
                              ageBreak, 
                              tMax, 
                              deltaT,
                              pLength);
        
        S.Run<RunType::Serial>(); // This will return a bool (succ/fail)
        
        // S.Write();
        // ^ Uncomment for debug

        // Only running one trajectory. Pull out number
        // of infections per period ("pLength")
        auto InfectionsModel = S.GetTrajectoryResult(0).Infections;

        auto Likelihood = CalculateLikelihood(*InfectionsModel, 
                                              *InfectionsData, 
                                              parameters, 
                                              distGen);

        return Likelihood;
    };

    using rNearlyZero  = std::ratio<1,100000>;
    using r10    = std::ratio<10,1>;
    using r100   = std::ratio<100,1>;
    using ZeroTo10 = Bound<double, rNearlyZero, r100>;
    using ZeroTo100 = Bound<double, rNearlyZero, r10>;
    using Xs = std::tuple<ZeroTo10, ZeroTo100>;

    Xs init {lambda, gamma};

    auto CalibrationResult = PolyRegCal(init, f, omega, alpha, epsilon, b, maxIters);

    return 0;
}

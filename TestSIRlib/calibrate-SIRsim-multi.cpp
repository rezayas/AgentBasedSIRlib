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

/*
input file: input.json
format:
{
    // Prefix of .csv file name (do not specify extension). Three files will be created: 
    // [fileName]-births.csv, [fileName]-deaths.csv, [filename]-population.csv
    "filename": "output/demo",
    // transmission parameter (double | > 0) unit: [cases/day]
    "lambda": 0.3,
    // duration of infectiousness. (double | > 0) double, unit: [day]
    "gamma": 5,
    "age": {
        // minimum age of an individual (uint) unit: [years]
        "min": 0,
        // maximum age of an individual (uint | >= ageMin) unit: [years]
        "max": 90,
        // interval between age breaks of population (uint | > 1, < (ageMax - ageMin)) unit: [years]
        "break": 5
    },
    // number of people in the population (uint | > 0)
    "nPeople": 10000,
    // maximum length of time to run simulation to (uint | >= 1) unit: [days]
    "tMax": 3650,
    // timestep (uint | >= 1, <= tMax) unit: [days]
    "deltaT": 1,
    // length of one data-aggregation period (uint | > 0, < tMax) unit: [days]
    "pLength": 7,   
    // The penalty for regressing towards steeper slopes, used in the 
    // construction of the polynomial used in regression
    // All inputs of the array will be looped and calibrated
    "omega": [0.001, 0.002],
    // The step size. It is multiplied by the slope to calculate
    // the distance to the next vector x_i
    "alpha": [0.0001],
    // A constant used to calculate Lambda, the learning weight.
    // As 'b' increases, lambda approaches 1. Therefore, the values of
    // 'b' and 'maxIters' should be chosen such that "b is very
    // close to 1 as b approaches 'maxIters'??
    "b": [10, 50, 100],
    // The minimum distance for each step. When a step becomes smaller 
    // than epsilon, the algorithm terminates
    "epsilon": 0.00001,
    // algorithm stops when maxIters is reached
    "maxIters": 100,
    // time series data to be calibrated, step size is 1.0
    "timeseries": [0,2,4,20,56,77,136,314,578,733,813,820,681,503,372]
}
*/

using RunType = SIRSimRunner::RunType;

template <typename Num>
Num toRange(Num v, Num low, Num high)
{
    return 2*(v-low)/(high-low) - 1;
}

template <typename Num>
Num fromRange(Num v, Num low, Num high)
{
    return (high-low)*(v+1)/2 + low;
}

using RunType = SIRSimRunner::RunType;

int main(int argc, char const *argv[])
{
    using Params = std::vector<SimulationLib::TimeSeries<int>::query_type>;
    if (argc < 1) {
        printf("Error: too few arguments\n");
        exit(1);
    }

    // Grab params from command line ////////////////////
    size_t i {0};
    auto inputFile  = string(argv[++i]);    
    // End grab from command line ////////////////////////

    // Read JSON input from file
    auto* file = new std::ifstream(inputFile);
    auto* inputJSON = new json();
    *file >> *inputJSON;

    // Parse JSON Params//////////////////////////////////
    auto fileName        = (*inputJSON)["filename"];
    auto lambda          = (double)(*inputJSON)["lambda"];
    auto gamma           = (double)(*inputJSON)["gamma"];
    auto ageMin          = (int)(*inputJSON)["age"]["min"];
    auto ageMax          = (int)(*inputJSON)["age"]["max"];
    auto ageBreak        = (int)(*inputJSON)["age"]["break"];
    auto nPeople         = (int)(*inputJSON)["nPeople"];
    auto tMax            = (int)(*inputJSON)["tMax"];
    auto deltaT          = (int)(*inputJSON)["deltaT"];
    auto pLength         = (int)(*inputJSON)["pLength"];
    auto omegaEntries    = (*inputJSON)["omega"];
    auto alphaEntries    = (*inputJSON)["alpha"];
    auto bEntries        = (*inputJSON)["b"];
    auto epsilon         = (double)(*inputJSON)["epsilon"];
    auto maxIters        = (int)(*inputJSON)["maxIters"];
    auto timeseries      = (*inputJSON)["timeseries"];
    // End Parse JSON Params//////////////////////////////

    // Free up memory
    delete inputJSON;
    delete file;
    
    // Create Historical Data
    Params parameters;
    auto InfectionsData = new IncidenceTimeSeries<int>("Historical data", 0, tMax, pLength, 1, nullptr);
    auto recordTime = 0.0;
    for (json::iterator it = timeseries.begin(); it != timeseries.end(); ++it) {
        auto unit = *it;
        InfectionsData->Record(recordTime, unit);
        parameters.push_back(std::make_tuple(unit));
        recordTime += 1.0;
    }

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

    // Prepare file for write
    std::ofstream ofs("PolyRegCal.csv", std::ofstream::out | std::ofstream::trunc);
    ofs.close();

    // Iterate through all possible values of lambda and gamma
    // Create a lambda function for use with PolyRegCal routine. The
    // only parameters which can be varied by the iterative method
    // are lambda and gamma.

    for (json::iterator omegaIt = omegaEntries.begin(); omegaIt != omegaEntries.end(); ++omegaIt) {
        for (json::iterator alphaIt = alphaEntries.begin(); alphaIt != alphaEntries.end(); ++alphaIt) {
            for (json::iterator bIt = bEntries.begin(); bIt != bEntries.end(); ++bIt) {
                auto omega = *omegaIt;
                auto alpha = *alphaIt;
                auto b = *bIt;

                std::cout << omega << "," << alpha << "," << b << std::endl;
    
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

                Xs init {toRange(lambda, 0., 10.), toRange(gamma, 0., 100.)};

                auto CalibrationResult = PolyRegCal(init, f, omega, alpha, epsilon, b, maxIters);
            }
        }
    }

    return 0;
}

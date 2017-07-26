#include "catch.hpp"

#include <map>
#include <string>

// Provides "compare_files" function for comparing output of CSVExporter's
//   to reference output
#include "../include/SIRlib/SIRlib.h"

using namespace std;
using namespace SIRlib;

TEST_CASE("Correct parameters, instantiation followed by destruction", "[SIR]") {
    RNG *rng = new RNG(time(NULL));
    SIRSimulation *sir =
      new SIRSimulation(rng, 1, 1, 10, 0, 100, 10, 365, 1, 7);

    delete sir;

    REQUIRE(true);
}

TEST_CASE("Correct parameters, instantiation, run, destruction", "[SIR]") {
    RNG *rng = new RNG(time(NULL));
    SIRSimulation *sir =
      new SIRSimulation(rng, 1, 1, 10, 0, 100, 10, 365, 1, 7);

    sir->Run();

    delete sir;

    REQUIRE(true);
}

TEST_CASE("Correct parameters, instantiation, run, extract, destruction", "[SIR]") {
    RNG *rng = new RNG(time(NULL));
    SIRSimulation *sir =
      new SIRSimulation(rng, 1, 1, 10, 0, 100, 10, 365, 1, 7);

    sir->Run();

    TimeSeries<int> *S_ts = sir->GetData<TimeSeries<int>>(SIRData::Susceptible);

    delete sir;

    REQUIRE(true);
}

TEST_CASE("Bad parameters", "[SIR]") {
    bool threw = false;
    SIRSimulation *sir = nullptr;
    RNG *rng = nullptr/*new RNG(time(NULL))*/;

    // GOOD params:
    // (rng, 1, 1, 10, 0, 100, 10, 365, 1, 7)

    // null RNG
    REQUIRE_THROWS(sir = new SIRSimulation(rng, 1, 1, 10, 0, 100, 10, 365, 1, 7));

    // negative constants
    REQUIRE_THROWS(sir = new SIRSimulation(rng, 0, -1, 10, 0, 100, 10, 365, 1, 7));

    // no people
    REQUIRE_THROWS(sir = new SIRSimulation(rng, 1, 1, 0, 0, 100, 10, 365, 1, 7));

    // ∆t too big
    REQUIRE_THROWS(sir = new SIRSimulation(rng, 1, 1, 10, 0, 100, 10, 365, 366, 7));

    // agebreaks too big
    REQUIRE_THROWS(sir = new SIRSimulation(rng, 1, 1, 10, 0, 10, 11, 365, 1, 7));
}

TEST_CASE("Correct parameters, really long duration of infectiousness", "[SIR]") {
    RNG *rng = new RNG(time(NULL));

    // 2 people, 5 days, duration of infectiousness = 100 days
    SIRSimulation *sir =
      new SIRSimulation(rng, 100, 100, 2, 0, 100, 10, 5, 1, 1);

    sir->Run();

    TimeSeries<int> *I_ts = sir->GetData<TimeSeries<int>>(SIRData::Infected);

    REQUIRE(I_ts->GetTotalAtTime(4) >= 1);

    printf("hi! 5\n");

    delete sir;
}

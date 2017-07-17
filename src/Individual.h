#include <RNG.h>
#include <StatisticalDistribution.h>

using namespace StatisticalDistributions;

namespace SIRlib {

enum class HealthState {
    Susceptible, Infected, Recovered;
};

enum class Sex {
    Male, Female;
};

using Individual = struct {
    HealthState  hs;
    Sex          sex;
    unsigned int age;
};

Individual newIndividual(RNG *rng, StatisticalDistribution *ageDist, \
                         StatisticalDistribution *sexDist, HealthState hs) {
    Individual idv;

    auto lToS = [](long val) { if (val == 0) return Sex::Male;
                               else return Sex::Female; };

    idv.hs  = hs;
    idv.age = (int)ageDist->Sample(*rng);
    idv.sex = lToS(sexDist->Sample(*rng));

    return idv;
}

}

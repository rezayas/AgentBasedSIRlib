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

auto sexN = [](Individual i) { switch(i.sex) { case Sex::Male   : return 0;
                                               case Sex::Female : return 1; } };

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

Individual changeHealthState(Individual idv, HealthState hs) {
    idv.hs = hs;
    return idv;
}

}

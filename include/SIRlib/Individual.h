#pragma once

#include <RNG.h>
#include <StatisticalDistribution.h>

using namespace StatisticalDistributions;

namespace SIRlib {

enum class HealthState {
    Susceptible, Infected, Recovered
};

enum class Sex {
    Male, Female
};

using Age = unsigned int;

using Individual = struct {
    SIRlib::HealthState  hs;
    SIRlib::Sex          sex;
    SIRlib::Age          age;
};

auto sexN = [](Individual i) { switch(i.sex) { case Sex::Male   : return 0;
                                               case Sex::Female : return 1; } };

auto Nsex = [](long n) { if (n == 0) return Sex::Male;
                         else        return Sex::Female; };

Individual newIndividual(RNG *rng, StatisticalDistribution<long double> *ageDist, \
                         StatisticalDistribution<long double> *sexDist, SIRlib::HealthState hs) {
    Individual idv;

    idv.age = (Age)ageDist->Sample(*rng);
    idv.sex = Nsex(sexDist->Sample(*rng));
    idv.hs  = hs;

    return idv;
}

Individual changeHealthState(Individual idv, SIRlib::HealthState hs) {
    idv.hs = hs;
    return idv;
}

}

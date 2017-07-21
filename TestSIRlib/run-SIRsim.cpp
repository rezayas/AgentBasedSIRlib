#include <string>
#include <SIRlib.h>
// # include "../include/SIRlib/SIRlib.h"

using namespace SIRlib;

// Parameters:
// 1: fileName:
//      Prefix of .csv file name (do not specify extension). Three files will
//        be created: [fileName]-births.csv, [fileName]-deaths.csv,
//        [filename]-population.csv
// 2: nTrajectories:
//      number of trajectories to run under the following parameters:
// 3: timeMax:
//      largest value of t to run simulation to
// 4: periodLength:
//      length of period, in units of t (integer)
// 5: nPeople:
//      initial size of population
// 6: pDeath:
//      individual probability of death over 1 time unit
// 7: pBirth:
//      individual probability of single reproduction over 1 time unit
int main(int argc, char const *argv[])
{
    int i;
    string fileName;
    double λ, Ɣ;
    long nPeople;
    uint ageMin, ageMax, tMax, dt, pLength;

    SIRSimulation *SIRsim;

    if (argc < 10) {
        printf("Error: too few arguments\n");
        exit(1);
    }

    i = 0;
    fileName      = string(argv[++i]);
    λ             = stof(argv[++i], NULL);
    Ɣ             = stof(argv[++i], NULL);
    nPeople       = atol(argv[++i]);
    ageMin        = atoi(argv[++i]);
    ageMax        = atoi(argv[++i]);
    tMax          = atoi(argv[++i]);
    dt            = atoi(argv[++i]);
    pLength       = atoi(argv[++i]);      


    printf("Args:\n\tfileName=%s\n\tλ=%4.4f\n\tƔ=%4.4f\n\tnPeople=%ld\n\tageMin=%d\n\tageMax=%d\n\ttMax=%d\n\tdt=%d\n\tpLength=%d\n\n", fileName.c_str(), λ, Ɣ, nPeople, ageMin, ageMax, tMax, dt, pLength); 

    SIRsim = new SIRSimulation(λ, Ɣ, nPeople, ageMin, ageMax, tMax, dt, pLength);
    SIRsim->Run();
    delete SIRsim;

    return 0;
}

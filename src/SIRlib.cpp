#include <cstdio>

#include "CSVExport.h"
#include "PrevalenceTimeSeries.h"

#include "../include/SIRlib/SIRlib.h"

using namespace SimulationLib;

void SIRlib::print(void) {
    TimeSeriesCSVExport<int> tse("output");
    PrevalenceTimeSeries<int> *pts = new PrevalenceTimeSeries<int>("Hello", 10, 2, 0, nullptr);

    pts->Record(0, 100);
    pts->Record(5, 50);
    pts->Record(9, 25);
    pts->Close();

    tse.Add(pts);
    tse.Write();

    delete pts;
}

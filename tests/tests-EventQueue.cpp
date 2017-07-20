#include "catch.hpp"

#include <map>
#include <string>

// Provides "compare_files" function for comparing output of CSVExporter's
//   to reference output
#include "../include/SIRlib/EventQueue.h"

using namespace std;

enum class Event {
    Hello, Goodbye
};

using MyEQ = EventQueue<Event, int>

MyEQ::EventFunc genHello(void) {
    return [](int t, decltype(MyEQ::schedule)) {
        string msg = string("Hello!");

        while (t != 0) {
            msg += string("?");
            t -= 1;
        }

        printf("[t=%2d] ", t);
        printf(c_str(msg));
        printf("\n");

        return true;
    };
}

MyEQ::EventFunc genGoodbye(int numGoodbyes) {
    return [](int t, decltype(MyEQ::schedule)) {
        string msg = string("Goodbye!");

        while (numGoodbyes != 0) {
            msg += string(" Goodbye?");
            numGoodbyes -= 1;
        }

        printf("[t=%2d] ", t);
        printf(c_str(msg));
        printf("\n");

        return true;
    };
}

TEST_CASE("Basic EventQueue: does it compile!?", "[csv]") {
    map<Event, MyEQ::EventFunc> funcs {
        {Event::Hello, genHello},
        {Event::Goodbye, genGoodbye}
    };
    MyEQ eq(funcs);
}

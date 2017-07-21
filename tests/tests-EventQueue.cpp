#include "catch.hpp"

#include <map>
#include <string>

// Provides "compare_files" function for comparing output of CSVExporter's
//   to reference output
#include "../include/SIRlib/EventQueue.h"

using namespace std;
using namespace SIRlib;

enum class Event {
    Hello, Goodbye
};

using MyEQ = EventQueue<Event, int>;

MyEQ::EventFunc<int> genHello(int numHellos) {
    return [numHellos](int t, function<void(int, Event, int)> scheduler) {
        string msg = string("Hello!");
        int nHellos = numHellos;

        while (nHellos != 0) {
            msg += string(" Hello?");
            nHellos -= 1;
        }

        printf("[t=%2d] ", t);
        printf("%s", msg.c_str());
        printf("\n");

        return true;
    };
}

MyEQ::EventFunc<int> genGoodbye(int numGoodbyes) {
    return [numGoodbyes](int t, function<void(int, Event, int)> scheduler) {
        string msg = string("Goodbye!");
        int nGoodbyes = numGoodbyes;

        while (nGoodbyes != 0) {
            msg += string(" Goodbye?");
            nGoodbyes -= 1;
        }

        printf("[t=%2d] ", t);
        printf("%s", msg.c_str());
        printf("\n");

        return true;
    };
}

TEST_CASE("Basic EventQueue: does it compile!?", "[csv]") {
    map<Event, MyEQ::EventGenerator<int>> funcs;
    funcs[Event::Hello]   = MyEQ::EventGenerator<int>(&genHello);
    funcs[Event::Goodbye] = MyEQ::EventGenerator<int>(&genGoodbye);

    MyEQ eq(funcs);

    eq.schedule(0, Event::Hello, 1);
    eq.schedule(0, Event::Goodbye, 1);
}

TEST_CASE("Basic adding, running, and popping events", "[csv]") {
    map<Event, MyEQ::EventGenerator<int>> funcs;
    funcs[Event::Hello]   = MyEQ::EventGenerator<int>(&genHello);
    funcs[Event::Goodbye] = MyEQ::EventGenerator<int>(&genGoodbye);

    MyEQ eq(funcs);

    REQUIRE(eq.empty() == true);

    eq.schedule(0, Event::Hello, 2);
    REQUIRE(eq.empty() == false);

    eq.schedule(1, Event::Goodbye, 2);

    auto e1 = eq.top();
    REQUIRE(e1.second());
    eq.pop();
    REQUIRE(eq.empty() == false);

    auto e2 = eq.top();
    REQUIRE(e2.second());
    eq.pop();
    REQUIRE(eq.empty() == true);
}

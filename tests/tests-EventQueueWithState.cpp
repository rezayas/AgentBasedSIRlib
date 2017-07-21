#include "catch.hpp"

#include <map>
#include <string>

// Provides "compare_files" function for comparing output of CSVExporter's
//   to reference output
#include "../include/SIRlib/EventQueue.h"
#include "./Person.h"

using namespace std;
using namespace SIRlib;

enum class Event {
    Adopt, ColorChange
};

vector<Person> Commmunity;

using MyEQ = EventQueue<Event, int>;

MyEQ::EventFunc<int> genAdopt(int personIdx) {
    return [personIdx](int t, function<void(int, Event, int)> scheduler) {
        printf("[t=%2d] ", t);
        printf("Adopt a dog!\n");

        Person per;
        per = Commmunity.at(personIdx);
        printf("You started with %d dogs\n", per.nDogs);

        Commmunity[personIdx] = incrementDogs(per, 1);
        per = Commmunity.at(personIdx);

        printf("You now have %d dogs\n", per.nDogs);

        return true;
    };
}

MyEQ::EventFunc<int> genColorChange(int personIdx) {
    return [personIdx](int t, function<void(int, Event, int)> scheduler) {
        printf("[t=%2d] ", t);
        printf("Change your favorite color!\n");

        Person per;
        per = Commmunity.at(personIdx);

        string oldFavorite;
        switch(per.color) {
            case FavoriteColor::Blue  : 
                oldFavorite = "blue";
                break;
            case FavoriteColor::Red   : 
                oldFavorite = "red";
                break;
            case FavoriteColor::Green : 
                oldFavorite = "green";
                break;
            default : 
                oldFavorite = "undefined";
                break;
        }

        printf("Your favorite color was: %s\n", oldFavorite.c_str());

        FavoriteColor newColor;
        switch(per.color) {
            case FavoriteColor::Blue  : 
                newColor = FavoriteColor::Red;
                break;
            case FavoriteColor::Red   : 
                newColor = FavoriteColor::Green;
                break;
            case FavoriteColor::Green : 
                newColor = FavoriteColor::Blue;
                break;
        }

        Commmunity[personIdx] = changeFavoriteColor(per, newColor);
        per = Commmunity.at(personIdx);

        string newFavorite;
        switch(per.color) {
            case FavoriteColor::Blue  : 
                newFavorite = "blue";
                break;
            case FavoriteColor::Red   : 
                newFavorite = "red";
                break;
            case FavoriteColor::Green : 
                newFavorite = "green";
                break;
        }

        printf("Your favorite is now: %s\n", newFavorite.c_str());

        return true;
    };
}

TEST_CASE("Check if object variables can get altered", "[csv]") {
    map<Event, MyEQ::EventGenerator<int>> funcs;
    funcs[Event::Adopt]       = MyEQ::EventGenerator<int>(&genAdopt);
    funcs[Event::ColorChange] = MyEQ::EventGenerator<int>(&genColorChange);

    MyEQ eq(funcs);

    Commmunity.push_back(newPerson(2,FavoriteColor::Blue));

    eq.schedule(0, Event::Adopt, 0);
    eq.schedule(1, Event::Adopt, 0);
    eq.schedule(2, Event::ColorChange, 0);
    eq.schedule(3, Event::ColorChange, 0);

    auto e1 = eq.top();
    REQUIRE(e1.second());
    eq.pop();

    auto e2 = eq.top();
    REQUIRE(e2.second());
    eq.pop();

    auto e3 = eq.top();
    REQUIRE(e3.second());
    eq.pop();

    auto e4 = eq.top();
    REQUIRE(e4.second());
    eq.pop();
}

TEST_CASE("Test while loop", "[csv]") {
    map<Event, MyEQ::EventGenerator<int>> funcs;
    funcs[Event::Adopt]       = MyEQ::EventGenerator<int>(&genAdopt);
    funcs[Event::ColorChange] = MyEQ::EventGenerator<int>(&genColorChange);

    MyEQ eq(funcs);

    Commmunity.push_back(newPerson(2,FavoriteColor::Blue));

    eq.schedule(0, Event::Adopt, 0);
    eq.schedule(1, Event::Adopt, 0);
    eq.schedule(2, Event::ColorChange, 0);
    eq.schedule(3, Event::ColorChange, 0);

    while(!eq.empty()) {
        auto e = eq.top();
        REQUIRE(e.second());
        eq.pop();
    }
}

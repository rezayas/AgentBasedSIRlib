#include <queue>
#include <vector>
#include <map>
#include <utility>
#include <functional>

using namespace std;

namespace SIRlib {

template <enum class Event, typename TimeT>
class EventQueue {
public:
    // Function to schedule stuff
    template <typename ...Params>
    void schedule(TimeT t, Event e, Params&&... params) {
        EventFunc ef = eventGenerators[e](forward<Params>(params)...);

        return pq->push(ScheduledEvent(t, ef));
    };

    bool            empty(void) { return pq->empty(); };
    const_reference top(void)   { return pq->top(); };
    void            pop(void)   { return pq->pop(); };

    // Alias types
    using EventFunc = function<bool(TimeT, decltype(schedule))>;

    template <typename ...Params>
    using EventGenerator = function<EventFunc(Params &&...params)>;

    using ScheduledEvent = pair<TimeT, EventFunc>;

    auto ScheduledEventCmp = [](ScheduledEvent left, ScheduledEvent right)
                               { return left.first < right.first; };

    using ScheduledEventPQ = \
      priority_queue<ScheduledEvent,
                     vector<ScheduledEvent>,
                     decltype(ScheduledEventCmp)>;

    // Constructor
    EventQueue(map<Event, EventGenerator<TimeT, ...>> _eventGenerators) {
        eventGenerators = _eventGenerators;
        pq = new ScheduledEventPQ(ScheduledEventCmp);
    }

    // Destructor
    ~EventQueue(void) {
        delete pq;
    }

private:
    map<Event, EventGenerator<TimeT, ...>> eventGenerators;
    ScheduledEventPQ *pq;
};



}

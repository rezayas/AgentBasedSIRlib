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
    // 'schedule' schedules an event 'e' for execution at time 't'. 'params'
    //   allows passing of a variable number of arguments to the EventGenerator
    //   associated with event 'e'. The event is generated and added to the
    //   queue.
    template <typename ...Params>
    void schedule(TimeT t, Event e, Params&&... params);

    // Alias types
    using EventFunc = function<bool(TimeT, decltype(schedule))>;

    using ScheduledEvent = pair<TimeT, EventFunc>;

    template <typename ...Params>
    void schedule(TimeT t, Event e, Params&&... params) {

        // Find the event generator and call it with the provded
        // parameters
        EventFunc ef = eventGenerators[e](forward<Params>(params)...);

        // Take the resulting event function and schedule it at time 't'
        return pq->push(ScheduledEvent(t, ef));
    };

    // The following three methods forward to the underlying data structure
    //   to allow access to the EventFuncs
    bool            empty(void) { return pq->empty(); };
    const_reference top(void)   { return pq->top(); };
    void            pop(void)   { return pq->pop(); };

    // EventGenerator is a function that takes a variable number of parameters
    //   and returns an EventFunc
    template <typename ...Params>
    using EventGenerator = function<EventFunc(Params &&...params)>;

    // Allows comparison of ScheduledEvents for insertion into priority queue
    auto ScheduledEventCmp = [](ScheduledEvent left, ScheduledEvent right)
                               { return left.first < right.first; };

    // Specialized priority_queue for storing 'ScheudledEvent's
    using ScheduledEventPQ = \
      priority_queue<ScheduledEvent,
                     vector<ScheduledEvent>,
                     decltype(ScheduledEventCmp)>;

    // Constructor
    EventQueue(map<Event, EventGenerator> _eventGenerators) {
        eventGenerators = _eventGenerators;
        pq = new ScheduledEventPQ(ScheduledEventCmp);
    }

    // Destructor
    ~EventQueue(void) {
        delete pq;
    }

private:
    map<Event, EventGenerator> eventGenerators;
    ScheduledEventPQ *pq;
};



}

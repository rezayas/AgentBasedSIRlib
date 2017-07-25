#pragma once

#include <queue>
#include <vector>
#include <map>
#include <utility>
#include <functional>

using namespace std;

namespace SIRlib {

template <typename TimeT = double, typename ResultT = bool>
class EventQueue {
public:

    // Alias types
    using EventFuncRunner = function<ResultT(void)>;

    using ScheduledEvent = struct {
        TimeT t;
        EventFuncRunner run;
    };

    using EventFunc = function<ResultT(TimeT, function<void(ScheduledEvent)>)>;

    // Find the event generator and call it with the provded
    // parameters
    ScheduledEvent MakeScheduledEvent(TimeT t, EventFunc ef) {
        ScheduledEvent se;

        auto boundScheduler = bind(&EventQueue::Schedule, this, placeholders::_1);

        EventFuncRunner efr = [this, t, ef, boundScheduler] (void) {
            return ef(t, boundScheduler);
        };

        se.t   = t;
        se.run = efr;

        return se;
    }

    // The following three methods forward to the underlying data structure
    //   to allow access to the EventFuncs
    bool                  Empty(void) { return pq->empty(); };
    const ScheduledEvent&   Top(void) { return pq->top(); };
    void                    Pop(void) { return pq->pop(); };

    void Schedule(ScheduledEvent e)   { return pq->push(e); }

    using SchedulerT = function<void(ScheduledEvent)>;

    // Constructor
    EventQueue(void) {
        auto boundScheduledEventCmp =
          bind(&EventQueue::scheduledEventCmp, this, placeholders::_1, placeholders::_2);

        pq = new ScheduledEventPQ(boundScheduledEventCmp);
    }

    // Destructor
    ~EventQueue(void) {
        delete pq;
    }

private:
    // Allows comparison of ScheduledEvents for insertion into priority queue
    int scheduledEventCmp(const ScheduledEvent& se1, const ScheduledEvent& se2)
        { return se1.t < se2.t; };

    // Specialized priority_queue for storing 'ScheudledEvent's
    using ScheduledEventPQ =
      priority_queue<ScheduledEvent,
                     vector<ScheduledEvent>,

                     function<int(const ScheduledEvent&, const ScheduledEvent&)>>;

    ScheduledEventPQ *pq;
};
}

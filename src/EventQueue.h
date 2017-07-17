#include <queue>
#include <vector>
#include <utility>
#include <functional>

namespace SIRlib {

using EQEventFunction = std::function<bool(void)>;
using EQTimeT = double;
using EQObject = std::pair<TimeT, EQEventFunction>;

auto EQCmp = [](EQObject left, EQObject right) { return left.first < right.first; };

using EventQueue = \
  std::priority_queue<EventQueueObject,
                      std::vector<EventQueueObject>,
                      EQCmp>;

}

#ifndef GROWTHPLAN_H
#define GROWTHPLAN_H

#include <items/sakura_items.h>

namespace Kitsunemimi
{
namespace Sakura
{
class SakuraItem;
struct ActiveCounter;

class GrowthPlan
{
public:
    // subtree, which should be processed by a worker-thread
    SakuraItem* completeSubtree = nullptr;
    // map with all input-values for the subtree
    DataMap items;
    // shared counter-instance, which will be increased after the subtree was fully processed
    ActiveCounter* activeCounter = nullptr;
    GrowthPlan* parentPlan = nullptr;
    // current position in the processing-hirarchy for status-output
    std::vector<std::string> hirarchy;
    std::string filePath = "";

    std::vector<GrowthPlan*> parallelObjects;
    ValueItemMap postAggregation;

    GrowthPlan();
    ~GrowthPlan();

    void clearChilds();
};

} // namespace Sakura
} // namespace Kitsunemimi

#endif // GROWTHPLAN_H

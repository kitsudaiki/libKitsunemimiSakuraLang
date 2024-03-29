#ifndef KITSUNEMIMI_SAKURA_LANG_GROWTHPLAN_H
#define KITSUNEMIMI_SAKURA_LANG_GROWTHPLAN_H

#include <items/sakura_items.h>
#include <processing/active_counter.h>

namespace Kitsunemimi
{
namespace Sakura
{
class SakuraItem;

class GrowthPlan
{
public:
    // subtree, which should be processed by a worker-thread
    SakuraItem* completeSubtree = nullptr;
    // map with all input-values for the subtree
    DataMap items;
    const DataMap* context = nullptr;

    bool success = true;
    ErrorContainer error;
    BlossomStatus status;

    // shared counter-instance, which will be increased after the subtree was fully processed
    ActiveCounter activeCounter;
    GrowthPlan* parentPlan = nullptr;
    // current position in the processing-hirarchy for status-output
    std::vector<std::string> hirarchy;
    std::string filePath = "";

    std::vector<GrowthPlan*> childPlans;
    ValueItemMap postAggregation;

    GrowthPlan();
    ~GrowthPlan();

    void clearChilds();
    void getErrorResult();
};

} // namespace Sakura
} // namespace Kitsunemimi

#endif // KITSUNEMIMI_SAKURA_LANG_GROWTHPLAN_H

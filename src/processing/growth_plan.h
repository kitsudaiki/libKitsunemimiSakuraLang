#ifndef KITSUNEMIMI_SAKURA_LANG_GROWTHPLAN_H
#define KITSUNEMIMI_SAKURA_LANG_GROWTHPLAN_H

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

    bool success = true;
    std::string errorMessage = "";
    uint64_t status;

    // shared counter-instance, which will be increased after the subtree was fully processed
    ActiveCounter* activeCounter = nullptr;
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

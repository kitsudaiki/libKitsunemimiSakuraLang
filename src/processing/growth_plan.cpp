#include "growth_plan.h"

#include <processing/active_counter.h>

namespace Kitsunemimi
{
namespace Sakura
{

GrowthPlan::GrowthPlan() {}

GrowthPlan::~GrowthPlan()
{
    if(completeSubtree != nullptr) {
        delete completeSubtree;
    }
    if(activeCounterParentPart != nullptr) {
        delete activeCounterParentPart;
    }

    clearChilds();
}

void GrowthPlan::clearChilds()
{
    for(GrowthPlan* obj : parallelObjects) {
        delete obj;
    }
    parallelObjects.clear();
}

} // namespace Sakura
} // namespace Kitsunemimi

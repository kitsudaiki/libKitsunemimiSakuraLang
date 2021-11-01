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
    if(activeCounter != nullptr) {
        delete activeCounter;
    }

    clearChilds();
}

void
GrowthPlan::clearChilds()
{
    for(GrowthPlan* child : childPlans) {
        delete child;
    }
    childPlans.clear();
}

void
GrowthPlan::getErrorResult()
{
    for(GrowthPlan* child : childPlans)
    {
        if(child->success == false)
        {
            errorMessage = child->errorMessage;
            status = child->status;
            success = child->success;
        }
    }
}

} // namespace Sakura
} // namespace Kitsunemimi

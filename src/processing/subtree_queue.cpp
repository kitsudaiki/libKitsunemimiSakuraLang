/**
 * @file        subtree_queue.cpp
 *
 * @author      Tobias Anker <tobias.anker@kitsunemimi.moe>
 *
 * @copyright   Apache License Version 2.0
 *
 *      Copyright 2019 Tobias Anker
 *
 *      Licensed under the Apache License, Version 2.0 (the "License");
 *      you may not use this file except in compliance with the License.
 *      You may obtain a copy of the License at
 *
 *          http://www.apache.org/licenses/LICENSE-2.0
 *
 *      Unless required by applicable law or agreed to in writing, software
 *      distributed under the License is distributed on an "AS IS" BASIS,
 *      WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *      See the License for the specific language governing permissions and
 *      limitations under the License.
 */

#include "subtree_queue.h"

#include <items/item_methods.h>
#include <processing/active_counter.h>
#include <processing/growth_plan.h>
#include <libKitsunemimiCommon/logger.h>

namespace Kitsunemimi
{
namespace Sakura
{

/**
 * @brief constructor
 */
SubtreeQueue::SubtreeQueue() {}

/**
 * @brief add a new subtree-object to the queue
 *
 * @param newObject the new subtree-object, which should be added to the queue
 */
void
SubtreeQueue::addGrowthPlan(GrowthPlan* newObject)
{
    std::lock_guard<std::mutex> guard(m_lock);
    m_queue.push(newObject);
}

/**
 * @brief run a parallel loop
 *
 * @param plan plan with all information of the current process
 * @param subtree subtree, which should be executed multiple times by multiple threads
 * @param tempVarName loop-internal variable
 * @param array data-array in case of an iterator-loop, else nullptr
 * @param endPos end position in array or counter end
 * @param startPos start position in array or counter start
 *
 * @return true, if successful, else false
 */
bool
SubtreeQueue::spawnParallelSubtreesLoop(GrowthPlan* plan,
                                        SakuraItem* subtreeItem,
                                        const std::string &tempVarName,
                                        DataArray* array,
                                        uint64_t endPos,
                                        const uint64_t startPos)
{
    plan->activeCounter.shouldCount = static_cast<uint32_t>(endPos - startPos);

    for(uint64_t i = startPos; i < endPos; i++)
    {
        // encapsulate the content of the loop together with the values and the counter-object
        // as an subtree-object and add it to the subtree-queue
        GrowthPlan* childPlan = new GrowthPlan();
        childPlan->completeSubtree = subtreeItem->copy();
        childPlan->items = plan->items;
        childPlan->hirarchy = plan->hirarchy;
        childPlan->parentPlan = plan;
        childPlan->filePath = plan->filePath;
        childPlan->context = plan->context;

        // add the counter-variable as new value to be accessable within the loop
        if(array != nullptr) {
            childPlan->items.insert(tempVarName, array->get(i)->copy(), true);
        } else {
            childPlan->items.insert(tempVarName, new DataValue(static_cast<long>(i)), true);
        }

        addGrowthPlan(childPlan);
        plan->childPlans.push_back(childPlan);
    }

    waitUntilFinish(&plan->activeCounter);

    if(plan->success)
    {
        // post-processing and cleanup
        GrowthPlan* parent = nullptr;
        for(GrowthPlan* child : plan->childPlans)
        {
            parent = child->parentPlan;
            if(fillInputValueItemMap(parent->postAggregation, child->items, plan->error) == false) {
                plan->success = false;
            }
        }

        if(parent != nullptr) {
            overrideItems(parent->items, parent->postAggregation, ONLY_EXISTING);
        }
    }
    else
    {
        plan->getErrorResult();
    }

    plan->clearChilds();

    return plan->success;
}

/**
 * @brief run multiple different subtrees in parallel threads
 *
 * @param plan plan with all information of the current process
 * @param childs vector with subtrees, where each subtree should be executed by another thread
 *
 * @return true, if successful, else false
 */
bool
SubtreeQueue::spawnParallelSubtrees(GrowthPlan* plan,
                                    const std::vector<SakuraItem*> &childs)
{
    LOG_DEBUG("spawnParallelSubtrees");

    plan->activeCounter.shouldCount = static_cast<uint32_t>(childs.size());

    // encapsulate each subtree of the paralle part as subtree-object and add it to the
    // subtree-queue for parallel processing
    for(uint64_t i = 0; i < childs.size(); i++)
    {
        GrowthPlan* childPlan = new GrowthPlan();
        childPlan->completeSubtree = childs.at(i)->copy();
        childPlan->hirarchy = plan->hirarchy;
        childPlan->items = plan->items;
        childPlan->parentPlan = plan;
        childPlan->filePath = plan->filePath;
        childPlan->context = plan->context;

        addGrowthPlan(childPlan);
        plan->childPlans.push_back(childPlan);
    }

    waitUntilFinish(&plan->activeCounter);

    // write result back for output
    if(plan->success)
    {
        for(GrowthPlan* child : plan->childPlans) {
            overrideItems(plan->items, child->items, ALL);
        }
    }
    else
    {
        plan->getErrorResult();
    }

    plan->clearChilds();

    return plan->success;
}


/**
 * @brief getGrowthPlan take ta object from the queue and delete it from the queue
 *
 * @return first object in the queue or an empty-object, if nothing is in the queue
 */
GrowthPlan*
SubtreeQueue::getGrowthPlan()
{
    std::lock_guard<std::mutex> guard(m_lock);

    GrowthPlan* subtree = nullptr;

    if(m_queue.empty() == false)
    {
        subtree = m_queue.front();
        m_queue.pop();
    }

    return subtree;
}

/**
 * @brief wait until all spawned tasks are finished
 *
 * @param activeCounter pointer to the active-counter, which was given each spawned thread
 *
 * @return true, if successful, else false
 */
void
SubtreeQueue::waitUntilFinish(ActiveCounter* activeCounter)
{
    // wait until the created subtree was fully processed by the worker-threads
    while(activeCounter->isEqual() == false) {
        std::this_thread::sleep_for(chronoMilliSec(10));
    }
}

} // namespace Sakura
} // namespace Kitsunemimi

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
#include <libKitsunemimiPersistence/logger/logger.h>

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
    m_lock.lock();
    m_queue.push(newObject);
    m_lock.unlock();
}

/**
 * @brief run a parallel loop
 *
 * @param subtree subtree, which should be executed multiple times by multiple threads
 * @param postProcessing post-aggregation information
 * @param filePath path of the file, where the subtree belongs to
 * @param hierarchy actual hierarchy for terminal output
 * @param parentValues data-map with parent-values
 * @param tempVarName loop-internal variable
 * @param array data-array in case of an iterator-loop, else nullptr
 * @param errorMessage reference for error-message
 * @param endPos end position in array or counter end
 * @param startPos start position in array or counter start
 *
 * @return true, if successful, else false
 */
bool
SubtreeQueue::spawnParallelSubtreesLoop(GrowthPlan* plan,
                                        SakuraItem* subtreeItem,
                                        ValueItemMap postProcessing,
                                        const std::string &tempVarName,
                                        DataArray* array,
                                        std::string &errorMessage,
                                        uint64_t endPos,
                                        const uint64_t startPos)
{
    ActiveCounter* activeCounter = new ActiveCounter();
    activeCounter->shouldCount = static_cast<uint32_t>(endPos - startPos);
    plan->activeCounter = activeCounter;

    for(uint64_t i = startPos; i < endPos; i++)
    {
        // encapsulate the content of the loop together with the values and the counter-object
        // as an subtree-object and add it to the subtree-queue
        GrowthPlan* object = new GrowthPlan();
        object->completeSubtree = subtreeItem->copy();
        object->items = plan->items;
        object->hirarchy = plan->hirarchy;
        object->activeCounter = activeCounter;
        object->filePath = plan->filePath;

        // add the counter-variable as new value to be accessable within the loop
        if(array != nullptr) {
            object->items.insert(tempVarName, array->get(i)->copy(), true);
        } else {
            object->items.insert(tempVarName, new DataValue(static_cast<long>(i)), true);
        }

        addGrowthPlan(object);
        plan->parallelObjects.push_back(object);
    }

    bool result = waitUntilFinish(activeCounter, errorMessage);

    // post-processing and cleanup
    for(uint64_t i = 0; i < endPos - startPos; i++)
    {
        std::string errorMessage = "";
        if(fillInputValueItemMap(postProcessing,
                                 plan->parallelObjects.at(static_cast<uint32_t>(i))->items,
                                 errorMessage) == false)
        {
            errorMessage = createError("subtree-processing",
                                       "error processing post-aggregation of for-loop:\n"
                                       + errorMessage);
            result = false;
        }
    }

    overrideItems(plan->items, postProcessing, ONLY_EXISTING);

    clearSpawnedObjects(plan->parallelObjects);
    //TODO: delete plan and its content

    return result;
}

/**
 * @brief run multiple different subtrees in parallel threads
 *
 * @param resultingItems map for resulting items
 * @param childs vector with subtrees, where each subtree should be executed by another thread
 * @param filePath path of the file, where the subtree belongs to
 * @param hierarchy actual hierarchy for terminal output
 * @param parentValues data-map with parent-values
 * @param errorMessage reference for error-message
 *
 * @return true, if successful, else false
 */
bool
SubtreeQueue::spawnParallelSubtrees(GrowthPlan* plan,
                                    const std::vector<SakuraItem*> &childs,
                                    std::string &errorMessage)
{
    LOG_DEBUG("spawnParallelSubtrees");

    ActiveCounter* activeCounter = new ActiveCounter();
    activeCounter->shouldCount = static_cast<uint32_t>(childs.size());
    plan->activeCounter = activeCounter;

    // encapsulate each subtree of the paralle part as subtree-object and add it to the
    // subtree-queue for parallel processing
    for(uint64_t i = 0; i < childs.size(); i++)
    {
        GrowthPlan* object = new GrowthPlan();
        object->completeSubtree = childs.at(i)->copy();
        object->hirarchy = plan->hirarchy;
        object->items = plan->items;
        object->activeCounter = activeCounter;
        object->filePath = plan->filePath;

        addGrowthPlan(object);
        plan->parallelObjects.push_back(object);
    }

    const bool ret = waitUntilFinish(activeCounter, errorMessage);

    // write result back for output
    if(plan->parallelObjects.size() >= 1)
    {
        overrideItems(plan->items,
                      plan->parallelObjects.at(0)->items,
                      ALL);
    }

    clearSpawnedObjects(plan->parallelObjects);

    //TODO: delete plan and its content

    return ret;
}


/**
 * @brief getGrowthPlan take ta object from the queue and delete it from the queue
 *
 * @return first object in the queue or an empty-object, if nothing is in the queue
 */
GrowthPlan*
SubtreeQueue::getGrowthPlan()
{
    GrowthPlan* subtree = nullptr;

    m_lock.lock();
    if(m_queue.empty() == false)
    {
        subtree = m_queue.front();
        m_queue.pop();
    }
    m_lock.unlock();

    return subtree;
}

/**
 * @brief wait until all spawned tasks are finished
 *
 * @param activeCounter pointer to the active-counter, which was given each spawned thread
 * @param errorMessage reference for error-message
 *
 * @return true, if successful, else false
 */
bool
SubtreeQueue::waitUntilFinish(ActiveCounter* activeCounter,
                              std::string &errorMessage)
{
    // wait until the created subtree was fully processed by the worker-threads
    while(activeCounter->isEqual() == false) {
        std::this_thread::sleep_for(chronoMilliSec(10));
    }

    // in case of on error, forward this error to the upper layer
    const bool result = activeCounter->success;
    if(result == false) {
        errorMessage = activeCounter->outputMessage;
    }

    return result;
}

/**
 * @brief free memory of all spawned objects
 *
 * @param vector with spawned queue-objects
 */
void
SubtreeQueue::clearSpawnedObjects(std::vector<GrowthPlan*> &spawnedObjects)
{
    ActiveCounter* activeCounter = nullptr;

    // free allocated resources
    for(GrowthPlan* obj : spawnedObjects)
    {
        activeCounter = obj->activeCounter;
        delete obj->completeSubtree;
        delete obj;
    }

    if(activeCounter != nullptr) {
        delete activeCounter;
    }
}

} // namespace Sakura
} // namespace Kitsunemimi

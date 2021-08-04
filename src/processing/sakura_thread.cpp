/**
 * @file        sakura_thread.cpp
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

#include "sakura_thread.h"

#include <items/item_methods.h>
#include <sakura_garden.h>

#include <processing/subtree_queue.h>
#include <processing/thread_pool.h>
#include <processing/active_counter.h>
#include <processing/growth_plan.h>

#include <libKitsunemimiSakuraLang/blossom.h>
#include <libKitsunemimiSakuraLang/sakura_lang_interface.h>

#include <libKitsunemimiJinja2/jinja2_converter.h>
#include <libKitsunemimiPersistence/logger/logger.h>
#include <libKitsunemimiPersistence/files/file_methods.h>

namespace Kitsunemimi
{
namespace Sakura
{

/**
 * @brief constructor
 *
 * @param queue pointer to the subtree-queue, where new separated subtrees should be added
 */
SakuraThread::SakuraThread(SakuraLangInterface* interface)
{
    m_interface = interface;
}

/**
 * @brief run the main-loop of the thread and process each subtree, which can be taken from the
 *        queue
 */
void
SakuraThread::run()
{
    m_started = true;
    while(m_abort == false)
    {
        GrowthPlan* plan = m_interface->m_queue->getGrowthPlan();

        if(plan != nullptr)
        {
            if(plan->completeSubtree != nullptr)
            {
                // process input-values
                DataMap baseItems;

                overrideItems(baseItems,
                              plan->completeSubtree->values,
                              ALL);
                overrideItems(baseItems,
                              plan->items,
                              ALL);

                // run the real task
                std::string errorMessage = "";
                const bool result = processSakuraItem(plan,
                                                      plan->completeSubtree,
                                                      errorMessage);
                // handle result
                if(result == false) {
                    plan->activeCounterChildPart->registerError(errorMessage);
                }

                // increase active-counter as last step, so the source subtree can check, if all
                // spawned subtrees are finished
                plan->activeCounterChildPart->increaseCounter();
            }
        }
        else
        {
            // if no tree was in the queue, then sleep before try it again
            std::this_thread::sleep_for(chronoMilliSec(10));
        }
    }
}

/**
 * @brief central method of the thread to process the current part of the execution-tree
 *
 * @param sakuraItem subtree, which should be processed
 * @param filePath of the current file
 * @param errorMessage reference for error-message
 *
 * @return true if successful, else false
 */
bool
SakuraThread::processSakuraItem(GrowthPlan* plan,
                                SakuraItem* sakuraItem,
                                std::string &errorMessage)
{
    // case that another thread has failed
    // only the failing thread return the false as result
    if(plan->activeCounterChildPart->success == false) {
        return true;
    }

    //----------------------------------------------------------------------------------------------
    if(sakuraItem->getType() == SakuraItem::SEQUENTIELL_ITEM)
    {
        SequentiellPart* sequential = dynamic_cast<SequentiellPart*>(sakuraItem);
        return processSequeniellPart(plan, sequential, errorMessage);
    }
    //----------------------------------------------------------------------------------------------
    if(sakuraItem->getType() == SakuraItem::TREE_ITEM)
    {
        TreeItem* subtreeItem = dynamic_cast<TreeItem*>(sakuraItem);
        plan->hirarchy.push_back("TREE: " + subtreeItem->id);
        const bool result = processTree(plan, subtreeItem, errorMessage);
        plan->hirarchy.pop_back();
        return result;
    }
    //----------------------------------------------------------------------------------------------
    if(sakuraItem->getType() == SakuraItem::SUBTREE_ITEM)
    {
        SubtreeItem* subtreeItem = dynamic_cast<SubtreeItem*>(sakuraItem);
        return processSubtree(plan, subtreeItem, errorMessage);
    }
    //----------------------------------------------------------------------------------------------
    if(sakuraItem->getType() == SakuraItem::BLOSSOM_ITEM)
    {
        BlossomItem* blossomItem = dynamic_cast<BlossomItem*>(sakuraItem);
        return processBlossom(plan, *blossomItem, errorMessage);
    }
    //----------------------------------------------------------------------------------------------
    if(sakuraItem->getType() == SakuraItem::BLOSSOM_GROUP_ITEM)
    {
        BlossomGroupItem* blossomGroupItem = dynamic_cast<BlossomGroupItem*>(sakuraItem);
        return processBlossomGroup(plan, *blossomGroupItem, errorMessage);
    }
    //----------------------------------------------------------------------------------------------
    if(sakuraItem->getType() == SakuraItem::IF_ITEM)
    {
        IfBranching* ifBranching = dynamic_cast<IfBranching*>(sakuraItem);
        return processIf(plan, ifBranching, errorMessage);
    }
    //----------------------------------------------------------------------------------------------
    if(sakuraItem->getType() == SakuraItem::FOR_EACH_ITEM)
    {
        ForEachBranching* forEachBranching = dynamic_cast<ForEachBranching*>(sakuraItem);
        return processForEach(plan, forEachBranching, errorMessage);
    }
    //----------------------------------------------------------------------------------------------
    if(sakuraItem->getType() == SakuraItem::FOR_ITEM)
    {
        ForBranching* forBranching = dynamic_cast<ForBranching*>(sakuraItem);
        return processFor(plan, forBranching, errorMessage);
    }
    //----------------------------------------------------------------------------------------------
    if(sakuraItem->getType() == SakuraItem::PARALLEL_ITEM)
    {
        ParallelPart* parallel = dynamic_cast<ParallelPart*>(sakuraItem);
        return processParallelPart(plan, parallel, errorMessage);
    }
    //----------------------------------------------------------------------------------------------

    // This case should never appear. It can't be produced by the parser at the moment,
    // so if this assert is called, there so something totally wrong in the implementation
    assert(false);

    return false;
}

/**
 * @brief process single blossom
 *
 * @param blossomItem item with all information for the blossom
 * @param filePath of the current file
 * @param errorMessage reference for error-message
 *
 * @return true if successful, else false
 */
bool
SakuraThread::processBlossom(GrowthPlan* plan,
                             BlossomItem &blossomItem,
                             std::string &errorMessage)
{
    // only debug-output
    LOG_DEBUG("process blossom:");
    LOG_DEBUG("    name: " + blossomItem.blossomName);

    // process values by filling with information of the parent-object
    const bool result = fillInputValueItemMap(blossomItem.values, plan->items, errorMessage);
    if(result == false)
    {
        errorMessage = createError(blossomItem,
                                   plan->filePath,
                                   "processing",
                                   "error while processing blossom items:\n    " + errorMessage);
        return false;
    }

    LOG_DEBUG("    values:\n" + blossomItem.values.toString());

    // get and prcess the requested blossom
    Blossom* blossom = m_interface->getBlossom(blossomItem.blossomGroupType,
                                               blossomItem.blossomType);
    if(blossom == nullptr)
    {
        errorMessage = createError(blossomItem,
                                   plan->filePath,
                                   "processing",
                                   "unknow blossom-type");
        return false;
    }

    BlossomLeaf blossomLeaf;

    // update blossom-leaf for processing
    blossomLeaf.blossomPath = plan->filePath;
    blossomLeaf.nameHirarchie = plan->hirarchy;
    blossomLeaf.parentValues = &plan->items;
    blossomLeaf.nameHirarchie.push_back("BLOSSOM: " + blossomItem.blossomName);

    convertValueMap(blossomLeaf.input, blossomItem.values);

    // process blossom
    const bool ret = blossom->growBlossom(blossomLeaf, errorMessage);
    if(ret == false) {
        return false;
    }

    // send result to root
    m_interface->printOutput(blossomLeaf);

    // write processing result back to parent
    fillOutputValueItemMap(blossomItem.values, blossomLeaf.output);

    // TODO: override only with the output-values to avoid unnecessary conflicts
    overrideItems(plan->items, blossomItem.values, ONLY_EXISTING);

    return true;
}

/**
 * @brief process a group of blossoms
 *
 * @param blossomGroupItem object, which should be processed
 * @param filePath of the current file
 * @param errorMessage reference for error-message
 *
 * @return true if successful, else false
 */
bool
SakuraThread::processBlossomGroup(GrowthPlan* plan,
                                  BlossomGroupItem &blossomGroupItem,
                                  std::string &errorMessage)
{
    // convert name as jinja2-string
    std::string convertResult = "";
    Jinja2::Jinja2Converter* converter = Jinja2::Jinja2Converter::getInstance();
    const bool ret = converter->convert(convertResult,
                                        blossomGroupItem.id,
                                        &plan->items,
                                        errorMessage);
    if(ret == false)
    {
        errorMessage = createError("jinja2-converter", errorMessage);
        return false;
    }

    LOG_DEBUG("process blossom group: " + convertResult);

    // print blossom-group
    blossomGroupItem.id = convertResult;
    blossomGroupItem.nameHirarchie = plan->hirarchy;
    blossomGroupItem.nameHirarchie.push_back("BLOSSOM-GROUP: " + blossomGroupItem.id);
    m_interface->printOutput(blossomGroupItem);

    // iterate over all blossoms of the group and process one after another
    for(BlossomItem* blossomItem : blossomGroupItem.blossoms)
    {
        // handle special-cass of a ressource-call
        TreeItem* tempItem = m_interface->m_garden->getRessource(blossomItem->blossomType);
        if(tempItem != nullptr)
        {
            LOG_DEBUG("process resouces: " + tempItem->id);

            const bool ret = runSubtreeCall(plan,
                                            tempItem,
                                            blossomGroupItem.values,
                                            errorMessage);
            delete tempItem;

            return ret;
        }

        // update blossom-item with group-values for console-output
        blossomItem->blossomGroupType = blossomGroupItem.blossomGroupType;
        blossomItem->blossomName = blossomGroupItem.id;
        blossomItem->blossomGroupType = blossomGroupItem.blossomGroupType;

        // copy values of the blossom-group into the blossom, but only values, which are not defined
        // which in the blossom
        overrideItems(blossomItem->values,
                      blossomGroupItem.values,
                      ONLY_NON_EXISTING);

        if(processBlossom(plan, *blossomItem, errorMessage) == false) {
            return false;
        }
    }

    return true;
}

/**
 * @brief process a new tree
 *
 * @param treeItem object, which should be processed
 * @param errorMessage reference for error-message
 *
 * @return true if successful, else false
 */
bool
SakuraThread::processTree(GrowthPlan* plan,
                          TreeItem* treeItem,
                          std::string &errorMessage)
{
    LOG_DEBUG("process tree: " + treeItem->id);

    // check if there are uninitialized items in the tree
    const std::vector<std::string> uninitItems = checkItems(plan->items);
    if(uninitItems.size() > 0)
    {
        std::string message = "The following items are not initialized: \n";
        for(const std::string& uninitItem : uninitItems) {
            message += "    " + uninitItem + "\n";
        }
        errorMessage = createError("processing", message);

        return false;
    }

    // process items of the tree
    const std::string originalFilePath = plan->filePath;
    plan->filePath = treeItem->rootPath + "/" + treeItem->relativePath;
    if(processSakuraItem(plan, treeItem->childs, errorMessage) == false) {
        return false;
    }

    plan->filePath = originalFilePath;

    return true;
}

/**
 * @brief process a new subtree
 *
 * @param subtreeItem object, which should be processed
 * @param filePath of the current file
 * @param errorMessage reference for error-message
 *
 * @return true if successful, else false
 */
bool
SakuraThread::processSubtree(GrowthPlan* plan,
                             SubtreeItem* subtreeItem,
                             std::string &errorMessage)
{
    // get sakura-file based on the required path
    Kitsunemimi::Sakura::SakuraGarden* garden = m_interface->m_garden;
    const bfs::path relPath = garden->getRelativePath(plan->filePath,
                                                      subtreeItem->nameOrPath);

    // get and check tree
    TreeItem* newSubtree = garden->getTree(relPath.string());
    if(newSubtree == nullptr)
    {
        errorMessage = createError("subtree-processing",
                                   "subtree doesn't exist: " + subtreeItem->nameOrPath);
        return false;
    }

    LOG_DEBUG("process subtree: " + newSubtree->id + " in path " + newSubtree->relativePath);

    const bool ret = runSubtreeCall(plan,
                                    newSubtree,
                                    subtreeItem->values,
                                    errorMessage);
    delete newSubtree;

    return ret;
}

/**
 * @brief process a if-else-condition
 *
 * @param ifCondition object, which should be processed
 * @param filePath of the current file
 * @param errorMessage reference for error-message
 *
 * @return true if successful, else false
 */
bool
SakuraThread::processIf(GrowthPlan* plan,
                        IfBranching* ifCondition,
                        std::string &errorMessage)
{
    // initialize
    bool ifMatch = false;
    ValueItem valueItem;

    // get left side of the comparism
    if(fillValueItem(ifCondition->leftSide, plan->items, errorMessage) == false)
    {
        errorMessage = createError("subtree-processing",
                                   "error processing if-condition:\n"
                                   + errorMessage);
        return false;
    }

    // get right side of the comparism
    if(fillValueItem(ifCondition->rightSide, plan->items, errorMessage) == false)
    {
        errorMessage = createError("subtree-processing",
                                   "error processing if-condition:\n"
                                   + errorMessage);
        return false;
    }

    // convert values into strings
    const std::string leftSide = ifCondition->leftSide.item->toString();
    const std::string rightSide = ifCondition->rightSide.item->toString();

    // compare based on the compare-type
    switch(ifCondition->ifType)
    {
        case IfBranching::EQUAL:
            {
                ifMatch = leftSide == rightSide;
                break;
            }
        case IfBranching::UNEQUAL:
            {
                ifMatch = leftSide != rightSide;
                break;
            }
        default:
            // not implemented
            assert(false);
            break;
    }

    // based on the result, process the if-subtree or the else-subtree
    if(ifMatch) {
        return processSakuraItem(plan, ifCondition->ifContent, errorMessage);
    } else {
        return processSakuraItem(plan, ifCondition->elseContent, errorMessage);
    }
}

/**
 * @brief process a for-each-loop
 *
 * @param forEachItem object, which should be processed
 * @param filePath of the current file
 * @param errorMessage reference for error-message
 *
 * @return true if successful, else false
 */
bool
SakuraThread::processForEach(GrowthPlan* plan,
                             ForEachBranching* forEachItem,
                             std::string &errorMessage)
{
    // initialize the array, over twhich the loop should iterate
    if(fillInputValueItemMap(forEachItem->iterateArray, plan->items, errorMessage) == false)
    {
        errorMessage = createError("subtree-processing",
                                   "error processing for-loop:\n"
                                   + errorMessage);
        return false;
    }

    DataArray* array = forEachItem->iterateArray.get("array")->toArray();

    // process content normal or parallel via worker-threads
    bool result = false;
    if(forEachItem->parallel == false)
    {
        result = runLoop(plan,
                         forEachItem->content,
                         forEachItem->values,
                         forEachItem->tempVarName,
                         array,
                         errorMessage,
                         array->size());
    }
    else
    {
        plan->postAggregation = forEachItem->values;
        result = m_interface->m_queue->spawnParallelSubtreesLoop(plan,
                                                                 forEachItem->content,
                                                                 forEachItem->tempVarName,
                                                                 array,
                                                                 errorMessage,
                                                                 array->size());
    }

    return result;
}

/**
 * @brief process a for-loop
 *
 * @param forItem object, which should be processed
 * @param filePath of the current file
 * @param errorMessage reference for error-message
 *
 * @return true if successful, else false
 */
bool
SakuraThread::processFor(GrowthPlan* plan,
                         ForBranching* forItem,
                         std::string &errorMessage)
{
    // get start-value
    if(fillValueItem(forItem->start, plan->items, errorMessage) == false)
    {
        errorMessage = createError("subtree-processing",
                                   "error processing for-loop:\n"
                                   + errorMessage);
        return false;
    }

    // get end-value
    if(fillValueItem(forItem->end, plan->items, errorMessage) == false)
    {
        errorMessage = createError("subtree-processing",
                                   "error processing for-loop:\n"
                                   + errorMessage);
        return false;
    }

    // convert values
    const uint64_t startValue = static_cast<uint64_t>(forItem->start.item->toValue()->getLong());
    const uint64_t endValue = static_cast<uint64_t>(forItem->end.item->toValue()->getLong());

    // process content normal or parallel via worker-threads
    bool result = false;
    if(forItem->parallel == false)
    {
        result = runLoop(plan,
                         forItem->content,
                         forItem->values,
                         forItem->tempVarName,
                         nullptr,
                         errorMessage,
                         endValue,
                         startValue);
    }
    else
    {
        plan->postAggregation = forItem->values;
        result = m_interface->m_queue->spawnParallelSubtreesLoop(plan,
                                                                 forItem->content,
                                                                 forItem->tempVarName,
                                                                 nullptr,
                                                                 errorMessage,
                                                                 endValue,
                                                                 startValue);
    }

    return result;
}

/**
 * @brief process sequentiall part
 *
 * @param subtree object, which should be processed
 * @param filePath of the current file
 * @param errorMessage reference for error-message
 *
 * @return true if successful, else false
 */
bool
SakuraThread::processSequeniellPart(GrowthPlan* plan,
                                    SequentiellPart* subtree,
                                    std::string &errorMessage)
{
    for(SakuraItem* item : subtree->childs)
    {
        if(processSakuraItem(plan, item, errorMessage) == false) {
            return false;
        }
    }

    return true;
}

/**
 * @brief process parallel part via worker-threads
 *
 * @param parallelPart object, which should be processed
 * @param filePath of the current file
 * @param errorMessage reference for error-message
 *
 * @return true if successful, else false
 */
bool
SakuraThread::processParallelPart(GrowthPlan* plan,
                                  ParallelPart* parallelPart,
                                  std::string &errorMessage)
{
    SequentiellPart* parts = dynamic_cast<SequentiellPart*>(parallelPart->childs);
    const bool result = m_interface->m_queue->spawnParallelSubtrees(plan,
                                                                    parts->childs,
                                                                    errorMessage);

    return result;
}

/**
 * @brief internal processing of tree-item
 *
 * @param newSubtree tree-item to call
 * @param values input-values
 * @param filePath of the current file
 * @param errorMessage reference for error-message
 *
 * @return true, if check successful, else false
 */
bool
SakuraThread::runSubtreeCall(GrowthPlan* plan,
                             SakuraItem* newSubtree,
                             ValueItemMap &values,
                             std::string &errorMessage)
{
    // fill values
    const bool fillResult = fillInputValueItemMap(values, plan->items, errorMessage);
    if(fillResult == false)
    {
        errorMessage = createError("subtree-processing",
                                   "error while processing blossom items:\n"
                                   + errorMessage);
        return false;
    }

    // backup and reset parent
    DataMap parentBackup = plan->items;
    plan->items.clear();

    // set values
    overrideItems(newSubtree->values, values, ALL);
    overrideItems(plan->items, newSubtree->values, ALL);

    // process tree-item
    if(processSakuraItem(plan, newSubtree, errorMessage) == false) {
        return false;
    }

    // write output back after restoring the parent-values to resume normally
    if(fillOutputValueItemMap(newSubtree->values, plan->items) == false) {
        return false;
    }

    // write values back
    plan->items = parentBackup;
    overrideItems(plan->items, newSubtree->values, ONLY_EXISTING);

    return true;
}

/**
 * @brief run a normal loop
 *
 * @param loopContent content of the loop, which should be executed multiple times
 * @param values input-values
 * @param filePath of the current file
 * @param tempVarName temporary variable name for usage within the loop to forward the object
 *                    over which is generated of the counter-variable
 * @param array nullptr if iterate over a range or pointer to array-item to iterate over this array
 * @param errorMessage reference for error-message
 * @param endPos start-position in the array or of the counter
 * @param startPos start-position in the array or of the counter
 *
 * @return true, if check successful, else false
 */
bool
SakuraThread::runLoop(GrowthPlan* plan,
                      SakuraItem* loopContent,
                      const ValueItemMap &values,
                      const std::string &tempVarName,
                      DataArray* array,
                      std::string &errorMessage,
                      const uint64_t endPos,
                      const uint64_t startPos)
{
    // backup the parent-values to avoid permanent merging with loop-internal values
    DataMap preBalueBackup = plan->items;
    overrideItems(plan->items, values, ALL);

    for(uint64_t i = startPos; i < endPos; i++)
    {
        // add the counter-variable as new value to be accessable within the loop
        if(array != nullptr) {
            plan->items.insert(tempVarName, array->get(i)->copy(), true);
        } else {
            plan->items.insert(tempVarName, new DataValue(static_cast<long>(i)), true);
        }

        // process content
        SakuraItem* tempItem = loopContent->copy();
        if(processSakuraItem(plan, tempItem, errorMessage) == false) {
            return false;
        }
        delete tempItem;
    }

    // restore the old parent values and update only the existing values with the one form the
    // loop. That way, variables like the counter-variable are not added to the parent.
    DataMap postBalueBackup = plan->items;
    plan->items = preBalueBackup;
    overrideItems(plan->items, postBalueBackup, ONLY_EXISTING);

    return true;
}

} // namespace Sakura
} // namespace Kitsunemimi

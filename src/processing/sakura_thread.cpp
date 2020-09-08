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
#include <libKitsunemimiSakuraLang/sakura_garden.h>

#include <processing/subtree_queue.h>
#include <processing/thread_pool.h>

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
        m_currentSubtree = m_interface->m_queue->getSubtreeObject();

        if(m_currentSubtree != nullptr)
        {
            if(m_currentSubtree->subtree != nullptr)
            {
                // process input-values
                m_hierarchy = m_currentSubtree->hirarchy;
                overrideItems(m_parentValues, m_currentSubtree->subtree->values, ALL);
                overrideItems(m_parentValues, m_currentSubtree->items, ALL);

                // run the real task
                std::string errorMessage = "";
                const bool result = processSakuraItem(m_currentSubtree->subtree,
                                                      m_currentSubtree->filePath,
                                                      errorMessage);
                // handle result
                if(result) {
                    overrideItems(m_currentSubtree->items, m_parentValues, ONLY_EXISTING);
                } else {
                    m_currentSubtree->activeCounter->registerError(errorMessage);
                }

                // increase active-counter as last step, so the source subtree can check, if all
                // spawned subtrees are finished
                m_currentSubtree->activeCounter->increaseCounter();
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
SakuraThread::processSakuraItem(SakuraItem* sakuraItem,
                                const std::string &filePath,
                                std::string &errorMessage)
{
    // case that another thread has failed
    // only the failing thread return the false as result
    if(m_currentSubtree->activeCounter->success == false) {
        return true;
    }

    //----------------------------------------------------------------------------------------------
    if(sakuraItem->getType() == SakuraItem::SEQUENTIELL_ITEM)
    {
        SequentiellPart* sequential = dynamic_cast<SequentiellPart*>(sakuraItem);
        return processSequeniellPart(sequential, filePath, errorMessage);
    }
    //----------------------------------------------------------------------------------------------
    if(sakuraItem->getType() == SakuraItem::TREE_ITEM)
    {
        TreeItem* subtreeItem = dynamic_cast<TreeItem*>(sakuraItem);
        m_hierarchy.push_back("TREE: " + subtreeItem->id);
        const bool result = processTree(subtreeItem, errorMessage);
        m_hierarchy.pop_back();
        return result;
    }
    //----------------------------------------------------------------------------------------------
    if(sakuraItem->getType() == SakuraItem::SUBTREE_ITEM)
    {
        SubtreeItem* subtreeItem = dynamic_cast<SubtreeItem*>(sakuraItem);
        return processSubtree(subtreeItem, filePath, errorMessage);
    }
    //----------------------------------------------------------------------------------------------
    if(sakuraItem->getType() == SakuraItem::BLOSSOM_ITEM)
    {
        BlossomItem* blossomItem = dynamic_cast<BlossomItem*>(sakuraItem);
        return processBlossom(*blossomItem, filePath, errorMessage);
    }
    //----------------------------------------------------------------------------------------------
    if(sakuraItem->getType() == SakuraItem::BLOSSOM_GROUP_ITEM)
    {
        BlossomGroupItem* blossomGroupItem = dynamic_cast<BlossomGroupItem*>(sakuraItem);
        return processBlossomGroup(*blossomGroupItem, filePath, errorMessage);
    }
    //----------------------------------------------------------------------------------------------
    if(sakuraItem->getType() == SakuraItem::IF_ITEM)
    {
        IfBranching* ifBranching = dynamic_cast<IfBranching*>(sakuraItem);
        return processIf(ifBranching, filePath, errorMessage);
    }
    //----------------------------------------------------------------------------------------------
    if(sakuraItem->getType() == SakuraItem::FOR_EACH_ITEM)
    {
        ForEachBranching* forEachBranching = dynamic_cast<ForEachBranching*>(sakuraItem);
        return processForEach(forEachBranching, filePath, errorMessage);
    }
    //----------------------------------------------------------------------------------------------
    if(sakuraItem->getType() == SakuraItem::FOR_ITEM)
    {
        ForBranching* forBranching = dynamic_cast<ForBranching*>(sakuraItem);
        return processFor(forBranching, filePath, errorMessage);
    }
    //----------------------------------------------------------------------------------------------
    if(sakuraItem->getType() == SakuraItem::PARALLEL_ITEM)
    {
        ParallelPart* parallel = dynamic_cast<ParallelPart*>(sakuraItem);
        return processParallelPart(parallel, filePath, errorMessage);
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
SakuraThread::processBlossom(BlossomItem &blossomItem,
                             const std::string &filePath,
                             std::string &errorMessage)
{
    // only debug-output
    LOG_DEBUG("processBlossom: \n");
    LOG_DEBUG("    name: " + blossomItem.blossomName);

    // process values by filling with information of the parent-object
    const bool result = fillInputValueItemMap(blossomItem.values, m_parentValues, errorMessage);
    if(result == false)
    {
        errorMessage = createError(blossomItem,
                                   "processing",
                                   "error while processing blossom items:\n    "
                                   + errorMessage);
        blossomItem.success = false;

        return false;
    }

    LOG_DEBUG("    values:\n" + blossomItem.values.toString());

    // get and prcess the requested blossom
    Blossom* blossom = m_interface->getBlossom(blossomItem.blossomGroupType,
                                               blossomItem.blossomType);
    if(blossom == nullptr)
    {
        errorMessage = createError(blossomItem, "processing", "unknow blossom-type");
        blossomItem.success = false;
        return blossomItem.success;
    }

    // update blossom-item for processing
    blossomItem.blossomPath = filePath;

    blossom->growBlossom(blossomItem, errorMessage);

    // check result
    if(blossomItem.success == false) {
        return false;
    }

    // send result to root
    m_interface->printOutput(blossomItem);

    // write processing result back to parent
    if(fillInputValueItemMap(blossomItem.values,
                             blossomItem.blossomOutput,
                             errorMessage) == false)
    {
        return false;
    }

    // TODO: override only with the output-values to avoid unnecessary conflicts
    overrideItems(m_parentValues, blossomItem.values, ONLY_EXISTING);

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
SakuraThread::processBlossomGroup(BlossomGroupItem &blossomGroupItem,
                                  const std::string &filePath,
                                  std::string &errorMessage)
{
    LOG_DEBUG("processBlossomGroup");

    // convert name as jinja2-string
    std::string convertResult = "";
    const bool ret = m_interface->jinja2Converter->convert(convertResult,
                                                             blossomGroupItem.id,
                                                             &m_parentValues,
                                                             errorMessage);
    if(ret == false)
    {
        errorMessage = createError("jinja2-converter", errorMessage);
        return false;
    }

    // print blossom-group
    blossomGroupItem.id = convertResult;
    blossomGroupItem.nameHirarchie = m_hierarchy;
    blossomGroupItem.nameHirarchie.push_back("BLOSSOM-GROUP: " + blossomGroupItem.id);
    m_interface->printOutput(blossomGroupItem);

    // iterate over all blossoms of the group and process one after another
    for(BlossomItem* blossomItem : blossomGroupItem.blossoms)
    {
        // handle special-cass of a ressource-call
        TreeItem* tempItem = m_interface->garden->getRessource(blossomItem->blossomType);
        if(tempItem != nullptr)
        {
            const bool ret = runSubtreeCall(tempItem,
                                            blossomGroupItem.values,
                                            filePath,
                                            errorMessage);
            delete tempItem;

            return ret;
        }

        // update blossom-item with group-values for console-output
        blossomItem->blossomGroupType = blossomGroupItem.blossomGroupType;
        blossomItem->nameHirarchie = m_hierarchy;
        blossomItem->blossomName = blossomGroupItem.id;
        blossomItem->blossomGroupType = blossomGroupItem.blossomGroupType;
        blossomItem->nameHirarchie.push_back("BLOSSOM: " + blossomGroupItem.id);

        // copy values of the blossom-group into the blossom, but only values, which are not defined
        // which in the blossom
        overrideItems(blossomItem->values,
                      blossomGroupItem.values,
                      ONLY_NON_EXISTING);

        if(processBlossom(*blossomItem, filePath, errorMessage) == false) {
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
SakuraThread::processTree(TreeItem* treeItem,
                          std::string &errorMessage)
{
    LOG_DEBUG("processTree");

    // check if there are uninitialized items in the tree
    const std::vector<std::string> uninitItems = checkItems(m_parentValues);
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
    const std::string completePath = treeItem->rootPath + "/" + treeItem->relativePath;
    if(processSakuraItem(treeItem->childs, completePath, errorMessage) == false) {
        return false;
    }

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
SakuraThread::processSubtree(SubtreeItem* subtreeItem,
                             const std::string &filePath,
                             std::string &errorMessage)
{
    LOG_DEBUG("processSubtree");

    // get sakura-file based on the required path
    Kitsunemimi::Sakura::SakuraGarden* garden = m_interface->garden;
    const bfs::path relPath = garden->getRelativePath(filePath, subtreeItem->nameOrPath);
    SakuraItem* newSubtree = garden->getTree(relPath.string(), garden->rootPath);

    if(newSubtree == nullptr)
    {
        errorMessage = createError("subtree-processing",
                                   "subtree doesn't exist: " + subtreeItem->nameOrPath);
        return false;
    }

    const bool ret = runSubtreeCall(newSubtree,
                                    subtreeItem->values,
                                    filePath,
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
SakuraThread::processIf(IfBranching* ifCondition,
                        const std::string &filePath,
                        std::string &errorMessage)
{
    LOG_DEBUG("processIf");

    // initialize
    bool ifMatch = false;
    ValueItem valueItem;

    // get left side of the comparism
    if(fillValueItem(ifCondition->leftSide, m_parentValues, errorMessage) == false)
    {
        errorMessage = createError("subtree-processing",
                                   "error processing if-condition:\n"
                                   + errorMessage);
        return false;
    }

    // get right side of the comparism
    if(fillValueItem(ifCondition->rightSide, m_parentValues, errorMessage) == false)
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
        return processSakuraItem(ifCondition->ifContent, filePath, errorMessage);
    } else {
        return processSakuraItem(ifCondition->elseContent, filePath, errorMessage);
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
SakuraThread::processForEach(ForEachBranching* forEachItem,
                             const std::string &filePath,
                             std::string &errorMessage)
{
    LOG_DEBUG("processForEach");

    // initialize the array, over twhich the loop should iterate
    if(fillInputValueItemMap(forEachItem->iterateArray, m_parentValues, errorMessage) == false)
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
        result = runLoop(forEachItem->content,
                         forEachItem->values,
                         filePath,
                         forEachItem->tempVarName,
                         array,
                         errorMessage,
                         array->size());
    }
    else
    {
        result = m_interface->m_queue->spawnParallelSubtreesLoop(forEachItem->content,
                                                                 filePath,
                                                                 m_hierarchy,
                                                                 m_parentValues,
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
SakuraThread::processFor(ForBranching* forItem,
                         const std::string &filePath,
                         std::string &errorMessage)
{
    LOG_DEBUG("processFor");

    // get start-value
    if(fillValueItem(forItem->start, m_parentValues, errorMessage) == false)
    {
        errorMessage = createError("subtree-processing",
                                   "error processing for-loop:\n"
                                   + errorMessage);
        return false;
    }

    // get end-value
    if(fillValueItem(forItem->end, m_parentValues, errorMessage) == false)
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
        result = runLoop(forItem->content,
                         forItem->values,
                         filePath,
                         forItem->tempVarName,
                         nullptr,
                         errorMessage,
                         endValue,
                         startValue);
    }
    else
    {
        result = m_interface->m_queue->spawnParallelSubtreesLoop(forItem->content,
                                                                 filePath,
                                                                 m_hierarchy,
                                                                 m_parentValues,
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
SakuraThread::processSequeniellPart(SequentiellPart* subtree,
                                    const std::string &filePath,
                                    std::string &errorMessage)
{
    LOG_DEBUG("processSequeniellPart");

    for(SakuraItem* item : subtree->childs)
    {
        if(processSakuraItem(item, filePath, errorMessage) == false) {
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
SakuraThread::processParallelPart(ParallelPart* parallelPart,
                                  const std::string &filePath,
                                  std::string &errorMessage)
{
    LOG_DEBUG("processParallelPart");

    SequentiellPart* parts = dynamic_cast<SequentiellPart*>(parallelPart->childs);

    const bool result = m_interface->m_queue->spawnParallelSubtrees(parts->childs,
                                                                    filePath,
                                                                    m_hierarchy,
                                                                    m_parentValues,
                                                                    errorMessage,
                                                                    parts->childs.size());

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
 * @return true, if check successfull, else false
 */
bool
SakuraThread::runSubtreeCall(SakuraItem* newSubtree,
                             ValueItemMap &values,
                             const std::string &filePath,
                             std::string &errorMessage)
{
    LOG_DEBUG("runSubtreeCall");

    // fill values
    bool fillResult = fillInputValueItemMap(values, m_parentValues, errorMessage);
    if(fillResult == false)
    {
        errorMessage = createError("subtree-processing",
                                   "error while processing blossom items:\n"
                                   + errorMessage);
        return false;
    }

    // backup and reset parent
    DataMap parentBackup = m_parentValues;
    m_parentValues.clear();

    // set values
    overrideItems(newSubtree->values, values, ALL);
    overrideItems(m_parentValues, newSubtree->values, ALL);

    // process tree-item
    const bool ret = processSakuraItem(newSubtree, filePath, errorMessage);
    if(ret == false) {
        return false;
    }

    // write output back after restoring the parent-values to resume normally
    fillSubtreeOutputValueItemMap(newSubtree->values, &m_parentValues);
    m_parentValues = parentBackup;
    overrideItems(m_parentValues, newSubtree->values, ONLY_EXISTING);

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
 * @return true, if check successfull, else false
 */
bool
SakuraThread::runLoop(SakuraItem* loopContent,
                      const ValueItemMap &values,
                      const std::string &filePath,
                      const std::string &tempVarName,
                      DataArray* array,
                      std::string &errorMessage,
                      const uint64_t endPos,
                      const uint64_t startPos)
{
    // backup the parent-values to avoid permanent merging with loop-internal values
    DataMap preBalueBackup = m_parentValues;
    overrideItems(m_parentValues, values, ALL);

    for(uint64_t i = startPos; i < endPos; i++)
    {
        // add the counter-variable as new value to be accessable within the loop
        if(array != nullptr) {
            m_parentValues.insert(tempVarName, array->get(i)->copy(), true);
        } else {
            m_parentValues.insert(tempVarName, new DataValue(static_cast<long>(i)), true);
        }

        // process content
        SakuraItem* tempItem = loopContent->copy();
        if(processSakuraItem(tempItem, filePath, errorMessage) == false) {
            return false;
        }
        delete tempItem;
    }

    // restore the old parent values and update only the existing values with the one form the
    // loop. That way, variables like the counter-variable are not added to the parent.
    DataMap postBalueBackup = m_parentValues;
    m_parentValues = preBalueBackup;
    overrideItems(m_parentValues, postBalueBackup, ONLY_EXISTING);

    return true;
}

} // namespace Sakura
} // namespace Kitsunemimi

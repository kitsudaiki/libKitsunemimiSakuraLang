/**
 * @file        sakura_thread.h
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

#ifndef KITSUNEMIMI_SAKURA_LANG_THREAD_H
#define KITSUNEMIMI_SAKURA_LANG_THREAD_H

#include <thread>
#include <string>
#include <vector>

#include <processing/subtree_queue.h>
#include <items/sakura_items.h>

#include <libKitsunemimiCommon/threading/thread.h>
#include <libKitsunemimiCommon/common_items/data_items.h>

namespace Kitsunemimi
{
namespace Sakura
{
class SakuraLangInterface;

class SakuraThread
        : public Kitsunemimi::Thread
{
public:
    SakuraThread(SakuraLangInterface* interface);

private:
    bool m_started = false;
    SakuraLangInterface* m_interface;
    SubtreeQueue::SubtreeObject* m_currentSubtree = nullptr;

    DataMap m_parentValues;
    std::vector<std::string> m_hierarchy;

    void run();

    bool processSakuraItem(SakuraItem* sakuraItem,
                           const std::string &filePath,
                           std::string &errorMessage);

    bool processBlossom(BlossomItem &blossomItem,
                        const std::string &filePath,
                        std::string &errorMessage);
    bool processBlossomGroup(BlossomGroupItem &blossomGroupItem,
                             const std::string &filePath,
                             std::string &errorMessage);
    bool processTree(TreeItem* treeItem,
                     std::string &errorMessage);
    bool processSubtree(SubtreeItem* subtreeItem,
                        const std::string &filePath,
                        std::string &errorMessage);
    bool processIf(IfBranching* ifCondition,
                   const std::string &filePath,
                   std::string &errorMessage);
    bool processForEach(ForEachBranching* forEachItem,
                        const std::string &filePath,
                        std::string &errorMessage);
    bool processFor(ForBranching* forItem,
                    const std::string &filePath,
                    std::string &errorMessage);
    bool processSequeniellPart(SequentiellPart* subtree,
                               const std::string &filePath,
                               std::string &errorMessage);
    bool processParallelPart(ParallelPart* parallelPart,
                             const std::string &filePath,
                             std::string &errorMessage);

    bool runSubtreeCall(SakuraItem* newSubtree,
                        ValueItemMap &values,
                        const std::string &filePath,
                        std::string &errorMessage);
    bool runLoop(SakuraItem* loopContent,
                 const ValueItemMap &values,
                 const std::string &filePath,
                 const std::string &tempVarName,
                 DataArray* array,
                 std::string &errorMessage,
                 const uint64_t endPos,
                 const uint64_t startPos = 0);
};

} // namespace Sakura
} // namespace Kitsunemimi

#endif // KITSUNEMIMI_SAKURA_LANG_THREAD_H

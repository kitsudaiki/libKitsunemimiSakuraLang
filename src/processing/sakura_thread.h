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
    SakuraThread(SakuraLangInterface* interface,
                 const std::string &threadName);

private:
    bool m_started = false;
    SakuraLangInterface* m_interface;

    void run();

    bool processSakuraItem(GrowthPlan* plan,
                           SakuraItem* sakuraItem);

    bool processBlossom(GrowthPlan* plan,
                        BlossomItem &blossomItem);
    bool processBlossomGroup(GrowthPlan* plan,
                             BlossomGroupItem &blossomGroupItem);
    bool processTree(GrowthPlan* plan,
                     TreeItem* treeItem);
    bool processSubtree(GrowthPlan* plan,
                        SubtreeItem* subtreeItem);
    bool processIf(GrowthPlan* plan,
                   IfBranching* ifCondition);
    bool processForEach(GrowthPlan* plan,
                        ForEachBranching* forEachItem);
    bool processFor(GrowthPlan* plan,
                    ForBranching* forItem);
    bool processSequeniellPart(GrowthPlan* plan,
                               SequentiellPart* subtree);
    bool processParallelPart(GrowthPlan* plan,
                             ParallelPart* parallelPart);

    bool runSubtreeCall(GrowthPlan* plan,
                        SakuraItem* newSubtree,
                        ValueItemMap &values);
    bool runLoop(GrowthPlan* plan,
                 SakuraItem* loopContent,
                 const ValueItemMap &values,
                 const std::string &tempVarName,
                 DataArray* array,
                 const uint64_t endPos,
                 const uint64_t startPos = 0);
};

} // namespace Sakura
} // namespace Kitsunemimi

#endif // KITSUNEMIMI_SAKURA_LANG_THREAD_H

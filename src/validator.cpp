/**
 * @file        validator.cpp
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

#include "validator.h"

#include <items/item_methods.h>
#include <sakura_garden.h>

#include <libKitsunemimiSakuraLang/sakura_lang_interface.h>
#include <libKitsunemimiSakuraLang/blossom.h>

namespace Kitsunemimi
{
namespace Sakura
{

/**
 * @brief constructor
 */
Validator::Validator() {}

/**
 * @brief destructor
 */
Validator::~Validator() {}

/**
 * @brief check content of a blossom-item
 *
 * @param blossomItem blossom-item with the values, which should be validated
 * @param filePath path of the file, which will be actually checked
 * @param errorMessage reference for error-message
 *
 * @return true, if check successful, else false
 */
bool
Validator::checkBlossomItem(BlossomItem &blossomItem,
                            const std::string &filePath,
                            std::string &errorMessage)
{
    SakuraLangInterface* interface = SakuraLangInterface::getInstance();

    // check if the object is a resource and skip check
    TreeItem* item = interface->m_garden->getRessource(blossomItem.blossomType);
    if(item != nullptr) {
        return true;
    }

    // get blossom by type and group-type
    Blossom* blossom = interface->getBlossom(blossomItem.blossomGroupType,
                                               blossomItem.blossomType);
    if(blossom == nullptr)
    {
        errorMessage = createError(blossomItem, filePath, "validator", "unknow blossom-type");
        return false;
    }

    return blossom->validateInput(blossomItem, filePath, errorMessage);
}

/**
 * @brief check all items of a tree recursively and validate all found blossom-items
 *
 * @param sakuraItem item to check
 * @param filePath path of the file, which will be actually checked
 * @param errorMessage reference for error-message
 *
 * @return true, if check successful, else false
 */
bool
Validator::checkSakuraItem(SakuraItem* sakuraItem,
                           const std::string &filePath,
                           std::string &errorMessage)
{
    //----------------------------------------------------------------------------------------------
    if(sakuraItem->getType() == SakuraItem::SEQUENTIELL_ITEM)
    {
        SequentiellPart* sequential = dynamic_cast<SequentiellPart*>(sakuraItem);
        for(SakuraItem* item : sequential->childs)
        {
            if(checkSakuraItem(item,
                               filePath,
                               errorMessage) == false)
            {
                return false;
            }
        }
        return true;
    }
    //----------------------------------------------------------------------------------------------
    if(sakuraItem->getType() == SakuraItem::TREE_ITEM)
    {
        TreeItem* treeItem = dynamic_cast<TreeItem*>(sakuraItem);
        const std::string completePath = treeItem->rootPath + "/" + treeItem->relativePath;
        return checkSakuraItem(treeItem->childs,
                               completePath,
                               errorMessage);
    }
    //----------------------------------------------------------------------------------------------
    if(sakuraItem->getType() == SakuraItem::SUBTREE_ITEM)
    {
        return true;
    }
    //----------------------------------------------------------------------------------------------
    if(sakuraItem->getType() == SakuraItem::BLOSSOM_ITEM)
    {
        BlossomItem* blossomItem = dynamic_cast<BlossomItem*>(sakuraItem);
        return checkBlossomItem(*blossomItem,
                                filePath,
                                errorMessage);
    }
    //----------------------------------------------------------------------------------------------
    if(sakuraItem->getType() == SakuraItem::BLOSSOM_GROUP_ITEM)
    {
        BlossomGroupItem* blossomGroupItem = dynamic_cast<BlossomGroupItem*>(sakuraItem);
        for(BlossomItem* blossomItem : blossomGroupItem->blossoms)
        {
            blossomItem->blossomGroupType = blossomGroupItem->blossomGroupType;
            blossomItem->blossomName = blossomGroupItem->id;

            overrideItems(blossomItem->values,
                          blossomGroupItem->values,
                          ONLY_NON_EXISTING);

            if(checkSakuraItem(blossomItem,
                               filePath,
                               errorMessage) == false)
            {
                return false;
            }
        }

        return true;
    }
    //----------------------------------------------------------------------------------------------
    if(sakuraItem->getType() == SakuraItem::IF_ITEM)
    {
        IfBranching* ifBranching = dynamic_cast<IfBranching*>(sakuraItem);
        if(checkSakuraItem(ifBranching->ifContent,
                           filePath,
                           errorMessage) == false)
        {
            return false;
        }

        if(checkSakuraItem(ifBranching->elseContent,
                           filePath,

                           errorMessage) == false)
        {
            return false;
        }

        return true;
    }
    //----------------------------------------------------------------------------------------------
    if(sakuraItem->getType() == SakuraItem::FOR_EACH_ITEM)
    {
        ForEachBranching* forEachBranching = dynamic_cast<ForEachBranching*>(sakuraItem);
        return checkSakuraItem(forEachBranching->content,
                               filePath,
                               errorMessage);
    }
    //----------------------------------------------------------------------------------------------
    if(sakuraItem->getType() == SakuraItem::FOR_ITEM)
    {
        ForBranching* forBranching = dynamic_cast<ForBranching*>(sakuraItem);
        return checkSakuraItem(forBranching->content,
                               filePath,
                               errorMessage);
    }
    //----------------------------------------------------------------------------------------------
    if(sakuraItem->getType() == SakuraItem::PARALLEL_ITEM)
    {
        ParallelPart* parallel = dynamic_cast<ParallelPart*>(sakuraItem);
        return checkSakuraItem(parallel->childs,
                               filePath,
                               errorMessage);
    }
    //----------------------------------------------------------------------------------------------

    // This case should never appear. It can't be produced by the parser at the moment,
    // so if this assert is called, there so something totally wrong in the implementation
    assert(false);

    return false;
}

/**
 * @brief check all blossom-items of all parsed trees
 *
 * @param errorMessage reference for error-message
 *
 * @return true, if check successful, else false
 */
bool
Validator::checkAllItems(std::string &errorMessage)
{
    SakuraLangInterface* interface = SakuraLangInterface::getInstance();

    std::map<std::string, TreeItem*>::const_iterator mapIt;
    for(mapIt = interface->m_garden->m_trees.begin();
        mapIt != interface->m_garden->m_trees.end();
        mapIt++)
    {
        if(checkSakuraItem(mapIt->second,
                           mapIt->second->relativePath,
                           errorMessage) == false)
        {
            return false;
        }
    }

    return true;
}

} // namespace Sakura
} // namespace Kitsunemimi

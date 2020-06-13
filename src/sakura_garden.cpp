/**
 * @file        sakura_garden.cpp
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

#include <libKitsunemimiSakuraParser/sakura_garden.h>
#include <libKitsunemimiSakuraParser/sakura_items.h>

namespace Kitsunemimi
{
namespace Sakura
{

/**
 * @brief SakuraGarden::SakuraGarden
 */
SakuraGarden::SakuraGarden()
{
}

/**
 * @brief SakuraGarden::~SakuraGarden
 */
SakuraGarden::~SakuraGarden()
{
    // TODO
}

/**
 * @brief SakuraGarden::getTreeById
 * @param id
 * @return
 */
TreeItem*
SakuraGarden::getTreeById(const std::string id)
{
    std::map<std::string, TreeItem*>::const_iterator it;
    for(it = trees.begin();
        it != trees.end();
        it++)
    {
        if(it->second->id == id) {
            return it->second;
        }
    }

    return nullptr;
}

/**
 * @brief SakuraGarden::getTreeByPath
 * @param relativePath
 * @return
 */
TreeItem*
SakuraGarden::getTreeByPath(const std::string relativePath)
{
    std::map<std::string, TreeItem*>::const_iterator it;
    it = trees.find(relativePath);

    if(it != trees.end()) {
        return it->second;
    }

    return nullptr;
}

/**
 * @brief SakuraGarden::getTemplate
 * @param relativePath
 * @return
 */
const std::string
SakuraGarden::getTemplate(const std::string relativePath)
{
    std::map<std::string, std::string>::const_iterator it;
    it = templates.find(relativePath);

    if(it != templates.end()) {
        return it->second;
    }

    return "";
}

/**
 * @brief SakuraGarden::getFile
 * @param relativePath
 * @return
 */
Kitsunemimi::DataBuffer*
SakuraGarden::getFile(const std::string relativePath)
{
    std::map<std::string, Kitsunemimi::DataBuffer*>::const_iterator it;
    it = files.find(relativePath);

    if(it != files.end()) {
        return it->second;
    }

    return nullptr;
}

}
}

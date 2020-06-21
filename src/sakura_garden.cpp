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
#include <libKitsunemimiSakuraParser/sakura_parsing.h>

#include <libKitsunemimiCommon/common_methods/string_methods.h>

#include <libKitsunemimiPersistence/logger/logger.h>
#include <libKitsunemimiPersistence/files/file_methods.h>

namespace Kitsunemimi
{
namespace Sakura
{

/**
 * @brief SakuraGarden::SakuraGarden
 */
SakuraGarden::SakuraGarden()
{
    m_parser = new SakuraParsing(false);
}

/**
 * @brief SakuraGarden::~SakuraGarden
 */
SakuraGarden::~SakuraGarden()
{
    // TODO
}

/**
 * @brief parse and add new tree
 *
 * @param treePath absolut file-path
 * @param errorMessage reference for error-message
 *
 * @return true, if successful, else false
 */
bool
SakuraGarden::addTree(const std::string &treePath,
                      std::string &errorMessage)
{
    // parse all files and convert the into
    const bool treeParseResult = m_parser->parseFiles(*this, treePath, errorMessage);
    if(treeParseResult == false) {
        return false;
    }

    return true;
}

/**
 * @brief SakuraGarden::addResource
 * @param content
 * @param errorMessage
 * @return
 */
bool
SakuraGarden::addResource(const std::string &content,
                          std::string &errorMessage)
{
    SakuraItem* parsedItem = m_parser->parseString(content, errorMessage);
    if(parsedItem == nullptr) {
        return false;
    }

    // add new item to the map
    TreeItem* treeItem = static_cast<TreeItem*>(parsedItem);
    treeItem->unparsedConent = content;
    resources.insert(std::make_pair(treeItem->id, treeItem));

    return true;
}

/**
 * @brief convert path, which is relative to a tree-file, into a path, which is relative to the
 *        root-path.
 *
 * @param blossomFilePath absolut path of the file of the blossom
 * @param blossomInternalRelPath relative path, which is called inside of the blossom
 *
 * @return path, which is relative to the root-path.
 */
const std::string
SakuraGarden::getRelativePath(const std::string &blossomFilePath,
                              const std::string &blossomInternalRelPath)
{
    // create source-path
    const std::string parentPath = Kitsunemimi::Persistence::getParent(blossomFilePath);
    const std::string relativePath = Kitsunemimi::Persistence::getRelativePath(parentPath,
                                                                               rootPath);

    // build new relative path for the new file-request
    if(relativePath == ".") {
       return blossomInternalRelPath;
    } else {
        return relativePath + "/" + blossomInternalRelPath;;
    }
}

/**
 * @brief get a fully parsed tree
 *
 * @param relativePath relative path of the tree (Default: empty string)
 * @param rootPath root-path of the requested tree
 *
 * @return requested pointer to tree-item
 */
TreeItem*
SakuraGarden::getTree(const std::string &relativePath,
                      const std::string &rootPath)
{
    // build complete file-path
    std::string completePath = relativePath;
    if(rootPath != "") {
        completePath = rootPath + "/" + relativePath;
    }

    // get tree-item based on the path
    if(Kitsunemimi::Persistence::isDir(completePath))
    {
        if(relativePath == "") {
            return getTreeByPath("root.tree");
        } else {
            return getTreeByPath(relativePath + "/root.tree");
        }
    }
    else
    {
        return getTreeByPath(relativePath);
    }
}

/**
 * @brief SakuraGarden::getRessource
 * @param id
 * @return
 */
TreeItem*
SakuraGarden::getRessource(const std::string &id)
{
    std::map<std::string, TreeItem*>::const_iterator it;
    for(it = resources.begin();
        it != resources.end();
        it++)
    {
        if(it->second->id == id) {
            return it->second;
        }
    }

    return nullptr;
}

/**
 * @brief SakuraGarden::getTreeById
 * @param id
 * @return
 */
TreeItem*
SakuraGarden::getTreeById(const std::string &id)
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
SakuraGarden::getTreeByPath(const std::string &relativePath)
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
SakuraGarden::getTemplate(const std::string &relativePath)
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
SakuraGarden::getFile(const std::string &relativePath)
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

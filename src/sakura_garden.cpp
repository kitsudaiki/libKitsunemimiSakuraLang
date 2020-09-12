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

#include "sakura_garden.h"

#include <items/sakura_items.h>
#include <parsing/sakura_parsing.h>

#include <libKitsunemimiCommon/common_methods/string_methods.h>

#include <libKitsunemimiPersistence/logger/logger.h>
#include <libKitsunemimiPersistence/files/file_methods.h>

namespace Kitsunemimi
{
namespace Sakura
{

/**
 * @brief constructor
 *
 * @param enableDebug true to enable debugging of the parser
 */
SakuraGarden::SakuraGarden(const bool enableDebug)
{
    m_parser = new SakuraParsing(enableDebug);
}

/**
 * @brief destructor
 */
SakuraGarden::~SakuraGarden()
{
    delete m_parser;
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
SakuraGarden::addTree(const bfs::path &treePath,
                      std::string &errorMessage)
{
    // parse all files and convert the into
    const bool treeParseResult = m_parser->parseTreeFiles(*this, treePath, errorMessage);
    if(treeParseResult == false) {
        return false;
    }

    return true;
}

/**
 * @brief add a tree as resource
 *
 * @param content string to parse and add
 * @param treePath absolut file-path
 * @param errorMessage reference for error-message
 *
 * @return true, if successful, else false
 */
bool
SakuraGarden::addResource(const std::string &content,
                          const bfs::path &treePath,
                          std::string &errorMessage)
{
    // parse all files and convert the into
    const bool treeParseResult = m_parser->parseRessourceString(*this,
                                                                treePath.string(),
                                                                content,
                                                                errorMessage);
    if(treeParseResult == false) {
        return false;
    }

    return true;
}

/**
 * @brief convert path, which is relative to a sakura-file, into a path, which is relative to the
 *        root-path.
 *
 * @param blossomFilePath absolut path of the file of the blossom
 * @param blossomInternalRelPath relative path, which is called inside of the blossom
 *
 * @return path, which is relative to the root-path.
 */
const bfs::path
SakuraGarden::getRelativePath(const bfs::path &blossomFilePath,
                              const bfs::path &blossomInternalRelPath)
{
    // create source-path
    const bfs::path parentPath = blossomFilePath.parent_path();
    const bfs::path relativePath = bfs::relative(parentPath, rootPath);

    // build new relative path for the new file-request
    if(relativePath == ".") {
        return blossomInternalRelPath;
    } else {
        return relativePath / blossomInternalRelPath;;
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
    bfs::path completePath = relativePath;
    if(relativePath != "") {
        completePath = bfs::path(rootPath) / relativePath;
    }

    // get tree-item based on the path
    if(bfs::is_directory(completePath))
    {
        if(relativePath == "") {
            return getTree(bfs::path("root.sakura").string());
        } else {
            return getTree((bfs::path(relativePath) / bfs::path("root.sakura")).string());
        }
    }
    else
    {
        return getTree(relativePath);
    }
}

/**
 * @brief request a resource
 *
 * @param id name of the resource
 *
 * @return copy of the tree-item, if id exist, else nullptr
 */
TreeItem*
SakuraGarden::getRessource(const std::string &id)
{
    std::map<std::string, TreeItem*>::const_iterator it;
    it = resources.find(id);

    if(it != resources.end())
    {
        TreeItem* result = dynamic_cast<TreeItem*>(it->second->copy());
        assert(result != nullptr);
        return result;
    }

    return nullptr;
}

/**
 * @brief request a tree
 *
 * @param id name of the tree
 *
 * @return copy of the tree-item, if id exist, else nullptr
 */
TreeItem*
SakuraGarden::getTree(const std::string &id)
{
    std::map<std::string, TreeItem*>::const_iterator it;
    it = trees.find(id);

    if(it != trees.end())
    {
        TreeItem* result = dynamic_cast<TreeItem*>(it->second->copy());
        assert(result != nullptr);
        return result;
    }

    return nullptr;
}

/**
 * @brief request template
 *
 * @param id id of the template
 *
 * @return template, if id found, else empty string
 */
const std::string
SakuraGarden::getTemplate(const std::string &id)
{
    std::map<std::string, std::string>::const_iterator it;
    it = templates.find(id);

    if(it != templates.end()) {
        return it->second;
    }

    return "";
}

/**
 * @brief request file
 *
 * @param id id of the file
 *
 * @return file as data-buffer, if id found, else nullptr
 */
Kitsunemimi::DataBuffer*
SakuraGarden::getFile(const std::string &id)
{
    std::map<std::string, Kitsunemimi::DataBuffer*>::const_iterator it;
    it = files.find(id);

    if(it != files.end()) {
        return it->second;
    }

    return nullptr;
}

} // namespace Sakura
} // namespace Kitsunemimi

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

#include <libKitsunemimiCommon/common_methods/string_methods.h>

#include <libKitsunemimiCommon/logger.h>
#include <libKitsunemimiCommon/common_methods/file_methods.h>

namespace Kitsunemimi
{
namespace Sakura
{

/**
 * @brief constructor
 */
SakuraGarden::SakuraGarden() {}

/**
 * @brief destructor
 */
SakuraGarden::~SakuraGarden() {}

/**
 * @brief convert path, which is relative to a sakura-file, into a path, which is relative to the
 *        root-path.
 *
 * @param blossomFilePath absolut path of the file of the blossom
 * @param blossomInternalRelPath relative path, which is called inside of the blossom
 *
 * @return path, which is relative to the root-path.
 */
const std::filesystem::path
SakuraGarden::getRelativePath(const std::filesystem::path &blossomFilePath,
                              const std::filesystem::path &blossomInternalRelPath)
{
    // create source-path
    const std::filesystem::path parentPath = blossomFilePath.parent_path();
    const std::filesystem::path relativePath = std::filesystem::relative(parentPath, rootPath);

    // build new relative path for the new file-request
    if(relativePath == ".") {
        return blossomInternalRelPath;
    } else {
        return relativePath / blossomInternalRelPath;;
    }
}

/**
 * @brief add new tree
 *
 * @param id id of the new tree
 * @param tree new tree
 *
 * @return false, if id already exist, else true
 */
bool
SakuraGarden::addTree(const std::string &id,
                      TreeItem* tree)
{
    // check if already exist
    std::map<std::string, TreeItem*>::const_iterator it;
    it = m_trees.find(id);
    if(it != m_trees.end()) {
        return false;
    }

    // add
    LOG_DEBUG("register new tree with id: " + id);
    m_trees.insert(std::make_pair(id, tree));

    return true;
}

/**
 * @brief add new resource
 *
 * @param id id of the new resource
 * @param resource new resource
 *
 * @return false, if id already exist, else true
 */
bool
SakuraGarden::addResource(const std::string &id,
                          TreeItem* resource)
{
    // check if already exist
    std::map<std::string, TreeItem*>::const_iterator it;
    it = m_resources.find(id);
    if(it != m_resources.end()) {
        return false;
    }

    // add
    LOG_DEBUG("register new ressource with id: " + id);
    m_resources.insert(std::make_pair(id, resource));

    return true;
}

/**
 * @brief add new template
 *
 * @param id id of the new template
 * @param templateContent content of the template
 *
 * @return false, if id already exist, else true
 */
bool
SakuraGarden::addTemplate(const std::string &id,
                          const std::string &templateContent)
{
    // check if already exist
    std::map<std::string, std::string>::const_iterator it;
    it = m_templates.find(id);
    if(it != m_templates.end()) {
        return false;
    }

    // add
    LOG_DEBUG("register new template with id: " + id);
    m_templates.insert(std::make_pair(id, templateContent));

    return true;
}

/**
 * @brief add new file
 *
 * @param id id of the new file
 * @param fileContent file-conent
 *
 * @return false, if id already exist, else true
 */
bool
SakuraGarden::addFile(const std::string &id,
                      DataBuffer* fileContent)
{
    // check if already exist
    std::map<std::string, DataBuffer*>::const_iterator it;
    it = m_files.find(id);
    if(it != m_files.end()) {
        return false;
    }

    // add
    LOG_DEBUG("register new file with id: " + id);
    m_files.insert(std::make_pair(id, fileContent));

    return true;
}

/**
 * @brief SakuraGarden::containsTree
 * @param id
 * @return
 */
bool
SakuraGarden::containsTree(std::string id)
{
    if(id == "") {
       id = "root.sakura";
    }

    std::map<std::string, TreeItem*>::const_iterator it;
    it = m_trees.find(id);
    if(it != m_trees.end()) {
        return true;
    }

    return false;
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
    it = m_resources.find(id);
    if(it != m_resources.end()) {
        return dynamic_cast<TreeItem*>(it->second->copy());
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
SakuraGarden::getTree(std::string id)
{
    if(id == "") {
       id = "root.sakura";
    }

    std::map<std::string, TreeItem*>::const_iterator it;
    it = m_trees.find(id);
    if(it != m_trees.end()) {
        return dynamic_cast<TreeItem*>(it->second->copy());
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
    it = m_templates.find(id);

    if(it != m_templates.end()) {
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
    it = m_files.find(id);

    if(it != m_files.end()) {
        return it->second;
    }

    return nullptr;
}

} // namespace Sakura
} // namespace Kitsunemimi

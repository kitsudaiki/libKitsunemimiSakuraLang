/**
 * @file       sakura_lang_interface.cpp
 *
 * @author     Tobias Anker <tobias.anker@kitsunemimi.moe>
 *
 * @copyright  Apache License Version 2.0
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

#include <libKitsunemimiSakuraLang/sakura_lang_interface.h>

#include <sakura_garden.h>
#include <validator.h>

#include <processing/subtree_queue.h>
#include <processing/thread_pool.h>

#include <items/item_methods.h>

#include <libKitsunemimiJinja2/jinja2_converter.h>
#include <libKitsunemimiPersistence/logger/logger.h>

namespace Kitsunemimi
{
namespace Sakura
{

SakuraLangInterface::SakuraLangInterface()
{
    m_garden = new SakuraGarden();
    m_queue = new SubtreeQueue();
    m_jinja2Converter = new Kitsunemimi::Jinja2::Jinja2Converter();
    // TODO: make number of threads configurable
    m_threadPoos = new ThreadPool(6, this);
}

SakuraLangInterface::~SakuraLangInterface()
{
    delete m_garden;
    delete m_queue;
    delete m_threadPoos;
    delete m_jinja2Converter;
}

/**
 * @brief SakuraLangInterface::processFiles
 * @param inputPath
 * @param initialValues
 * @param dryRun
 * @return
 */
bool
SakuraLangInterface::processFiles(const std::string &inputPath,
                                  const DataMap &initialValues,
                                  const bool dryRun)
{
    std::string errorMessage = "";

    // set default-file in case that a directory instead of a file was selected
    std::string treeFile = inputPath;
    if(bfs::is_directory(treeFile)) {
        treeFile = treeFile + "/root.sakura";
    }

    // parse all files
    if(m_garden->addTree(treeFile, errorMessage) == false)
    {
        LOG_ERROR("failed to add trees\n    " + errorMessage);
        return false;
    }

    SakuraItem* tree = nullptr;

    // get initial sakura-file
    if(bfs::is_regular_file(treeFile))
    {
        const bfs::path parent = bfs::path(treeFile).parent_path();
        const std::string relPath = bfs::relative(treeFile, parent).string();

        tree = m_garden->getTree(relPath, parent.string());
    }

    if(tree == nullptr)
    {
        LOG_ERROR("No tree found for the input-path " + treeFile);
        return false;
    }

    // check if input-values match with the first tree
    const std::vector<std::string> failedInput = checkInput(tree->values, initialValues);
    if(failedInput.size() > 0)
    {
        std::string errorMessage = "Following input-values are not valid for the initial tress:\n";
        for(const std::string& item : failedInput)
        {
            errorMessage += "    " + item + "\n";
        }
        LOG_ERROR(errorMessage);
        return false;
    }

    // validate parsed blossoms
    errorMessage = "";
    if(checkAllItems(this, *m_garden, errorMessage) == false)
    {
        LOG_ERROR("\n" + errorMessage);
        return false;
    }

    // in case of a dry-run, cancel here before executing the scripts
    if(dryRun)
    {
        LOG_INFO("dry-run successfully finished", GREEN_COLOR);
        return true;
    }

    // process sakura-file with initial values
    errorMessage = "";
    if(runProcess(tree, initialValues, errorMessage) == false)
    {
        LOG_ERROR("\n" + errorMessage);
        return false;
    }

    LOG_INFO("finish", GREEN_COLOR);

    return true;
}

/**
 * @brief SakuraLangInterface::doesBlossomExist
 * @param groupName
 * @param itemName
 * @return
 */
bool
SakuraLangInterface::doesBlossomExist(const std::string &groupName,
                                      const std::string &itemName)
{
    std::map<std::string, std::map<std::string, Blossom*>>::const_iterator groupIt;
    groupIt = m_registeredBlossoms.find(groupName);

    if(groupIt != m_registeredBlossoms.end())
    {
        std::map<std::string, Blossom*>::const_iterator itemIt;
        itemIt = groupIt->second.find(itemName);

        if(itemIt != groupIt->second.end()) {
            return true;
        }
    }

    return false;
}

/**
 * @brief SakuraLangInterface::addBlossom
 * @param groupName
 * @param itemName
 * @param newBlossom
 * @return
 */
bool
SakuraLangInterface::addBlossom(const std::string &groupName,
                                const std::string &itemName,
                                Blossom* newBlossom)
{
    if(doesBlossomExist(groupName, itemName) == true) {
        return false;
    }

    std::map<std::string, std::map<std::string, Blossom*>>::iterator groupIt;
    groupIt = m_registeredBlossoms.find(groupName);

    if(groupIt == m_registeredBlossoms.end())
    {
        std::map<std::string, Blossom*> newMap;
        m_registeredBlossoms.insert(std::make_pair(groupName, newMap));
    }

    groupIt = m_registeredBlossoms.find(groupName);
    groupIt->second.insert(std::make_pair(itemName, newBlossom));

    return false;
}

/**
 * @brief SakuraLangInterface::getBlossom
 * @param groupName
 * @param itemName
 * @return
 */
Blossom*
SakuraLangInterface::getBlossom(const std::string &groupName,
                                const std::string &itemName)
{
    std::map<std::string, std::map<std::string, Blossom*>>::const_iterator groupIt;
    groupIt = m_registeredBlossoms.find(groupName);

    if(groupIt != m_registeredBlossoms.end())
    {
        std::map<std::string, Blossom*>::const_iterator itemIt;
        itemIt = groupIt->second.find(itemName);

        if(itemIt != groupIt->second.end()) {
            return itemIt->second;
        }
    }

    return nullptr;
}

/**
 * @brief SakuraRoot::runProcess
 * @return
 */
bool
SakuraLangInterface::runProcess(SakuraItem* item,
                                const DataMap &initialValues,
                                std::string &errorMessage)
{
    std::vector<SakuraItem*> childs;
    childs.push_back(item);
    std::vector<std::string> hierarchy;

    const bool result = m_queue->spawnParallelSubtrees(childs,
                                                       0,
                                                       1,
                                                       "",
                                                       hierarchy,
                                                       initialValues,
                                                       errorMessage);
    return result;
}

}
}

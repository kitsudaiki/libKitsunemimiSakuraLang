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

/**
 * @brief constructor
 *
 * @param enableDebug set to true to enable the debug-output of the parser
 */
SakuraLangInterface::SakuraLangInterface(const bool enableDebug)
{
    m_garden = new SakuraGarden(enableDebug);
    m_queue = new SubtreeQueue();
    jinja2Converter = new Kitsunemimi::Jinja2::Jinja2Converter();
    // TODO: make number of threads configurable
    m_threadPoos = new ThreadPool(6, this);
}

/**
 * @brief destructor
 */
SakuraLangInterface::~SakuraLangInterface()
{
    delete m_garden;
    delete m_queue;
    delete m_threadPoos;
    delete jinja2Converter;
}

/**
 * @brief process set of sakura-files
 *
 * @param inputPath path to the initial sakura-file or directory with the root.sakura file
 * @param initialValues map-item with initial values to override the items of the initial tree-item
 * @param dryRun set to true to only parse and check the files, without executing them
 * @param errorMessage reference for error-message
 *
 * @return true, if successfule, else false
 */
bool
SakuraLangInterface::processFiles(const std::string &inputPath,
                                  const DataMap &initialValues,
                                  const bool dryRun,
                                  std::string &errorMessage)
{
    // precheck input
    if(bfs::is_regular_file(inputPath) == false
            && bfs::is_directory(inputPath) == false)
    {
        errorMessage = "Not a regular file or directory as input-path " + inputPath;
        return false;
    }

    // set default-file in case that a directory instead of a file was selected
    std::string treeFile = inputPath;
    if(bfs::is_directory(treeFile)) {
        treeFile = treeFile + "/root.sakura";
    }

    // parse all files
    if(m_garden->addTree(treeFile, errorMessage) == false)
    {
        errorMessage = "failed to add trees\n    " + errorMessage;
        return false;
    }

    // get relative path from the input-path
    const bfs::path parent = bfs::path(treeFile).parent_path();
    const std::string relPath = bfs::relative(treeFile, parent).string();

    // get initial tree-item
    TreeItem* tree = m_garden->getTree(relPath, parent.string());
    if(tree == nullptr)
    {
        errorMessage = "No tree found for the input-path " + treeFile;
        return false;
    }

    // check if input-values match with the first tree
    const std::vector<std::string> failedInput = checkInput(tree->values, initialValues);
    if(failedInput.size() > 0)
    {
        errorMessage = "Following input-values are not valid for the initial tress:\n";

        for(const std::string& item : failedInput) {
            errorMessage += "    " + item + "\n";
        }

        return false;
    }

    // validate parsed blossoms
    Validator validator;
    if(validator.checkAllItems(this, errorMessage) == false) {
        return false;
    }

    // in case of a dry-run, cancel here before executing the scripts
    if(dryRun) {
        return true;
    }

    // process sakura-file with initial values
    if(runProcess(tree, initialValues, errorMessage) == false) {
        return false;
    }

    return true;
}

/**
 * @brief get template-string
 *
 * @param relativePath relative path of the template as identification
 *
 * @return template as string or empty string, if no template found for the given path
 */
const std::string
SakuraLangInterface::getTemplate(const std::string &relativePath)
{
    return m_garden->getTemplate(relativePath);
}

/**
 * @brief get file as data-buffer
 *
 * @param relativePath relative path of the file as identification
 *
 * @return file as data-buffer or nullptr, if no file found for the given path
 */
DataBuffer*
SakuraLangInterface::getFile(const std::string &relativePath)
{
    return m_garden->getFile(relativePath);
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
SakuraLangInterface::getRelativePath(const bfs::path &blossomFilePath,
                                     const bfs::path &blossomInternalRelPath)
{
    return m_garden->getRelativePath(blossomFilePath, blossomInternalRelPath);
}

/**
 * @brief check if a specific blossom was registered
 *
 * @param groupName group-identifier of the blossom
 * @param itemName item-identifier of the blossom
 *
 * @return true, if blossom with the given group- and item-name exist, else false
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
 *
 * @param groupName group-identifier of the blossom
 * @param itemName item-identifier of the blossom
 * @param newBlossom pointer to the new blossom
 *
 * @return true, if blossom was registered or false, if the group- and item-name are already
 *         registered
 */
bool
SakuraLangInterface::addBlossom(const std::string &groupName,
                                const std::string &itemName,
                                Blossom* newBlossom)
{
    // check if already used
    if(doesBlossomExist(groupName, itemName) == true) {
        return false;
    }

    std::map<std::string, std::map<std::string, Blossom*>>::iterator groupIt;
    groupIt = m_registeredBlossoms.find(groupName);

    // create internal group-map, if not already exist
    if(groupIt == m_registeredBlossoms.end())
    {
        std::map<std::string, Blossom*> newMap;
        m_registeredBlossoms.insert(std::make_pair(groupName, newMap));
    }

    // add item to group
    groupIt = m_registeredBlossoms.find(groupName);
    groupIt->second.insert(std::make_pair(itemName, newBlossom));

    return true;
}

/**
 * @brief request a registered blossom
 *
 * @param groupName group-identifier of the blossom
 * @param itemName item-identifier of the blossom
 *
 * @return pointer to the blossom or
 *         nullptr, if blossom the given group- and item-name was not found
 */
Blossom*
SakuraLangInterface::getBlossom(const std::string &groupName,
                                const std::string &itemName)
{
    // search for group
    std::map<std::string, std::map<std::string, Blossom*>>::const_iterator groupIt;
    groupIt = m_registeredBlossoms.find(groupName);

    if(groupIt != m_registeredBlossoms.end())
    {
        // search for item within group
        std::map<std::string, Blossom*>::const_iterator itemIt;
        itemIt = groupIt->second.find(itemName);

        if(itemIt != groupIt->second.end()) {
            return itemIt->second;
        }
    }

    return nullptr;
}

/**
 * @brief start processing by spawning the first subtree-object
 *
 * @param item subtree to spawn
 * @param initialValues initial set of values to override the same named values within the initial
 *                      called tree-item
 * @param errorMessage reference for error-message
 *
 * @return true, if proocess was successfull, else false
 */
bool
SakuraLangInterface::runProcess(TreeItem* item,
                                const DataMap &initialValues,
                                std::string &errorMessage)
{
    std::vector<SakuraItem*> childs;
    childs.push_back(item);
    std::vector<std::string> hierarchy;

    const bool result = m_queue->spawnParallelSubtrees(childs,
                                                       "",
                                                       hierarchy,
                                                       initialValues,
                                                       errorMessage);
    return result;
}

/**
 * @brief convert blossom-group-item into an output-message
 *
 * @param blossomItem blossom-group-tem to generate the output
 */
void
SakuraLangInterface::printOutput(const BlossomGroupItem &blossomGroupItem)
{
    std::string output = "";

    // print call-hierarchy
    for(uint32_t i = 0; i < blossomGroupItem.nameHirarchie.size(); i++)
    {
        for(uint32_t j = 0; j < i; j++) {
            output += "   ";
        }
        output += blossomGroupItem.nameHirarchie.at(i) + "\n";
    }

    printOutput(output);
}

/**
 * @brief convert blossom-item into an output-message
 *
 * @param blossomItem blossom-item to generate the output
 */
void
SakuraLangInterface::printOutput(const BlossomLeaf &blossomItem)
{
    printOutput(convertBlossomOutput(blossomItem));
}

/**
 * @brief print output-string
 *
 * @param output string, which should be printed
 */
void
SakuraLangInterface::printOutput(const std::string &output)
{
    m_mutex.lock();

    // get width of the termial to draw the separator-line
    struct winsize size;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &size);
    uint32_t terminalWidth = size.ws_col;

    // limit the length of the line to avoid problems in the gitlab-ci-runner
    if(terminalWidth > 300) {
        terminalWidth = 300;
    }

    // draw separator line
    std::string line(terminalWidth, '=');

    LOG_INFO(line + "\n\n" + output + "\n");

    m_mutex.unlock();
}

} // namespace Sakura
} // namespace Kitsunemimi

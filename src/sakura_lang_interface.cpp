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
#include <libKitsunemimiSakuraLang/blossom.h>

#include <sakura_garden.h>
#include <validator.h>
#include <sakura_file_collector.h>

#include <parsing/sakura_parser_interface.h>

#include <processing/subtree_queue.h>
#include <processing/thread_pool.h>
#include <processing/growth_plan.h>

#include <items/item_methods.h>

#include <libKitsunemimiJinja2/jinja2_converter.h>
#include <libKitsunemimiPersistence/logger/logger.h>


namespace Kitsunemimi
{
namespace Sakura
{

Kitsunemimi::Sakura::SakuraLangInterface* SakuraLangInterface::m_instance = nullptr;

/**
 * @brief constructor
 *
 * @param enableDebug set to true to enable the debug-output of the parser
 */
SakuraLangInterface::SakuraLangInterface(const uint16_t numberOfThreads,
                                         const bool enableDebug)
{
    m_validator = new Validator();
    m_fileCollector = new SakuraFileCollector(this);
    m_parser = new SakuraParserInterface(enableDebug);
    m_garden = new SakuraGarden();
    m_queue = new SubtreeQueue();
    m_threadPoos = new ThreadPool(numberOfThreads, this);
}

/**
 * @brief static methode to get instance of the interface
 *
 * @return pointer to the static instance
 */
SakuraLangInterface*
SakuraLangInterface::getInstance()
{
    if(m_instance == nullptr) {
        m_instance = new SakuraLangInterface();
    }

    return m_instance;
}

/**
 * @brief destructor
 */
SakuraLangInterface::~SakuraLangInterface()
{
    delete m_garden;
    delete m_queue;
    delete m_threadPoos;
}

/**
 * @brief trigger existing tree
 *
 * @param map with resulting items
 * @param id id of the tree to trigger
 * @param initialValues input-values for the tree
 * @param errorMessage reference for error-message
 *
 * @return true, if successfule, else false
 */
bool
SakuraLangInterface::triggerTree(DataMap &result,
                                 const std::string &id,
                                 DataMap &initialValues,
                                 std::string &errorMessage)
{
    LOG_DEBUG("trigger tree");

    m_lock.lock();

    // get initial tree-item
    TreeItem* tree = m_garden->getTree(id);
    if(tree == nullptr)
    {
        errorMessage = "No tree found for the input-path " + id;
        m_lock.unlock();
        return false;
    }

    GrowthPlan* currentObj = new GrowthPlan();
    currentObj->items = initialValues;

    overrideItems(currentObj->items, tree->values, ONLY_NON_EXISTING);

    // process sakura-file with initial values
    if(runProcess(currentObj,
                  tree,
                  errorMessage) == false)
    {
        delete currentObj;
        m_lock.unlock();
        return false;
    }

    result = currentObj->items;
    delete currentObj;
    m_lock.unlock();

    return true;
}

/**
 * @brief trigger existing blossom
 *
 * @param map with resulting items
 * @param id id of the blossom to trigger
 * @param initialValues input-values for the tree
 * @param errorMessage reference for error-message
 *
 * @return true, if successfule, else false
 */
bool
SakuraLangInterface::triggerBlossom(DataMap &result,
                                    const std::string &id,
                                    DataMap &initialValues,
                                    std::string &errorMessage)
{
    LOG_DEBUG("trigger blossom");

    // get initial blossom-item
    Blossom* blossom = getBlossom("special", id);
    if(blossom == nullptr)
    {
        errorMessage = "No blosom found for the id " + id;
        return false;
    }

    // inialize a new blossom-leaf for processing
    BlossomLeaf blossomLeaf;
    blossomLeaf.blossomName = id;
    blossomLeaf.blossomPath = id;
    blossomLeaf.blossomGroupType = "special";
    blossomLeaf.parentValues = &initialValues;
    blossomLeaf.input = initialValues;
    blossomLeaf.nameHirarchie.push_back("BLOSSOM: " + id);

    const bool validationResult = blossom->validateInput(initialValues, errorMessage);
    if(validationResult == false) {
        return false;
    }

    // process blossom
    const bool ret = blossom->growBlossom(blossomLeaf, errorMessage);
    if(ret == false) {
        return false;
    }

    // TODO: override only with the output-values to avoid unnecessary conflicts
    result = initialValues;
    overrideItems(result, blossomLeaf.output, ONLY_EXISTING);

    return true;
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
 * @brief add new tree
 *
 * @param id id of the new tree
 * @param treeContent content of the new tree, which should be parsed
 * @param errorMessage reference for error-message
 *
 * @return true, if successfule, else false
 */
bool
SakuraLangInterface::addTree(std::string id,
                             const std::string &treeContent,
                             std::string &errorMessage)
{
    m_lock.lock();

    // get initial tree-item
    TreeItem* tree = m_parser->parseTreeString(id, treeContent, errorMessage);
    if(tree == nullptr)
    {
        errorMessage = "Failed to parse " + id;
        m_lock.unlock();
        return false;
    }

    // validator parsed tree
    if(m_validator->checkSakuraItem(tree, "", errorMessage) == false)
    {
        m_lock.unlock();
        return false;
    }

    if(id == "") {
        id = tree->id;
    }
    const bool result = m_garden->addTree(id, tree);
    m_lock.unlock();

    return result;
}

/**
 * @brief add new template
 *
 * @param id id of the new template
 * @param templateContent content of the template
 *
 * @return true, if successfule, else false
 */
bool
SakuraLangInterface::addTemplate(const std::string &id,
                                 const std::string &templateContent)
{
    m_lock.lock();
    const bool result = m_garden->addTemplate(id, templateContent);
    m_lock.unlock();

    return result;
}

/**
 * @brief add new file to internal storage
 *
 * @param id id of the new file
 * @param data file-content as data-buffer
 *
 * @return true, if successfule, else false
 */
bool
SakuraLangInterface::addFile(const std::string &id,
                             DataBuffer* data)
{
    m_lock.lock();
    const bool result = m_garden->addFile(id, data);
    m_lock.unlock();

    return result;
}

/**
 * @brief add new ressource
 *
 * @param id id of the new ressource
 * @param content content of the new ressource, which should be parsed
 * @param errorMessage reference for error-message
 *
 * @return true, if successfule, else false
 */
bool
SakuraLangInterface::addResource(std::string id,
                                 const std::string &content,
                                 std::string &errorMessage)
{
    m_lock.lock();

    // get initial tree-item
    TreeItem* ressource = m_parser->parseTreeString(id, content, errorMessage);
    if(ressource == nullptr)
    {
        errorMessage = "Failed to parse " + id;
        m_lock.unlock();
        return false;
    }

    // validator parsed tree
    if(m_validator->checkSakuraItem(ressource, "", errorMessage) == false)
    {
        m_lock.unlock();
        return false;
    }

    if(id == "") {
        id = ressource->id;
    }
    const bool result = m_garden->addResource(id, ressource);
    m_lock.unlock();

    return result;
}

/**
 * @brief simple read all files within a directory and register by id instead of path
 *
 * @param directoryPath path to directory with sakura-files to read
 * @param errorMessage reference for error-message
 *
 * @return true, if successfule, else false
 */
bool
SakuraLangInterface::readFilesInDir(const std::string &directoryPath,
                                    std::string &errorMessage)
{
    return m_fileCollector->readFilesInDir(directoryPath, errorMessage);
}

/**
 * @brief get template-string
 *
 * @param id of the template
 *
 * @return template as string or empty string, if no template found for the given path
 */
const std::string
SakuraLangInterface::getTemplate(const std::string &id)
{
    return m_garden->getTemplate(id);
}

/**
 * @brief get file as data-buffer
 *
 * @param id id of the file
 *
 * @return file as data-buffer or nullptr, if no file found for the given path
 */
DataBuffer*
SakuraLangInterface::getFile(const std::string &id)
{
    return m_garden->getFile(id);
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
 * @brief start processing by spawning the first subtree-object
 *
 * @param resultingItems map for resulting items
 * @param item subtree to spawn
 * @param initialValues initial set of values to override the same named values within the initial
 *                      called tree-item
 * @param errorMessage reference for error-message
 *
 * @return true, if proocess was successful, else false
 */
bool
SakuraLangInterface::runProcess(GrowthPlan* plan,
                                TreeItem* tree,
                                std::string &errorMessage)
{
    // check if input-values match with the first tree
    const std::vector<std::string> failedInput = checkInput(tree->values,
                                                            plan->items);
    if(failedInput.size() > 0)
    {
        errorMessage = "Following input-values are not valid for the initial tress:\n";

        for(const std::string& item : failedInput) {
            errorMessage += "    " + item + "\n";
        }

        return false;
    }

    std::vector<SakuraItem*> childs;
    childs.push_back(tree);

    const bool result = m_queue->spawnParallelSubtrees(plan,
                                                       childs,
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
}

} // namespace Sakura
} // namespace Kitsunemimi

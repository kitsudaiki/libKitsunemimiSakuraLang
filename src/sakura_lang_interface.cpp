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
#include <initial_validator.h>
#include <sakura_file_collector.h>
#include <runtime_validation.h>
#include <parsing/sakura_parser_interface.h>

#include <processing/subtree_queue.h>
#include <processing/thread_pool.h>
#include <processing/growth_plan.h>

#include <items/item_methods.h>

#include <libKitsunemimiJinja2/jinja2_converter.h>


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
    m_validator = new InitialValidator();
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
 * @param status reference for status-output
 * @param error reference for error-output
 *
 * @return true, if successfule, else false
 */
bool
SakuraLangInterface::triggerTree(DataMap &result,
                                 const std::string &id,
                                 const DataMap &context,
                                 const DataMap &initialValues,
                                 BlossomStatus &status,
                                 ErrorContainer &error)
{
    LOG_DEBUG("trigger tree");

    std::lock_guard<std::mutex> guard(m_lock);

    // get initial tree-item
    TreeItem* tree = m_garden->getTree(id);
    if(tree == nullptr)
    {
        error.addMeesage("No tree found for the input-path " + id);
        LOG_ERROR(error);
        return false;
    }

    // prepare
    GrowthPlan growthPlan;
    growthPlan.items = initialValues;
    growthPlan.context = &context;
    overrideItems(growthPlan.items, tree->values, ONLY_NON_EXISTING);
    result.clear();

    // check input-values for its type
    if(checkTreeValues(tree->values, growthPlan.items, ValueItem::INPUT_PAIR_TYPE, error) == false)
    {
        LOG_ERROR(error);
        delete tree;
        return false;
    }

    // process sakura-file with initial values
    if(runProcess(result, &growthPlan, tree) == false)
    {
        status = growthPlan.status;
        error = growthPlan.error;
        LOG_ERROR(error);
        delete tree;
        return false;
    }

    // check output-values for its type
    if(checkTreeValues(tree->values, result, ValueItem::OUTPUT_PAIR_TYPE, error) == false)
    {
        LOG_ERROR(error);
        delete tree;
        return false;
    }

    delete tree;
    return true;
}

/**
 * @brief trigger existing blossom
 *
 * @param result map with resulting items
 * @param blossomName id of the blossom to trigger
 * @param blossomGroupName id of the group of the blossom to trigger
 * @param initialValues input-values for the tree
 * @param status reference for status-output
 * @param error reference for error-output
 *
 * @return true, if successfule, else false
 */
bool
SakuraLangInterface::triggerBlossom(DataMap &result,
                                    const std::string &blossomName,
                                    const std::string &blossomGroupName,
                                    const DataMap &context,
                                    const DataMap &initialValues,
                                    BlossomStatus &status,
                                    ErrorContainer &error)
{
    LOG_DEBUG("trigger blossom");

    // get initial blossom-item
    Blossom* blossom = getBlossom(blossomGroupName, blossomName);
    if(blossom == nullptr)
    {
        error.addMeesage("No blosom found for the id " + blossomName);
        return false;
    }

    // inialize a new blossom-leaf for processing
    BlossomIO blossomIO;
    blossomIO.blossomName = blossomName;
    blossomIO.blossomPath = blossomName;
    blossomIO.blossomGroupType = blossomGroupName;
    blossomIO.input = &initialValues;
    blossomIO.parentValues = blossomIO.input.getItemContent()->toMap();
    blossomIO.nameHirarchie.push_back("BLOSSOM: " + blossomName);

    std::string errorMessage;
    // check input to be complete
    if(blossom->validateFieldsCompleteness(initialValues,
                                           *blossom->getInputValidationMap(),
                                           FieldDef::INPUT_TYPE,
                                           errorMessage) == false)
    {
        error.addMeesage(errorMessage);
        error.addMeesage("check of completeness of input-fields failed");
        status.statusCode = 400;
        status.errorMessage = errorMessage;
        LOG_ERROR(error);
        return false;
    }

    // process blossom
    if(blossom->growBlossom(blossomIO, &context, status, error) == false)
    {
        error.addMeesage("trigger blossom failed.");
        LOG_ERROR(error);
        return false;
    }

    // check output to be complete
    DataMap* output = blossomIO.output.getItemContent()->toMap();
    if(blossom->validateFieldsCompleteness(*output,
                                           *blossom->getOutputValidationMap(),
                                           FieldDef::OUTPUT_TYPE,
                                           errorMessage) == false)
    {
        error.addMeesage(errorMessage);
        error.addMeesage("check of completeness of output-fields failed");
        status.statusCode = 500;
        status.errorMessage = errorMessage;
        LOG_ERROR(error);
        return false;
    }

    // TODO: override only with the output-values to avoid unnecessary conflicts
    result.clear();
    overrideItems(result, *output, ALL);

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
 * @brief getter for the comment of a tree-item
 *
 * @param comment reference for the comment-output
 * @param id id of the tree
 *
 * @return false, if tree for the given id was not found, else true
 */
bool
SakuraLangInterface::getTreeComment(std::string &comment,
                                    const std::string &id) const
{
    TreeItem* tree = m_garden->getTree(id, false);
    if(tree == nullptr) {
        return false;
    }

    comment = tree->comment;
    return true;
}

/**
 * @brief get the validation-map of a tree-item
 *
 * @param validationMap reference for the resulting validaiton-map
 * @param id id of the tree
 *
 * @return false, if tree for the given id was not found, else true
 */
bool
SakuraLangInterface::getTreeValidMap(std::map<std::string, FieldDef> &validationMap,
                                     const std::string &id) const
{
    TreeItem* tree = m_garden->getTree(id);
    if(tree == nullptr) {
        return false;
    }

    tree->getValidationMap(validationMap);
    return true;
}

/**
 * @brief add new tree
 *
 * @param id id of the new tree
 * @param treeContent content of the new tree, which should be parsed
 * @param error reference for error-output
 *
 * @return true, if successfule, else false
 */
bool
SakuraLangInterface::addTree(std::string id,
                             const std::string &treeContent,
                             ErrorContainer &error)
{
    std::lock_guard<std::mutex> guard(m_lock);

    // get initial tree-item
    TreeItem* tree = m_parser->parseTreeString(id, treeContent, error);
    if(tree == nullptr) {
        return false;
    }

    // validator parsed tree
    if(m_validator->checkSakuraItem(tree, "", error) == false) {
        return false;
    }

    if(id == "") {
        id = tree->id;
    }

    return m_garden->addTree(id, tree);
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
    std::lock_guard<std::mutex> guard(m_lock);
    return m_garden->addTemplate(id, templateContent);
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
    std::lock_guard<std::mutex> guard(m_lock);
    return m_garden->addFile(id, data);
}

/**
 * @brief add new ressource
 *
 * @param id id of the new ressource
 * @param content content of the new ressource, which should be parsed
 * @param error reference for error-output
 *
 * @return true, if successfule, else false
 */
bool
SakuraLangInterface::addResource(std::string id,
                                 const std::string &content,
                                 ErrorContainer &error)
{
    std::lock_guard<std::mutex> guard(m_lock);

    // get initial tree-item
    TreeItem* ressource = m_parser->parseTreeString(id, content, error);
    if(ressource == nullptr)
    {
        error.addMeesage("Failed to parse " + id);
        return false;
    }

    // validator parsed tree
    if(m_validator->checkSakuraItem(ressource, "", error) == false) {
        return false;
    }

    if(id == "") {
        id = ressource->id;
    }

    return m_garden->addResource(id, ressource);
}

/**
 * @brief simple read all files within a directory and register by id instead of path
 *
 * @param directoryPath path to directory with sakura-files to read
 * @param error reference for error-output
 *
 * @return true, if successfule, else false
 */
bool
SakuraLangInterface::readFilesInDir(const std::string &directoryPath,
                                    ErrorContainer &error)
{
    return m_fileCollector->readFilesInDir(directoryPath, error);
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
 * @brief start processing by spawning the first subtree-object
 *
 * @param resultingItems map for resulting items
 * @param item subtree to spawn
 * @param initialValues initial set of values to override the same named values within the initial
 *                      called tree-item
 *
 * @return true, if proocess was successful, else false
 */
bool
SakuraLangInterface::runProcess(DataMap &result,
                                GrowthPlan* plan,
                                TreeItem* tree)
{
    // check if input-values match with the first tree
    const std::vector<std::string> failedInput = checkInput(tree->values, plan->items);
    if(failedInput.size() > 0)
    {
        std::string message ="Following input-values are not valid for the initial tress:\n";
        for(const std::string& item : failedInput) {
            message += "    " + item + "\n";
        }
        plan->error.addMeesage(message);

        return false;
    }

    std::vector<SakuraItem*> childs;
    childs.push_back(tree);

    // init task
    const bool ret = m_queue->spawnParallelSubtrees(plan, childs);
    if(ret == false) {
        return false;
    }

    // collect relevant output-values
    std::map<std::string, DataItem*>::const_iterator it;
    for(it = plan->items.map.begin();
        it != plan->items.map.end();
        it++)
    {
        const ValueItem item = tree->values.getValueItem(it->first);
        if(item.type == ValueItem::OUTPUT_PAIR_TYPE) {
            result.insert(it->first, it->second->copy());
        }
    }

    return true;
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
SakuraLangInterface::printOutput(const BlossomIO &blossomItem)
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

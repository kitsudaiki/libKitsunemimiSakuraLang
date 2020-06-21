/**
 * @file        sakura_parsing.cpp
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

#include <libKitsunemimiSakuraParser/sakura_parsing.h>

#include <libKitsunemimiSakuraParser/sakura_items.h>
#include <libKitsunemimiSakuraParser/sakura_garden.h>
#include <sakura_parsing/sakura_parser_interface.h>

#include <libKitsunemimiCommon/common_methods/string_methods.h>
#include <libKitsunemimiCommon/common_items/data_items.h>

#include <libKitsunemimiPersistence/files/text_file.h>
#include <libKitsunemimiPersistence/files/file_methods.h>
#include <libKitsunemimiPersistence/files/binary_file.h>

using Kitsunemimi::Persistence::readFile;

namespace Kitsunemimi
{
namespace Sakura
{

/**
 * @brief constructor
 */
SakuraParsing::SakuraParsing(const bool debug)
{
    m_debug = debug;
    m_parser = new SakuraParserInterface(m_debug, this);
}

/**
 * @brief destructor
 */
SakuraParsing::~SakuraParsing()
{
    delete m_parser;
}

/**
 * @brief parse all tree-files at a specific location
 *
 * @param rootPath path to file or directory with the file(s) to parse
 *
 * @return true, if pasing all files was successful, else false
 */
bool
SakuraParsing::parseFiles(SakuraGarden &result,
                          const std::string &initialFilePath,
                          std::string &errorMessage)
{
    const std::string rootPath = Kitsunemimi::Persistence::getParent(initialFilePath);
    const std::string fileName = Kitsunemimi::Persistence::getRelativePath(initialFilePath,
                                                                           rootPath);
    result.rootPath = rootPath;
    m_rootPath = rootPath;
    m_fileQueue.push_back(fileName);

    while(m_fileQueue.size() > 0)
    {
        // get path from queue
        const std::string currentRelPath = m_fileQueue.front();
        m_fileQueue.pop_front();

        // check if already parsed
        std::map<std::string, TreeItem*>::iterator it;
        it = result.trees.find(currentRelPath);
        if(it != result.trees.end()) {
            continue;
        }

        // build absolute path
        const std::string filePath = rootPath + "/" + currentRelPath;

        // precheck if file exist
        if(Kitsunemimi::Persistence::doesPathExist(filePath) == false)
        {
            TableItem errorOutput;
            initErrorOutput(errorOutput);
            errorOutput.addRow(std::vector<std::string>{"source", "while reading sakura-files"});
            errorOutput.addRow(std::vector<std::string>{"message",
                                                        "path doesn't exist: " + filePath});
            errorMessage = errorOutput.toString();
            return false;
        }

        // parse file
        SakuraItem* parsed = parseSingleFile(currentRelPath,
                                             rootPath,
                                             errorMessage);
        if(parsed == nullptr) {
            return false;
        }

        // add parsed tree-file to results
        result.trees.insert(std::make_pair(currentRelPath, dynamic_cast<TreeItem*>(parsed)));

        // get additional files
        const std::string dirPath = Kitsunemimi::Persistence::getParent(filePath);
        if(alreadyCollected(dirPath) == false)
        {
            m_collectedDirectories.push_back(dirPath);

            if(collectFiles(result, dirPath, errorMessage) == false) {
                return false;
            }
            if(collectTemplates(result, dirPath, errorMessage) == false) {
                return false;
            }
        }
    }

    return true;
}

/**
 * @brief SakuraParsing::addFileToQueue
 * @param realtivePath
 * @param oringinPath
 * @return
 */
void
SakuraParsing::addFileToQueue(const std::string oldRelativePath)
{
    const std::string oldRootPath = Kitsunemimi::Persistence::getParent(m_currentFilePath);
    const std::string newRelativePath = Kitsunemimi::Persistence::getRelativePath(oldRootPath,
                                                                                  oldRelativePath,
                                                                                  m_rootPath);

    m_fileQueue.push_back(newRelativePath);
}

/**
 * @brief parse a single file
 *
 * @param result reference for the resulting string - json-item - pair
 * @param filePath path to a single file
 *
 * @return true, if successful, else false
 */
TreeItem*
SakuraParsing::parseSingleFile(const std::string &relativePath,
                               const std::string &rootPath,
                               std::string &errorMessage)
{
    const std::string filePath = rootPath + "/" + relativePath;

    // read file
    std::string fileContent = "";
    bool readResult = readFile(fileContent, filePath, errorMessage);
    if(readResult == false)
    {
        TableItem errorOutput;
        initErrorOutput(errorOutput);
        errorOutput.addRow(std::vector<std::string>{"source", "while reading sakura-files"});
        errorOutput.addRow(std::vector<std::string>{"message",
                                                    "failed to read file-path: "
                                                    + filePath
                                                    + " with error: "
                                                    + errorMessage});
        errorMessage = errorOutput.toString();
        return nullptr;
    }

    SakuraItem* resultItem = parseString(fileContent, errorMessage);
    if(resultItem == nullptr) {
        return nullptr;
    }

    TreeItem* tempTree = dynamic_cast<TreeItem*>(resultItem);
    tempTree->unparsedConent = fileContent;
    tempTree->relativePath = relativePath;
    tempTree->rootPath = rootPath;

    return tempTree;
}

/**
 * @brief parseString
 * @param result
 * @param content
 * @param errorMessage
 * @return
 */
bool
SakuraParsing::parseString(SakuraGarden &result,
                           const std::string &relativePath,
                           const std::string &content,
                           std::string &errorMessage)
{
    SakuraItem* parsetItem = parseString(content, errorMessage);
    if(parsetItem == nullptr
            || parsetItem->getType() != SakuraItem::TREE_ITEM)
    {
        return false;
    }

    TreeItem* parsedTree = dynamic_cast<TreeItem*>(parsetItem);

    TreeItem* check = result.getTreeByPath(relativePath);
    if(check == nullptr)
    {
        errorMessage = "tree-id already registered: " + parsedTree->id;
        return false;
    }

    parsedTree->unparsedConent = content;
    parsedTree->relativePath = relativePath;
    m_fileQueue.clear();

    result.trees.insert(std::make_pair(relativePath, parsedTree));

    return true;
}

/**
 * @brief SakuraParsing::collectFiles
 * @param result
 * @param path
 * @param rootPath
 */
bool
SakuraParsing::collectFiles(SakuraGarden &result,
                            const std::string &dirPath,
                            std::string &errorMessage)
{
    const std::string parent = dirPath + "/files";
    if(Kitsunemimi::Persistence::doesPathExist(parent))
    {
        return getFilesInDir(result,
                             boost::filesystem::path(parent),
                             "trees",
                             errorMessage);
    }

    return true;
}

/**
 * @brief SakuraParsing::collectTemplates
 * @param result
 * @param path
 * @param errorMessage
 * @return
 */
bool
SakuraParsing::collectTemplates(SakuraGarden &result,
                                const std::string &dirPath,
                                std::string &errorMessage)
{
    const std::string parent = dirPath + "/templates";
    if(Kitsunemimi::Persistence::doesPathExist(parent))
    {
        return getFilesInDir(result,
                             boost::filesystem::path(parent),
                             "templates",
                             errorMessage);
    }

    return true;
}


/**
 * @brief SakuraParsing::getFilesInDir
 * @param result
 * @param directory
 * @param rootPath
 * @param type
 */
bool
SakuraParsing::getFilesInDir(SakuraGarden &result,
                             const boost::filesystem::path &directory,
                             const std::string &type,
                             std::string &errorMessage)
{
    boost::filesystem::directory_iterator end_itr;
    for(boost::filesystem::directory_iterator itr(directory);
        itr != end_itr;
        ++itr)
    {
        if(is_directory(itr->path()))
        {
            if(getFilesInDir(result,
                             itr->path(),
                             type,
                             errorMessage) == false)
            {
                return false;
            }
        }
        else
        {
            std::string relPath = Kitsunemimi::Persistence::getRelativePath(itr->path().string(),
                                                                            m_rootPath);
            if(type == "files")
            {
                Kitsunemimi::DataBuffer* buffer = new DataBuffer();
                Kitsunemimi::Persistence::BinaryFile binFile(itr->path().string());
                bool ret = binFile.readCompleteFile(*buffer);

                if(ret == false)
                {
                    TableItem errorOutput;
                    initErrorOutput(errorOutput);
                    errorOutput.addRow({"source", "while reading files"});
                    errorOutput.addRow({"message", "can not read file " + itr->path().string()});
                    errorMessage = errorOutput.toString();
                    return false;
                }

                result.files.insert(std::make_pair(relPath, buffer));
            }
            if(type == "templates")
            {
                std::string fileContent = "";
                bool ret = Kitsunemimi::Persistence::readFile(fileContent,
                                                              itr->path().string(),
                                                              errorMessage);
                if(ret == false)
                {
                    TableItem errorOutput;
                    initErrorOutput(errorOutput);
                    errorOutput.addRow({"source", "while reading template-files"});
                    errorOutput.addRow({"message", errorMessage});
                    errorMessage = errorOutput.toString();
                    return false;
                }

                result.templates.insert(std::make_pair(relPath, fileContent));
            }
        }
    }

    return true;
}

/**
 * @brief SakuraParsing::alreadyCollected
 * @return
 */
bool
SakuraParsing::alreadyCollected(const std::string &path)
{
    std::vector<std::string>::iterator it;
    it = std::find(m_collectedDirectories.begin(),
                   m_collectedDirectories.end(),
                   path);

    if(it != m_collectedDirectories.end()) {
        return true;
    }

    return false;
}

/**
 * @brief parse a string
 *
 * @param result reference for the resulting json-item
 * @param content file-content to parse
 *
 * @return
 */
SakuraItem*
SakuraParsing::parseString(const std::string &content,
                           std::string &errorMessage)
{
    const bool parserResult = m_parser->parse(content);
    if(parserResult == false)
    {
        TableItem errorOutput = m_parser->getErrorMessage();
        errorMessage = errorOutput.toString();
        return nullptr;
    }

    return m_parser->getOutput();
}

/**
 * @brief SakuraParsing::initErrorOutput
 * @param errorOutput
 */
void
SakuraParsing::initErrorOutput(TableItem &errorOutput)
{
    errorOutput.addColumn("key");
    errorOutput.addColumn("value");
    errorOutput.addRow(std::vector<std::string>{"ERROR", " "});
    errorOutput.addRow(std::vector<std::string>{"component", "libKitsunemimiSakuraParser"});
}

}  // namespace Sakura
}  // namespace Kitsunemimi

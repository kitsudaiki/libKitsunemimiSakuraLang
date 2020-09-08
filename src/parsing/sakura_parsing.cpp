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

#include "sakura_parsing.h"

#include <libKitsunemimiSakuraLang/items/sakura_items.h>
#include <libKitsunemimiSakuraLang/sakura_garden.h>
#include <parsing/sakura_parser_interface.h>

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
 * @brief parse all sakura-files at a specific location
 *
 * @param initialFilePath path to file initial file to parse
 *
 * @return true, if pasing all files was successful, else false
 */
bool
SakuraParsing::parseTreeFiles(SakuraGarden &result,
                              const bfs::path &initialFilePath,
                              std::string &errorMessage)
{
    // precheck
    if(bfs::is_regular_file(initialFilePath) == false)
    {
        TableItem errorOutput;
        initErrorOutput(errorOutput);
        errorOutput.addRow(std::vector<std::string>{"source", "while reading sakura-files"});
        errorOutput.addRow(std::vector<std::string>{"message",
                                                    "path doesn't exist or is not a file: "
                                                    + initialFilePath.string()});
        errorMessage = errorOutput.toString();
        return false;
    }

    // prepare path-values
    const bfs::path rootPath = initialFilePath.parent_path();
    const bfs::path fileName = initialFilePath.leaf();

    // set global stuff
    result.rootPath = rootPath.string();
    m_rootPath = rootPath;
    m_fileQueue.push_back(fileName.string());

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
        const bfs::path filePath = rootPath / currentRelPath;
        m_currentFilePath = filePath;

        // precheck if file exist
        if(bfs::exists(filePath) == false)
        {
            TableItem errorOutput;
            initErrorOutput(errorOutput);
            errorOutput.addRow(std::vector<std::string>{"source", "while reading sakura-files"});
            errorOutput.addRow(std::vector<std::string>{"message",
                                                        "path doesn't exist: "
                                                        + filePath.string()});
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

        // add parsed sakura-file to results
        result.trees.insert(std::make_pair(currentRelPath, dynamic_cast<TreeItem*>(parsed)));

        // get additional files
        const bfs::path dirPath = filePath.parent_path();
        if(alreadyCollected(dirPath) == false)
        {
            m_collectedDirectories.push_back(dirPath.string());

            if(collectFiles(result, dirPath, errorMessage) == false) {
                return false;
            }
            if(collectRessources(result, dirPath, errorMessage) == false) {
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
SakuraParsing::addFileToQueue(bfs::path oldRelativePath)
{
    const bfs::path oldRootPath = m_currentFilePath.parent_path();
    if(bfs::is_directory(oldRootPath / oldRelativePath)) {
        oldRelativePath /= bfs::path("root.sakura");
    }

    const bfs::path oldAbsolutePath = oldRootPath / oldRelativePath;
    const bfs::path newRelativePath = bfs::relative(oldAbsolutePath, m_rootPath);

    m_fileQueue.push_back(newRelativePath.string());
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
SakuraParsing::parseSingleFile(const bfs::path &relativePath,
                               const bfs::path &rootPath,
                               std::string &errorMessage)
{
    const bfs::path filePath = rootPath / relativePath;

    // read file
    std::string fileContent = "";
    bool readResult = readFile(fileContent, filePath.string(), errorMessage);
    if(readResult == false)
    {
        TableItem errorOutput;
        initErrorOutput(errorOutput);
        errorOutput.addRow(std::vector<std::string>{"source", "while reading sakura-files"});
        errorOutput.addRow(std::vector<std::string>{"message",
                                                    "failed to read file-path: "
                                                    + filePath.string()
                                                    + " with error: "
                                                    + errorMessage});
        errorMessage = errorOutput.toString();
        return nullptr;
    }

    SakuraItem* resultItem = parseStringToTree(fileContent, errorMessage);
    if(resultItem == nullptr) {
        return nullptr;
    }

    TreeItem* tempTree = dynamic_cast<TreeItem*>(resultItem);
    tempTree->unparsedConent = fileContent;
    tempTree->relativePath = relativePath.string();
    tempTree->rootPath = rootPath.string();

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
SakuraParsing::parseTreeString(SakuraGarden &result,
                               const bfs::path &relativePath,
                               const std::string &content,
                               std::string &errorMessage)
{
    TreeItem* parsetItem = parseStringToTree(content, errorMessage);
    if(parsetItem == nullptr) {
        return false;
    }

    TreeItem* check = result.getTree(relativePath);
    if(check != nullptr)
    {
        errorMessage = "tree-id already registered: " + parsetItem->id;
        return false;
    }

    parsetItem->unparsedConent = content;
    parsetItem->relativePath = relativePath.string();
    m_fileQueue.clear();

    result.trees.insert(std::make_pair(relativePath.string(), parsetItem));

    return true;
}

/**
 * @brief SakuraParsing::parseRessourceString
 * @param result
 * @param content
 * @param errorMessage
 * @return
 */
bool
SakuraParsing::parseRessourceString(SakuraGarden &result,
                                    const std::string &content,
                                    std::string &errorMessage)
{
    TreeItem* parsetItem = parseStringToTree(content, errorMessage);
    if(parsetItem == nullptr) {
        return false;
    }

    result.resources.insert(std::make_pair(parsetItem->id, parsetItem));
    m_fileQueue.clear();

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
                            const bfs::path &dirPath,
                            std::string &errorMessage)
{
    const bfs::path parent = dirPath / bfs::path("files");
    if(bfs::exists(parent))
    {
        return getFilesInDir(result,
                             bfs::path(parent),
                             "files",
                             errorMessage);
    }

    return true;
}

/**
 * @brief SakuraParsing::collectRessources
 * @param result
 * @param dirPath
 * @param errorMessage
 * @return
 */
bool
SakuraParsing::collectRessources(SakuraGarden &result,
                                 const bfs::path &dirPath,
                                 std::string &errorMessage)
{
    const bfs::path parent = dirPath / bfs::path("resources");
    if(bfs::exists(parent))
    {
        return getFilesInDir(result,
                             bfs::path(parent),
                             "resources",
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
                                const bfs::path &dirPath,
                                std::string &errorMessage)
{
    const bfs::path parent = dirPath / bfs::path("templates");
    if(bfs::exists(parent))
    {
        return getFilesInDir(result,
                             bfs::path(parent),
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
                             const bfs::path &directory,
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
            bfs::path relPath = bfs::relative(itr->path(), m_rootPath);
            //--------------------------------------------------------------------------------------
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

                result.files.insert(std::make_pair(relPath.string(), buffer));
            }
            //--------------------------------------------------------------------------------------
            if(type == "resources")
            {
                std::string fileContent = "";
                bool ret = Kitsunemimi::Persistence::readFile(fileContent,
                                                              itr->path().string(),
                                                              errorMessage);
                if(ret == false)
                {
                    TableItem errorOutput;
                    initErrorOutput(errorOutput);
                    errorOutput.addRow({"source", "while reading ressource-files"});
                    errorOutput.addRow({"message", errorMessage});
                    errorMessage = errorOutput.toString();
                    return false;
                }

                TreeItem* parsedTree = parseStringToTree(fileContent, errorMessage);

                if(parsedTree == nullptr)
                {
                    TableItem errorOutput;
                    initErrorOutput(errorOutput);
                    errorOutput.addRow({"source", "while parsing ressource-files"});
                    errorOutput.addRow({"message", errorMessage});
                    errorMessage = errorOutput.toString();
                    return false;
                }

                result.resources.insert(std::make_pair(parsedTree->id, parsedTree));
            }
            //--------------------------------------------------------------------------------------
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

                result.templates.insert(std::make_pair(relPath.string(), fileContent));
            }
            //--------------------------------------------------------------------------------------
        }
    }

    return true;
}

/**
 * @brief SakuraParsing::alreadyCollected
 * @return
 */
bool
SakuraParsing::alreadyCollected(const bfs::path &path)
{
    std::vector<std::string>::iterator it;
    it = std::find(m_collectedDirectories.begin(),
                   m_collectedDirectories.end(),
                   path.string());

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
TreeItem*
SakuraParsing::parseStringToTree(const std::string &content,
                                 std::string &errorMessage)
{
    const bool parserResult = m_parser->parse(content);
    if(parserResult == false)
    {
        TableItem errorOutput = m_parser->getErrorMessage();
        errorMessage = errorOutput.toString();
        return nullptr;
    }

    return dynamic_cast<TreeItem*>(m_parser->getOutput());
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
    errorOutput.addRow(std::vector<std::string>{"component", "libKitsunemimiSakuraLang"});
}

} // namespace Sakura
} // namespace Kitsunemimi

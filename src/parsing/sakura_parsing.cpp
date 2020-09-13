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

#include <items/sakura_items.h>
#include <sakura_garden.h>
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
    m_parser = new SakuraParserInterface(debug, this);
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
 * @param garden reference to the sakura-garden-object to store all found data
 * @param initialFilePath path to file initial file to parse
 * @param errorMessage reference to error-message
 *
 * @return true, if pasing all files was successful, else false
 */
bool
SakuraParsing::parseTreeFiles(SakuraGarden &garden,
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
    garden.rootPath = rootPath.string();
    m_rootPath = rootPath;
    m_fileQueue.push_back(fileName.string());

    while(m_fileQueue.size() > 0)
    {
        // get path from queue
        const std::string currentRelPath = m_fileQueue.front();
        m_fileQueue.pop_front();

        // check if already parsed
        std::map<std::string, TreeItem*>::iterator it;
        it = garden.trees.find(currentRelPath);
        if(it != garden.trees.end()) {
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
        garden.trees.insert(std::make_pair(currentRelPath, dynamic_cast<TreeItem*>(parsed)));

        // get additional files
        const bfs::path dirPath = filePath.parent_path();
        if(alreadyCollected(dirPath) == false)
        {
            m_collectedDirectories.push_back(dirPath.string());

            if(collectFiles(garden, dirPath, errorMessage) == false) {
                return false;
            }
            if(collectResources(garden, dirPath, errorMessage) == false) {
                return false;
            }
            if(collectTemplates(garden, dirPath, errorMessage) == false) {
                return false;
            }
        }
    }

    return true;
}

/**
 * @brief add subtree to processing-queue
 *
 * @param relativePath relative path of the file to add
 */
void
SakuraParsing::addFileToQueue(bfs::path relativePath)
{
    const bfs::path rootPath = m_currentFilePath.parent_path();
    if(bfs::is_directory(rootPath / relativePath)) {
        relativePath /= bfs::path("root.sakura");
    }

    const bfs::path oldAbsolutePath = rootPath / relativePath;
    const bfs::path newRelativePath = bfs::relative(oldAbsolutePath, m_rootPath);

    m_fileQueue.push_back(newRelativePath.string());
}

/**
 * @brief parse a single file
 *
 * @param relativePath relative file-path related to the root-path
 * @param rootPath directory-path of the initial file
 * @param errorMessage reference to error-message
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

    // check if file was empty
    if(fileContent.size() == 0)
    {
        TableItem errorOutput;
        initErrorOutput(errorOutput);
        errorOutput.addRow(std::vector<std::string>{"source", "while reading sakura-files"});
        errorOutput.addRow(std::vector<std::string>{"message", "file-path: "
                                                               + filePath.string()
                                                               + " is an empty file"});
        errorMessage = errorOutput.toString();
        return nullptr;
    }

    // parse tree
    SakuraItem* resultItem = parseStringToTree(fileContent, filePath.string(), errorMessage);
    if(resultItem == nullptr) {
        return nullptr;
    }

    // add additinal information to the parsed object
    TreeItem* tempTree = dynamic_cast<TreeItem*>(resultItem);
    tempTree->unparsedConent = fileContent;
    tempTree->relativePath = relativePath.string();
    tempTree->rootPath = rootPath.string();

    return tempTree;
}

/**
 * @brief parse single string
 *
 * @param garden reference to the sakura-garden-object to store all found data
 * @param name name of the parsed tree as identification
 * @param content content to parse
 * @param errorMessage reference to error-message
 *
 * @return true, if successful, else false
 */
bool
SakuraParsing::parseTreeString(SakuraGarden &garden,
                               const std::string &name,
                               const std::string &content,
                               std::string &errorMessage)
{
    // precheck
    TreeItem* check = garden.getTree(name);
    if(check != nullptr)
    {
        errorMessage = "tree already registered with path: " + name;
        return false;
    }

    // parse
    TreeItem* parsetItem = parseStringToTree(content, "", errorMessage);
    if(parsetItem == nullptr) {
        return false;
    }

    // update content
    parsetItem->unparsedConent = content;
    parsetItem->relativePath = name;

    // add parsed item to garden
    garden.trees.insert(std::make_pair(name, parsetItem));

    return true;
}

/**
 * @brief parse resource
 *
 * @param garden reference to the sakura-garden-object to store all found data
 * @param absolutePath absolute path of the resource-file
 * @param content conentn of the resource
 * @param errorMessage reference to error-message
 *
 * @return true, if successful, else false
 */
bool
SakuraParsing::parseRessourceString(SakuraGarden &garden,
                                    const std::string &absolutePath,
                                    const std::string &content,
                                    std::string &errorMessage)
{
    TreeItem* parsetItem = parseStringToTree(content, absolutePath, errorMessage);
    if(parsetItem == nullptr) {
        return false;
    }

    garden.resources.insert(std::make_pair(parsetItem->id, parsetItem));

    return true;
}

/**
 * @brief collect files within a directory
 *
 * @param garden reference to the sakura-garden-object to store all found data
 * @param dirPath directory-path where to search for files
 * @param errorMessage reference to error-message
 *
 * @return true, if successful, else false
 */
bool
SakuraParsing::collectFiles(SakuraGarden &garden,
                            const bfs::path &dirPath,
                            std::string &errorMessage)
{
    const bfs::path parent = dirPath / bfs::path("files");
    if(bfs::exists(parent))
    {
        return getFilesInDir(garden,
                             bfs::path(parent),
                             "files",
                             errorMessage);
    }

    return true;
}

/**
 * @brief collect resources within a directory
 *
 * @param garden reference to the sakura-garden-object to store all found data
 * @param dirPath directory-path where to search for resources
 * @param errorMessage reference to error-message
 *
 * @return true, if successful, else false
 */
bool
SakuraParsing::collectResources(SakuraGarden &garden,
                                const bfs::path &dirPath,
                                std::string &errorMessage)
{
    const bfs::path parent = dirPath / bfs::path("resources");

    if(bfs::exists(parent))
    {
        return getFilesInDir(garden,
                             bfs::path(parent),
                             "resources",
                             errorMessage);
    }

    return true;
}

/**
 * @brief collect templates within a directory
 *
 * @param garden reference to the sakura-garden-object to store all found data
 * @param dirPath directory-path where to search for templates
 * @param errorMessage reference to error-message
 *
 * @return true, if successful, else false
 */
bool
SakuraParsing::collectTemplates(SakuraGarden &garden,
                                const bfs::path &dirPath,
                                std::string &errorMessage)
{
    const bfs::path parent = dirPath / bfs::path("templates");

    if(bfs::exists(parent))
    {
        return getFilesInDir(garden,
                             bfs::path(parent),
                             "templates",
                             errorMessage);
    }

    return true;
}

/**
 * @brief iterate over all files and directory within the current directory
 *
 * @param garden reference to the sakura-garden-object to store all found data
 * @param directory directory-path to iterate over
 * @param type type to search for
 * @param errorMessage reference to error-message
 *
 * @return true, if successful, else false
 */
bool
SakuraParsing::getFilesInDir(SakuraGarden &garden,
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
            if(getFilesInDir(garden,
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

                garden.files.insert(std::make_pair(relPath.string(), buffer));
            }
            //--------------------------------------------------------------------------------------
            if(type == "resources")
            {
                // read resource-file
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

                // parse resource-file
                TreeItem* parsedTree = parseStringToTree(fileContent,
                                                         itr->path().string(),
                                                         errorMessage);
                if(parsedTree == nullptr)
                {
                    TableItem errorOutput;
                    initErrorOutput(errorOutput);
                    errorOutput.addRow({"source", "while parsing ressource-files"});
                    errorOutput.addRow({"message", errorMessage});
                    errorMessage = errorOutput.toString();
                    return false;
                }

                garden.resources.insert(std::make_pair(parsedTree->id, parsedTree));
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

                garden.templates.insert(std::make_pair(relPath.string(), fileContent));
            }
            //--------------------------------------------------------------------------------------
        }
    }

    return true;
}

/**
 * @brief check if templates, resources and file of a location are already read
 *
 * @return true, if location was already corrected, else false
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
 * @param content file-content to parse
 * @param filePath file-path for error-output
 * @param errorMessage reference for error-message
 *
 * @return pointer to the parded tree-item
 */
TreeItem*
SakuraParsing::parseStringToTree(const std::string &content,
                                 const std::string &filePath,
                                 std::string &errorMessage)
{
    const bool parserResult = m_parser->parse(content, filePath);

    if(parserResult == false)
    {
        TableItem errorOutput = m_parser->getErrorMessage();
        errorMessage = errorOutput.toString();
        return nullptr;
    }

    return dynamic_cast<TreeItem*>(m_parser->getOutput());
}

/**
 * @brief initialize table for error-message output
 *
 * @param errorOutput reference to table-item for error-message output
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

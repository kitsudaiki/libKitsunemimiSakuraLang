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
    m_parser = new SakuraParserInterface(m_debug);
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
                          const std::string &rootPath,
                          std::string &errorMessage)
{
    result.rootPath = rootPath;

    // parse
    if(parseAllFiles(result, rootPath, errorMessage) == false) {
        return false;
    }

    return true;
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
    TreeItem* parsetTree = parseString(content, errorMessage);
    if(parsetTree == nullptr)  {
        return false;
    }

    TreeItem* check = result.getTreeById(parsetTree->id);
    if(check == nullptr)
    {
        errorMessage = "tree-id already registered: " + parsetTree->id;
        return false;
    }

    result.trees.insert(std::make_pair(relativePath, parsetTree));

    return true;
}

/**
 * @brief search and parse all files in a specific location
 *
 * @param rootPath path to file or directory with the file(s) to parse
 *
 * @return true, if all was successful, else false
 */
bool
SakuraParsing::parseAllFiles(SakuraGarden &result,
                             const std::string &rootPath,
                             std::string &errorMessage)
{
    // precheck
    if(Kitsunemimi::Persistence::doesPathExist(rootPath) == false)
    {
        TableItem errorOutput;
        initErrorOutput(errorOutput);
        errorOutput.addRow(std::vector<std::string>{"source", "while reading sakura-files"});
        errorOutput.addRow(std::vector<std::string>{"message",
                                                    "path doesn't exist: " + rootPath});
        errorMessage = errorOutput.toString();
        return false;
    }

    // get all files
    bool ret = collectFiles(result, rootPath, errorMessage);
    if(ret == false) {
        return false;
    }

    // check result
    if(result.trees.size() == 0)
    {
        TableItem errorOutput;
        initErrorOutput(errorOutput);
        errorOutput.addRow(std::vector<std::string>{"source", "while reading sakura-files"});
        errorOutput.addRow(std::vector<std::string>{"message",
                                                    "no files found in the directory: "
                                                    + rootPath});
        errorMessage = errorOutput.toString();
        return false;
    }

    // iterate over all file-paths and parse them
    std::map<std::string, TreeItem*>::iterator it;
    for(it = result.trees.begin();
        it != result.trees.end();
        it++)
    {
        TreeItem* parsed = parseSingleFile(it->first,
                                           result.rootPath,
                                           errorMessage);
        if(parsed == nullptr) {
            return false;
        }

        it->second = parsed;
    }

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
                            const std::string &path,
                            std::string &errorMessage)
{
    boost::filesystem::path pathObj(path);

    if(is_directory(pathObj))
    {
        return getFilesInDir(result,
                             pathObj,
                             path,
                             "trees",
                             errorMessage);
    }
    else
    {
        const std::string parent = Kitsunemimi::Persistence::getParent(path);
        const std::string relPath = Kitsunemimi::Persistence::getRelativePath(path, parent);

        result.rootPath = parent;
        result.trees.insert(std::make_pair(relPath, nullptr));
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
bool SakuraParsing::getFilesInDir(SakuraGarden &result,
                             const boost::filesystem::path &directory,
                             const std::string &rootPath,
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
            if(itr->path().leaf().string() == "files")
            {
                if(getFilesInDir(result,
                                 itr->path(),
                                 rootPath,
                                 "files",
                                 errorMessage) == false)
                {
                    return false;
                }
            }
            else if(itr->path().leaf().string() == "templates")
            {
                if(getFilesInDir(result,
                                 itr->path(),
                                 rootPath,
                                 "templates",
                                 errorMessage) == false)
                {
                    return false;
                }
            }
            else
            {
                if(getFilesInDir(result,
                                 itr->path(),
                                 rootPath,
                                 type,
                                 errorMessage) == false)
                {
                    return false;
                }
            }
        }
        else
        {
            std::string relPath = Kitsunemimi::Persistence::getRelativePath(itr->path().string(),
                                                                            rootPath);

            if(type == "trees")
            {
                result.trees.insert(std::make_pair(relPath, nullptr));
            }
            if(type == "files")
            {
                Kitsunemimi::DataBuffer* buffer = new DataBuffer();
                Kitsunemimi::Persistence::BinaryFile binFile(itr->path().string());
                bool ret = binFile.readCompleteFile(buffer);

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

    TreeItem* resultItem = parseString(fileContent, errorMessage);
    if(resultItem == nullptr) {
        return resultItem;
    }

    resultItem->unparsedConent = fileContent;
    resultItem->relativePath = relativePath;
    resultItem->rootPath = rootPath;

    return resultItem;
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

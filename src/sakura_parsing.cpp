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
    if(Kitsunemimi::Persistence::isDir(rootPath))
    {
        Kitsunemimi::Persistence::listFiles(m_allFilePaths,
                                            rootPath,
                                            true,
                                            {"files", "templates"});
        // check result
        if(m_allFilePaths.size() == 0)
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
    }
    else
    {
        // store file-path with a placeholder in a list
        m_allFilePaths.push_back(rootPath);
    }

    // get and parse file-contents
    for(uint32_t i = 0; i < m_allFilePaths.size(); i++)
    {
        const std::string filePath = m_allFilePaths.at(i);

        if(parseSingleFile(result, filePath, rootPath, errorMessage) == false) {
            return false;
        }
    }

    m_allFilePaths.clear();

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
bool
SakuraParsing::parseSingleFile(SakuraGarden &result,
                               const std::string &filePath,
                               const std::string &rootPath,
                               std::string &errorMessage)
{
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
        return false;
    }

    TreeItem* resultItem = parseString(fileContent, errorMessage);
    if(resultItem == nullptr) {
        return false;
    }

    const std::string relativePath = Kitsunemimi::Persistence::getRelativePath(filePath, rootPath);

    resultItem->unparedConent = fileContent;
    resultItem->path = relativePath;

    result.trees.insert(std::make_pair(relativePath, resultItem));

    return true;
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

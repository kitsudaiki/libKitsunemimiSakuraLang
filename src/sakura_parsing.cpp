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

#include <sakura_parsing/sakura_parser_interface.h>

#include <libKitsunemimiCommon/common_methods/string_methods.h>
#include <libKitsunemimiCommon/common_items/data_items.h>

#include <libKitsunemimiJson/json_item.h>
#include <libKitsunemimiPersistence/files/text_file.h>

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
SakuraParsing::parseFiles(const std::string &rootPath,
                          std::string &errorMessage)
{
    JsonItem result;
    m_idContentMapping.clear();

    // init error-message
    m_errorMessage.clearTable();
    m_errorMessage.addColumn("key");
    m_errorMessage.addColumn("value");
    m_errorMessage.addRow(std::vector<std::string>{"ERROR", " "});
    m_errorMessage.addRow(std::vector<std::string>{"component", "libKitsunemimiSakuraParser"});
    errorMessage = m_errorMessage.toString();

    // parse
    if(parseAllFiles(rootPath, errorMessage) == false) {
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
SakuraParsing::parseAllFiles(const std::string &rootPath,
                             std::string &errorMessage)
{
    boost::filesystem::path rootPathObj(rootPath);    

    // precheck
    if(exists(rootPathObj) == false)
    {
        m_errorMessage.addRow(std::vector<std::string>{"source", "while reading sakura-files"});
        m_errorMessage.addRow(std::vector<std::string>{"message",
                                                       "path doesn't exist: " + rootPath});
        errorMessage = m_errorMessage.toString();
        return false;
    }

    // get all files
    if(is_directory(rootPathObj))
    {
        getFilesInDir(rootPathObj);
        // check result
        if(m_allFilePaths.size() == 0)
        {
            m_errorMessage.addRow(std::vector<std::string>{"source", "while reading sakura-files"});
            m_errorMessage.addRow(std::vector<std::string>{"message",
                                                           "no files found in the directory: "
                                                           + rootPath});
            errorMessage = m_errorMessage.toString();
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

        if(parseSingleFile(m_allFilePaths.at(i), filePath, errorMessage) == false) {
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
SakuraParsing::parseSingleFile(const std::string &path,
                               const std::string &filePath,
                               std::string &errorMessage)
{
    // read file
    std::string fileContent = "";
    bool readResult = readFile(fileContent, filePath, errorMessage);
    if(readResult == false)
    {
        m_errorMessage.addRow(std::vector<std::string>{"source", "while reading sakura-files"});
        m_errorMessage.addRow(std::vector<std::string>{"message",
                                                       "failed to read file-path: "
                                                       + filePath
                                                       + " with error: "
                                                       + errorMessage});
        errorMessage = m_errorMessage.toString();
        return false;
    }

    JsonItem resultItem;
    if(parseString(resultItem, fileContent, errorMessage) == false) {
        return false;
    }

    resultItem.insert("b_path", new DataValue(path), true);
    m_idContentMapping.insert(std::make_pair(resultItem.get("b_id").toString(), resultItem));
    m_pathIdMapping.insert(std::make_pair(path, resultItem.get("b_id").toString()));

    // debug-output to print the parsed file-content as json-string
    if(m_debug) {
        std::cout<<resultItem.toString(true)<<std::endl;
    }

    return true;
}

/**
 * @brief parse a string
 *
 * @param result reference for the resulting json-item
 * @param content file-content to parse
 *
 * @return true, if successful, else false
 */
bool
SakuraParsing::parseString(Json::JsonItem &result,
                           const std::string &content,
                           std::string &errorMessage)
{
    const bool parserResult = m_parser->parse(content);
    if(parserResult == false)
    {
        m_errorMessage = m_parser->getErrorMessage();
        return false;
    }

    // get the parsed result from the parser and get path of the file,
    // where the skript actually is and add it to the parsed content.
    //result = Json::JsonItem(m_parser->getOutput()->copy()->toMap());

    return true;
}

/**
 * @brief request the parsed content of a specific subtree
 *
 * @param name Name of the requested file-content. If string is empty, the content of the first
 *             file in the list will be returned.
 *
 * @return Subtree-content as json-item. This is an invalid item, when the requested name
 *         doesn't exist in the parsed file list.
 */
const JsonItem
SakuraParsing::getParsedFileContent(const std::string &name)
{
    // precheck
    if(name == ""
            && m_idContentMapping.size() > 0)
    {
        return m_idContentMapping.begin()->second;
    }

    // search
    std::map<std::string, JsonItem>::iterator it;
    for(it = m_idContentMapping.begin();
        it != m_idContentMapping.end();
        it++)
    {
        if(it->second.get("b_id").toString() == name) {
            return it->second;
        }
    }

    return JsonItem();
}

/**
 * @brief get all file-paths in a directory and its subdirectory
 *
 * @param directory parent-directory for searching
 */
void
SakuraParsing::getFilesInDir(const boost::filesystem::path &directory)
{
    directory_iterator end_itr;
    for(directory_iterator itr(directory);
        itr != end_itr;
        ++itr)
    {
        if(is_directory(itr->path()))
        {
            // process subdirectories, but no directories named tempales or files, because
            // they shouldn't contain any tree-files
            if(itr->path().leaf().string() != "templates"
                    && itr->path().leaf().string() != "files")
            {
                getFilesInDir(itr->path());
            }
        }
        else
        {
            if(m_debug) {
                std::cout<<"found file: "<<itr->path().string()<<std::endl;
            }
            m_allFilePaths.push_back(itr->path().string());
        }
    }
}

}  // namespace Sakura
}  // namespace Kitsunemimi

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

#include <libKitsunemimiSakuraParser/sakura_parsing.h>
#include <libKitsunemimiCommon/common_methods/string_methods.h>
#include <libKitsunemimiJson/json_item.h>
#include <libKitsunemimiCommon/common_items/data_items.h>
#include <sakura_parsing/sakura_parser_interface.h>

namespace Kitsunemimi
{
namespace Sakura
{

/**
 * @brief constructor
 */
SakuraParsing::SakuraParsing(const bool debug)
{
    m_parser = new SakuraParserInterface(debug);
}

/**
 * @brief SakuraCompiler::parseFiles
 * @param rootPath
 * @param seedName
 * @return
 */
JsonItem
SakuraParsing::parseFiles(const std::string &rootPath,
                          const std::string &seedName)
{
    JsonItem result;

    // init error-message
    m_errorMessage.clearTable();
    m_errorMessage.addColumn("key");
    m_errorMessage.addColumn("value");
    m_errorMessage.addRow(std::vector<std::string>{"ERROR", " "});
    m_errorMessage.addRow(std::vector<std::string>{"component", "libKitsunemimiSakuraParser"});

    if(initFileCollector(rootPath) == false) {
        return result;
    }

    if(seedName == "") {
        result = getObject(getSeedName(0));
    } else {
        result = getObject(seedName);
    }

    if(result.isValid()) {
        preProcessObject(result);
    }

    return result;
}

/**
 * @brief SakuraParsing::getError
 * @return
 */
TableItem
SakuraParsing::getError() const
{
    return m_errorMessage;
}

/**
 * @brief SakuraCompiler::processObject
 * @param value
 * @return
 */
void
SakuraParsing::preProcessObject(JsonItem &object)
{
    // precheck
    if(object.isValid() == false) {
        return;
    }

    // end of tree
    if(object.get("b_type").toString() == "blossom") {
        return;
    }

    // continue building
    JsonItem branch = object;

    if(object.get("b_type").toString() == "tree"
            || object.get("b_type").toString() == "branch")
    {
        branch = getObject(object.get("b_id").toString());
        object.insert("parts", branch.get("parts"));
        object.insert("items", branch.get("items"));
    }

    if(object.get("b_type").toString() == "seed")
    {
        JsonItem subtree = object.get("subtree");
        preProcessObject(subtree);
    }

    if(object.contains("parts"))
    {
        JsonItem parts = object.get("parts");
        preProcessArray(parts);
    }

    if(object.contains("if_parts"))
    {
        JsonItem parts = object.get("if_parts");
        preProcessArray(parts);
    }

    if(object.contains("else_parts"))
    {
        JsonItem parts = object.get("else_parts");
        preProcessArray(parts);
    }
}

/**
 * @brief SakuraCompiler::processArray
 * @param value
 * @return
 */
void
SakuraParsing::preProcessArray(JsonItem &object)
{
    for(uint32_t i = 0; i < object.size(); i++)
    {
        JsonItem item = object.get(i);
        preProcessObject(item);
    }
}

/**
 * @brief ParserInit::initProcess
 * @return
 */
bool
SakuraParsing::initFileCollector(const std::string &rootPath)
{
    boost::filesystem::path rootPathObj(rootPath);    

    // precheck
    if(exists(rootPathObj) == false)
    {
        m_errorMessage.addRow(std::vector<std::string>{"source", "while reading sakura-files"});
        m_errorMessage.addRow(std::vector<std::string>{"message",
                                                       "path doesn't exist: " + rootPath});
        return false;
    }

    // get all files
    if(is_directory(rootPathObj))
    {
        getFilesInDir(rootPathObj);
        // check result
        if(m_fileContents.size() == 0)
        {
            m_errorMessage.addRow(std::vector<std::string>{"source", "while reading sakura-files"});
            m_errorMessage.addRow(std::vector<std::string>{"message",
                                                           "no files found in the directory: "
                                                           + rootPath});
            return false;
        }
    }
    else
    {
        m_fileContents.push_back(std::make_pair(rootPath, JsonItem()));
    }

    // get and parse file-contents
    for(uint32_t i = 0; i < m_fileContents.size(); i++)
    {
        const std::string filePath = m_fileContents.at(i).first.string();

        // parse sakura-template into a data-tree
        const bool parserResult = m_parser->parse(readFile(filePath));
        if(parserResult == false)
        {
            m_errorMessage = m_parser->getErrorMessage();
            return false;
        }

        m_fileContents[i].second = m_parser->getOutput()->copy()->toMap();

        const std::string directoryPath = m_fileContents.at(i).first.parent_path().string();
        m_fileContents[i].second.insert("b_path",
                                        new DataValue(directoryPath),
                                        true);

        //const std::string output = m_fileContents[i].second.toString(true);
        //std::cout<<output<<std::endl;
    }

    return true;
}

/**
 * @brief FileCollector::getObject
 * @param name
 * @param type
 * @return
 */
JsonItem
SakuraParsing::getObject(const std::string &name,
                         const std::string &type)
{
    // precheck
    if(name == "") {
        return JsonItem();
    }

    // search
    std::vector<std::pair<boost::filesystem::path, JsonItem>>::iterator it;
    for(it = m_fileContents.begin();
        it != m_fileContents.end();
        it++)
    {
        if(it->second.get("b_id").toString() == name)
        {
            if(type != ""
                    && it->second.get("b_type").toString() == type)
            {
                return it->second;
            }

            if(type == "") {
                return it->second;
            }
        }
    }

    return JsonItem();
}

/**
 * @brief FileCollector::getSeedName
 * @param index
 * @return
 */
const std::string
SakuraParsing::getSeedName(const uint32_t index)
{
    if(index >= m_fileContents.size()) {
        return std::string("");
    }

    return m_fileContents.at(index).second.get("b_id").toString();
}

/**
 * @brief ParserInit::getFilesInDir
 * @param directory
 * @return
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
            if(itr->path().leaf().string() != "templates"
                    && itr->path().leaf().string() != "files")
            {
                getFilesInDir(itr->path());
            }
        }
        else
        {
            //TODO: delete output
            std::cout<<"file: "<<itr->path().string()<<std::endl;
            m_fileContents.push_back(std::make_pair(itr->path(), JsonItem()));
        }
    }
}

/**
 * reads the content of a specific text-file
 *
 * @return string with the file-content
 */
const std::string
SakuraParsing::readFile(const std::string &filePath)
{
    // read into string
    std::ifstream inFile;
    inFile.open(filePath);
    std::stringstream strStream;
    strStream << inFile.rdbuf();
    const std::string fileContent = strStream.str();

    inFile.close();

    return fileContent;
}

}  // namespace Sakura
}  // namespace Kitsunemimi

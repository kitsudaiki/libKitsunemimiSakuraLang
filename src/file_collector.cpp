/**
 * @file        file_collector.cpp
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

#include <libKitsunemimiSakuraParser/file_collector.h>

#include <libKitsunemimiSakuraParser/sakura_parsing.h>
#include <libKitsunemimiCommon/common_methods/string_methods.h>
#include <libKitsunemimiJson/json_item.h>
#include <libKitsunemimiCommon/common_items/data_items.h>

namespace SakuraTree
{

/**
 * @brief constructor
 */
FileCollector::FileCollector() {}

/**
 * @brief SakuraCompiler::parseFiles
 * @param rootPath
 * @param seedName
 * @return
 */
JsonItem
FileCollector::parseFiles(const std::string &rootPath,
                          std::string &seedName,
                          const bool debug)
{
    SakuraParsing* sakuraParser = new SakuraParsing(debug);

    if(initFileCollector(rootPath, sakuraParser) == false)
    {
        //TODO: replace with better solution
        std::cout<<"ERROR: "<<m_errorMessage<<std::endl;
        exit(1);
    }

    if(seedName == "") {
        seedName = getSeedName(0);
    }

    JsonItem completePlan = getObject(seedName);
    assert(completePlan.isValid());

    preProcessObject(completePlan);

    delete sakuraParser;

    return completePlan;
}


/**
 * @brief SakuraCompiler::processObject
 * @param value
 * @return
 */
void
FileCollector::preProcessObject(JsonItem &object)
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
        branch = getObject(object.get("id").toString());
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
FileCollector::preProcessArray(JsonItem &object)
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
FileCollector::initFileCollector(const std::string &rootPath,
                                 SakuraParsing* sakuraParser)
{
    boost::filesystem::path rootPathObj(rootPath);    
    m_errorMessage = "";

    // precheck
    if(exists(rootPathObj) == false)
    {
        m_errorMessage = "path doesn't exist: " + rootPath;
        return false;
    }

    // get all files
    if(is_directory(rootPathObj)) {
        getFilesInDir(rootPathObj);
    } else {
        m_fileContents.push_back(std::make_pair(rootPath, JsonItem()));
    }

    // check result
    if(m_fileContents.size() == 0)
    {
        m_errorMessage = "no files found at the end of the path: " + rootPath;
        return false;
    }

    // get and parse file-contents
    for(uint32_t i = 0; i < m_fileContents.size(); i++)
    {
        const std::string filePath = m_fileContents.at(i).first.string();
        std::pair<DataItem*, bool> result = sakuraParser->parse(readFile(filePath));

        if(result.second == false)
        {
            m_errorMessage = result.first->get("error")->toString();
            delete result.first;
            // TODO: delete content of m_fileContents too
            return false;
        }

        m_fileContents[i].second = result.first->toMap();

        const std::string directoryPath = m_fileContents.at(i).first.parent_path().string();
        m_fileContents[i].second.insert("b_path",
                                        new DataValue(directoryPath),
                                        true);

        const std::string output = m_fileContents[i].second.toString(true);
        std::cout<<output<<std::endl;
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
FileCollector::getObject(const std::string &name,
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
        if(it->second.get("id").toString() == name)
        {
            if(type != ""
                    && it->second.get("type").toString() == type)
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
FileCollector::getSeedName(const uint32_t index)
{
    if(index >= m_fileContents.size()) {
        return std::string("");
    }

    return m_fileContents.at(index).second.get("name").toString();
}

/**
 * @brief ParserInit::getFilesInDir
 * @param directory
 * @return
 */
void
FileCollector::getFilesInDir(const boost::filesystem::path &directory)
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
FileCollector::readFile(const std::string &filePath)
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

}

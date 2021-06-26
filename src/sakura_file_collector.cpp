/**
 * @file        sakura_file_collector.cpp
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

#include "sakura_file_collector.h"

#include <libKitsunemimiPersistence/files/text_file.h>
#include <libKitsunemimiPersistence/files/binary_file.h>
#include <libKitsunemimiPersistence/files/file_methods.h>
#include <libKitsunemimiPersistence/logger/logger.h>

#include <libKitsunemimiSakuraLang/sakura_lang_interface.h>

#include <libKitsunemimiCommon/common_items/table_item.h>
#include <libKitsunemimiCommon/common_items/data_items.h>
#include <libKitsunemimiCommon/buffer/data_buffer.h>

namespace Kitsunemimi
{
namespace Sakura
{

SakuraFileCollector::SakuraFileCollector(SakuraLangInterface* interface)
{
    m_interface = interface;
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
SakuraFileCollector::readFilesInDir(const std::string &directoryPath,
                                    std::string &errorMessage)
{
    const bfs::path rootPath = directoryPath;

    // precheck
    if(bfs::is_directory(rootPath) == false)
    {
        TableItem errorOutput;
        initErrorOutput(errorOutput);
        errorOutput.addRow(std::vector<std::string>{"source", "while reading sakura-files"});
        errorOutput.addRow(std::vector<std::string>{"message",
                                                    "path doesn't exist or is not a directory: "
                                                    + rootPath.string()});
        errorMessage = errorOutput.toString();
        return false;
    }


    // get list the all files
    std::vector<std::string> sakuraFiles;
    if(Kitsunemimi::Persistence::listFiles(sakuraFiles, directoryPath) == false)
    {
        errorMessage = "path with sakura-files doesn't exist: " + directoryPath;
        return false;
    }

    // iterate over all files and parse them
    for(const std::string &filePath : sakuraFiles)
    {
        std::string content = "";
        if(Kitsunemimi::Persistence::readFile(content, filePath, errorMessage) == false)
        {
            errorMessage = "reading sakura-files failed with error: " + errorMessage;
            return false;
        }

        if(m_interface->addTree("", content, errorMessage) == false)
        {
            errorMessage = "parsing sakura-files failed with error: " + errorMessage;
            return false;
        }
    }

    // collect special files
    if(collectFiles(rootPath, directoryPath, errorMessage) == false) {
        return false;
    }
    if(collectResources(rootPath, directoryPath, errorMessage) == false) {
        return false;
    }
    if(collectTemplates(rootPath, directoryPath, errorMessage) == false) {
        return false;
    }

    return true;
}


/**
 * @brief collect files within a directory
 *
 * @param dirPath directory-path where to search for files
 * @param errorMessage reference to error-message
 *
 * @return true, if successful, else false
 */
bool
SakuraFileCollector::collectFiles(const bfs::path &rootPath,
                                  const bfs::path &dirPath,
                                  std::string &errorMessage)
{
    const bfs::path filesPath = dirPath / bfs::path("files");
    if(bfs::exists(filesPath))
    {
        return getFilesInDir(rootPath,
                             bfs::path(filesPath),
                             "files",
                             errorMessage);
    }

    return true;
}

/**
 * @brief collect resources within a directory
 *
 * @param dirPath directory-path where to search for resources
 * @param errorMessage reference to error-message
 *
 * @return true, if successful, else false
 */
bool
SakuraFileCollector::collectResources(const bfs::path &rootPath,
                                      const bfs::path &dirPath,
                                      std::string &errorMessage)
{
    const bfs::path ressourcePath = dirPath / bfs::path("resources");

    if(bfs::exists(ressourcePath))
    {
        return getFilesInDir(rootPath,
                             bfs::path(ressourcePath),
                             "resources",
                             errorMessage);
    }

    return true;
}

/**
 * @brief collect templates within a directory
 *
 * @param dirPath directory-path where to search for templates
 * @param errorMessage reference to error-message
 *
 * @return true, if successful, else false
 */
bool
SakuraFileCollector::collectTemplates(const bfs::path &rootPath,
                                      const bfs::path &dirPath,
                                      std::string &errorMessage)
{
    const bfs::path templatesPath = dirPath / bfs::path("templates");

    if(bfs::exists(templatesPath))
    {
        return getFilesInDir(rootPath,
                             bfs::path(templatesPath),
                             "templates",
                             errorMessage);
    }

    return true;
}

/**
 * @brief iterate over all files and directory within the current directory
 *
 * @param directory directory-path to iterate over
 * @param type type to search for
 * @param errorMessage reference to error-message
 *
 * @return true, if successful, else false
 */
bool
SakuraFileCollector::getFilesInDir(const bfs::path &rootPath,
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
            if(getFilesInDir(rootPath,
                             itr->path(),
                             type,
                             errorMessage) == false)
            {
                return false;
            }
        }
        else
        {
            bfs::path relPath = bfs::relative(itr->path(), rootPath);
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

                if(m_interface->addFile(relPath.string(), buffer) == false)
                {
                    TableItem errorOutput;
                    initErrorOutput(errorOutput);
                    errorOutput.addRow({"source", "while reading files"});
                    errorOutput.addRow({"message", "id already used: " + relPath.string()});
                    errorMessage = errorOutput.toString();
                    return false;
                }
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

                if(m_interface->addResource("", fileContent, errorMessage) == false)
                {
                    TableItem errorOutput;
                    initErrorOutput(errorOutput);
                    errorOutput.addRow({"source", "while parsing ressource-files"});
                    errorOutput.addRow({"message", "id already used: "});
                    errorMessage = errorOutput.toString();
                    return false;
                }
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

                if(m_interface->addTemplate(relPath.string(), fileContent) == false)
                {
                    TableItem errorOutput;
                    initErrorOutput(errorOutput);
                    errorOutput.addRow({"source", "while reading template-files"});
                    errorOutput.addRow({"message", "id already used: " + relPath.string()});
                    errorMessage = errorOutput.toString();
                    return false;
                }
            }
            //--------------------------------------------------------------------------------------
        }
    }

    return true;
}

/**
 * @brief initialize table for error-message output
 *
 * @param errorOutput reference to table-item for error-message output
 */
void
SakuraFileCollector::initErrorOutput(TableItem &errorOutput)
{
    errorOutput.addColumn("key");
    errorOutput.addColumn("value");
    errorOutput.addRow(std::vector<std::string>{"ERROR", " "});
    errorOutput.addRow(std::vector<std::string>{"component", "libKitsunemimiSakuraLang"});
}

}
}

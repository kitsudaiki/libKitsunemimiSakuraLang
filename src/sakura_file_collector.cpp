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

#include <libKitsunemimiCommon/files/text_file.h>
#include <libKitsunemimiCommon/files/binary_file.h>
#include <libKitsunemimiCommon/common_methods/file_methods.h>

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
 * @param error reference for error-output
 *
 * @return true, if successfule, else false
 */
bool
SakuraFileCollector::readFilesInDir(const std::string &directoryPath,
                                    ErrorContainer &error)
{
    const std::filesystem::path rootPath = directoryPath;

    // precheck
    if(std::filesystem::is_directory(rootPath) == false)
    {
        error.addMeesage("while reading sakura-files, "
                         "because path doesn't exist or is not a directory: "
                         + rootPath.string());
        LOG_ERROR(error);
        return false;
    }


    // get list the all files
    std::vector<std::string> sakuraFiles;
    if(Kitsunemimi::listFiles(sakuraFiles, directoryPath) == false)
    {
        error.addMeesage( "path with sakura-files doesn't exist: " + directoryPath);
        LOG_ERROR(error);
        return false;
    }

    // iterate over all files and parse them
    for(const std::string &filePath : sakuraFiles)
    {
        std::string content = "";
        if(Kitsunemimi::readFile(content, filePath, error) == false)
        {
            error.addMeesage("reading sakura-files failed");
            LOG_ERROR(error);
            return false;
        }

        if(m_interface->addTree("", content, error) == false)
        {
            error.addMeesage("parsing sakura-files failed");
            LOG_ERROR(error);
            return false;
        }
    }

    // collect special files
    if(collectFiles(rootPath, directoryPath, error) == false) {
        return false;
    }
    if(collectResources(rootPath, directoryPath, error) == false) {
        return false;
    }
    if(collectTemplates(rootPath, directoryPath, error) == false) {
        return false;
    }

    return true;
}


/**
 * @brief collect files within a directory
 *
 * @param rootPath root-path of the file-collecting
 * @param dirPath directory-path where to search for files
 * @param error reference for error-output
 *
 * @return true, if successful, else false
 */
bool
SakuraFileCollector::collectFiles(const std::filesystem::path &rootPath,
                                  const std::filesystem::path &dirPath,
                                  ErrorContainer &error)
{
    const std::filesystem::path filesPath = dirPath / std::filesystem::path("files");
    if(std::filesystem::exists(filesPath))
    {
        return getFilesInDir(rootPath,
                             std::filesystem::path(filesPath),
                             "files",
                             error);
    }

    return true;
}

/**
 * @brief collect resources within a directory
 *
 * @param rootPath initial path of the file collecting process
 * @param dirPath directory-path where to search for resources
 * @param error reference to error-message
 *
 * @return true, if successful, else false
 */
bool
SakuraFileCollector::collectResources(const std::filesystem::path &rootPath,
                                      const std::filesystem::path &dirPath,
                                      ErrorContainer &error)
{
    const std::filesystem::path ressourcePath = dirPath / std::filesystem::path("resources");

    if(std::filesystem::exists(ressourcePath))
    {
        return getFilesInDir(rootPath,
                             std::filesystem::path(ressourcePath),
                             "resources",
                             error);
    }

    return true;
}

/**
 * @brief collect templates within a directory
 *
 * @param rootPath initial path of the file collecting process
 * @param dirPath directory-path where to search for templates
 * @param error reference for error-output
 *
 * @return true, if successful, else false
 */
bool
SakuraFileCollector::collectTemplates(const std::filesystem::path &rootPath,
                                      const std::filesystem::path &dirPath,
                                      ErrorContainer &error)
{
    const std::filesystem::path templatesPath = dirPath / std::filesystem::path("templates");

    if(std::filesystem::exists(templatesPath))
    {
        return getFilesInDir(rootPath,
                             std::filesystem::path(templatesPath),
                             "templates",
                             error);
    }

    return true;
}

/**
 * @brief iterate over all files and directory within the current directory
 *
 * @param rootPath initial path of the file collecting process
 * @param directory directory-path to iterate over
 * @param type type to search for
 * @param error reference for error-output
 *
 * @return true, if successful, else false
 */
bool
SakuraFileCollector::getFilesInDir(const std::filesystem::path &rootPath,
                                   const std::filesystem::path &directory,
                                   const std::string &type,
                                   ErrorContainer &error)
{
    std::filesystem::directory_iterator end_itr;
    for(std::filesystem::directory_iterator itr(directory);
        itr != end_itr;
        ++itr)
    {
        if(is_directory(itr->path()))
        {
            if(getFilesInDir(rootPath,
                             itr->path(),
                             type,
                             error) == false)
            {
                return false;
            }
        }
        else
        {
            std::filesystem::path relPath = std::filesystem::relative(itr->path(), rootPath);
            //--------------------------------------------------------------------------------------
            if(type == "files")
            {
                Kitsunemimi::DataBuffer* buffer = new DataBuffer();
                Kitsunemimi::BinaryFile binFile(itr->path().string());
                bool ret = binFile.readCompleteFile(*buffer);

                if(ret == false)
                {
                    error.addMeesage("can not read file " + itr->path().string());
                    LOG_ERROR(error);
                    return false;
                }

                if(m_interface->addFile(relPath.string(), buffer) == false)
                {
                    error.addMeesage("file with the id '" + relPath.string() + "'already used");
                    LOG_ERROR(error);
                    return false;
                }
            }
            //--------------------------------------------------------------------------------------
            if(type == "resources")
            {
                // read resource-file
                std::string fileContent = "";
                bool ret = Kitsunemimi::readFile(fileContent,  itr->path().string(), error);
                if(ret == false)
                {
                    error.addMeesage("Error while reading ressource-files");
                    LOG_ERROR(error);
                    return false;
                }

                if(m_interface->addResource("", fileContent, error) == false)
                {
                    error.addMeesage("Error while parsing ressource-files");
                    LOG_ERROR(error);
                    return false;
                }
            }
            //--------------------------------------------------------------------------------------
            if(type == "templates")
            {
                std::string fileContent = "";
                bool ret = Kitsunemimi::readFile(fileContent, itr->path().string(), error);
                if(ret == false)
                {
                    error.addMeesage("Error while reading template-files");
                    LOG_ERROR(error);
                    return false;
                }

                if(m_interface->addTemplate(relPath.string(), fileContent) == false)
                {
                    error.addMeesage("template-file with the id '"
                                     + relPath.string()
                                     + "'already used");
                    LOG_ERROR(error);
                    return false;
                }
            }
            //--------------------------------------------------------------------------------------
        }
    }

    return true;
}

} // namespace Sakura
} // namespace Kitsunemimi

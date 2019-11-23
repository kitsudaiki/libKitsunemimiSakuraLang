/**
 * @file        file_collector.h
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

#ifndef FILE_COLLECTOR_H
#define FILE_COLLECTOR_H

#include <string>
#include <vector>
#include <utility>
#include <assert.h>

#include <boost/filesystem.hpp>

namespace Kitsunemimi
{
namespace Common
{
class DataItem;
class DataValue;
class DataMap;
}
namespace Json
{
class JsonItem;
}
namespace Sakura
{
class SakuraParsing;
}
}
using Kitsunemimi::Json::JsonItem;
using Kitsunemimi::Common::DataItem;
using Kitsunemimi::Common::DataValue;
using Kitsunemimi::Common::DataMap;
using namespace boost::filesystem;
using Kitsunemimi::Sakura::SakuraParsing;

namespace SakuraTree
{

class FileCollector
{
public:
    FileCollector();

    JsonItem parseFiles(const std::string &rootPath,
                        std::string &seedName,
                        const bool debug = false);

private:
    std::vector<std::pair<boost::filesystem::path, JsonItem>> m_fileContents;
    std::string m_errorMessage = "";

    bool initFileCollector(const std::string &rootPath,
                           SakuraParsing* sakuraParser);
    JsonItem getObject(const std::string &name,
                       const std::string &type="");
    const std::string getSeedName(const uint32_t index);

    void getFilesInDir(const path &directory);
    const std::string readFile(const std::string &filePath);

    void preProcessArray(JsonItem &object);
    void preProcessObject(JsonItem &object);
};

}

#endif // FILE_COLLECTOR_H
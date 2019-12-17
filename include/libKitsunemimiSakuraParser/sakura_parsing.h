/**
 * @file        sakura_parsing.h
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

#ifndef SAKURA_PARSING_H
#define SAKURA_PARSING_H

#include <string>
#include <vector>
#include <utility>
#include <assert.h>
#include <fstream>

#include <boost/filesystem.hpp>
#include <libKitsunemimiCommon/common_items/data_items.h>
#include <libKitsunemimiCommon/common_items/table_item.h>
#include <libKitsunemimiJson/json_item.h>

using Kitsunemimi::Common::DataItem;
using Kitsunemimi::Common::DataValue;
using Kitsunemimi::Common::DataMap;
using Kitsunemimi::Json::JsonItem;
using Kitsunemimi::Common::TableItem;
using namespace boost::filesystem;

namespace Kitsunemimi
{
namespace Sakura
{

class SakuraParserInterface;

class SakuraParsing
{
public:
    SakuraParsing(const bool debug = false);

    JsonItem parseFiles(const std::string &rootPath,
                        const std::string &seedName="");
    TableItem getError() const;

private:
    std::vector<std::pair<boost::filesystem::path, JsonItem>> m_fileContents;
    SakuraParserInterface* m_parser = nullptr;
    TableItem m_errorMessage;

    bool initFileCollector(const std::string &rootPath);
    JsonItem getObject(const std::string &name,
                       const std::string &type="");
    const std::string getSeedName(const uint32_t index);

    void getFilesInDir(const path &directory);
    const std::string readFile(const std::string &filePath);

    void preProcessArray(JsonItem &object);
    void preProcessObject(JsonItem &object);
};

}  // namespace Sakura
}  // namespace Kitsunemimi

#endif // SAKURA_PARSING_H

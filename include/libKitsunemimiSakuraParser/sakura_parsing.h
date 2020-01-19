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

using Kitsunemimi::DataItem;
using Kitsunemimi::DataValue;
using Kitsunemimi::DataMap;
using Kitsunemimi::Json::JsonItem;
using Kitsunemimi::TableItem;
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
    ~SakuraParsing();

    bool parseFiles(const std::string &rootPath);
    TableItem getError() const;
    JsonItem getParsedFileContent(const std::string &name="");

private:
    std::vector<std::pair<boost::filesystem::path, JsonItem>> m_fileContents;
    SakuraParserInterface* m_parser = nullptr;
    TableItem m_errorMessage;
    bool m_debug = false;

    bool parseAllFiles(const std::string &rootPath);
    void getFilesInDir(const path &directory);
};

}  // namespace Sakura
}  // namespace Kitsunemimi

#endif // SAKURA_PARSING_H

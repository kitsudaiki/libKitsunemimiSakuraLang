/**
 * @file        sakura_parser_interface.h
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

#ifndef KITSUNEMIMI_SAKURA_LANG_PARSER_INTERFACE_H
#define KITSUNEMIMI_SAKURA_LANG_PARSER_INTERFACE_H

#include <vector>
#include <string>

#include <libKitsunemimiCommon/items/data_items.h>
#include <libKitsunemimiCommon/items/table_item.h>
#include <libKitsunemimiCommon/logger.h>

#include <iostream>

namespace Kitsunemimi
{
namespace Sakura
{
class location;
class SakuraItem;
class TreeItem;

class SakuraParserInterface
{

public:
    SakuraParserInterface(const bool traceParsing);
    ~SakuraParserInterface();

    TreeItem* parseTreeString(const std::string &name,
                              const std::string &content,
                              ErrorContainer &error);

    // connection the the scanner and parser
    void scan_begin(const std::string &inputString);
    void scan_end();
    bool parse(const std::string &inputString,
               const std::string &filePath);

    // output-handling
    void setOutput(SakuraItem* output);
    SakuraItem* getOutput() const;

    // Error handling.
    void error(const Kitsunemimi::Sakura::location &location,
               const std::string& message,
               const bool customError=false);
    TableItem getErrorMessage() const;

    const std::string removeQuotes(const std::string &input);
    std::vector<std::string> m_registeredKeys;
    bool isKeyRegistered(const std::string &key);

private:
    bool m_traceParsing = false;
    std::string m_inputString = "";
    SakuraItem* m_output = nullptr;
    TableItem m_errorMessage;
};

}  // namespace Sakura
}  // namespace Kitsunemimi

#endif // KITSUNEMIMI_SAKURA_LANG_PARSER_INTERFACE_H

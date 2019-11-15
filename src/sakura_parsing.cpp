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
#include <libKitsunemimiCommon/common_items/data_items.h>

namespace Kitsunemimi
{
namespace Sakura
{

/**
 * constructor
 */
SakuraParsing::SakuraParsing(const bool traceParsing)
{
    m_parser = new SakuraParserInterface(traceParsing);
}

/**
 * destructor
 */
SakuraParsing::~SakuraParsing()
{
    delete m_parser;
}

/**
 * Public convert-method for the external using. At first it parse the template-string
 * and then it merge the parsed information with the content of the data-input.
 *
 * @return Pair of string and boolean where the boolean shows
 *         if the parsing and converting were successful
 *         and the string contains the output-string, if the search was successful
 *         else the string contains the error-message
 */
std::pair<Common::DataItem*, bool>
SakuraParsing::parse(const std::string &inputString)
{
    std::pair<Common::DataItem*, bool> result;

    // parse sakura-template into a data-tree
    result.second = m_parser->parse(inputString);

    // process a failure
    if(result.second) {
        result.first = m_parser->getOutput();
    } else {
        result.first = m_parser->getErrorMessage();
    }

    return result;
}

}  // namespace Sakura
}  // namespace Kitsunemimi

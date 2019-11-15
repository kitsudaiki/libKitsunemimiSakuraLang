/**
 * @file        sakura_parser_interface.cpp
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

#include <sakura_parsing/sakura_parser_interface.h>
#include <sakura_parser.h>
#include <libKitsunemimiSakuraParser/sakura_parsing.h>

#include <libKitsunemimiCommon/common_methods/string_methods.h>

# define YY_DECL \
    Kitsunemimi::Sakura::SakuraParser::symbol_type sakuralex (Kitsunemimi::Sakura::SakuraParserInterface& driver)
YY_DECL;

using Kitsunemimi::Common::DataItem;
using Kitsunemimi::Common::DataArray;
using Kitsunemimi::Common::DataValue;
using Kitsunemimi::Common::DataMap;

namespace Kitsunemimi
{
namespace Sakura
{
using Common::splitStringByDelimiter;

/**
 * The class is the interface for the bison-generated parser.
 * It starts the parsing-process and store the returned values.
 */
SakuraParserInterface::SakuraParserInterface(const bool traceParsing)
{
    m_traceParsing = traceParsing;
    m_errorMessage = new DataMap();
}

/**
 * Start the scanner and parser
 *
 * @return true, if parsing was successful, else false
 */
bool
SakuraParserInterface::parse(const std::string &inputString)
{
    // init global values
    m_inputString = inputString;
    m_errorMessage = new DataMap();
    m_output = nullptr;

    // run parser-code
    this->scan_begin(inputString);
    Kitsunemimi::Sakura::SakuraParser parser(*this);
    int res = parser.parse();
    this->scan_end();

    if(res != 0) {
        return false;
    }

    return true;
}

/**
 * @brief SakuraParserInterface::setOutput
 * @param output
 */
void
SakuraParserInterface::setOutput(Common::DataItem* output)
{
    m_output = output;
}

/**
 * @brief SakuraParserInterface::getOutput
 * @return
 */
Common::DataItem* SakuraParserInterface::getOutput() const
{
    return m_output;
}

/**
 * Is called from the parser in case of an error
 *
 * @param message error-specific message from the parser
 */
void
SakuraParserInterface::error(const Kitsunemimi::Sakura::location& location,
                             const std::string& message)
{
    // get the broken part of the parsed string
    const uint32_t errorStart = location.begin.column;
    const uint32_t errorLength = location.end.column - location.begin.column;
    const uint32_t linenumber = location.begin.line;

    const std::vector<std::string> splittedContent = splitStringByDelimiter(m_inputString, '\n');

    // -1 because the number starts for user-readability at 1 instead of 0
    const std::string errorStringPart = splittedContent[linenumber - 1].substr(errorStart - 1,
                                                                               errorLength);
    // build error-message
    std::string errorString = "";
    errorString += "error while parsing sakura-file \n";
    errorString += "parser-message: " + message + " \n";
    errorString += "line-number: " + std::to_string(linenumber) + " \n";
    errorString += "position in line: " + std::to_string(location.begin.column) + " \n";
    errorString += "broken part in template: \"" + errorStringPart + "\" \n";

    m_errorMessage->toMap()->insert("error", new DataValue(errorString));
}

/**
 * getter fot the error-message in case of an error while parsing
 *
 * @return error-message
 */
Common::DataItem* SakuraParserInterface::getErrorMessage() const
{
    return m_errorMessage;
}

/**
 * @brief SakuraParserInterface::removeQuotes
 * @param input
 * @return
 */
std::string
SakuraParserInterface::removeQuotes(std::string input)
{
    if(input.length() == 0) {
        return input;
    }

    if(input[0] == '\"' && input[input.length()-1] == '\"')
    {
        std::string result = "";
        for(uint32_t i = 1; i < input.length()-1; i++)
        {
            result += input[i];
        }
        return result;
    }

    return input;
}

}  // namespace Sakura
}  // namespace Kitsunemimi

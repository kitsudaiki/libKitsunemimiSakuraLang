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

#include <parsing/sakura_parser_interface.h>
#include <sakura_parser.h>
#include <items/sakura_items.h>

#include <libKitsunemimiCommon/methods/string_methods.h>

# define YY_DECL \
    Kitsunemimi::Sakura::SakuraParser::symbol_type sakuralex (Kitsunemimi::Sakura::SakuraParserInterface& driver)
YY_DECL;

using Kitsunemimi::DataItem;
using Kitsunemimi::DataArray;
using Kitsunemimi::DataValue;
using Kitsunemimi::DataMap;

namespace Kitsunemimi
{
namespace Sakura
{
using Kitsunemimi::splitStringByDelimiter;

/**
 * @brief constructor
 *
 * @param traceParsing true to enable debug-output of the parser
 * @param sakuraParsing pointer to SakuraParsing-object to write the result back
 */
SakuraParserInterface::SakuraParserInterface(const bool traceParsing)
{
    m_traceParsing = traceParsing;
}

/**
 * @brief destructor
 */
SakuraParserInterface::~SakuraParserInterface()
{
    if(m_output != nullptr)
    {
        delete m_output;
        m_output = nullptr;
    }
}

/**
 * @brief parse string
 *
 * @param inputString string to parse
 * @param filePath absolute path to the file, where the content belongs to for the case of and
 *                 error for easier debugging
 *
 * @return true, if parsing was successful, else false
 */
bool
SakuraParserInterface::parse(const std::string &inputString,
                             const std::string &filePath)
{
    // init global values
    m_inputString = inputString;
    m_registeredKeys.clear();
    m_registeredKeys.push_back("blossom_output");

    // init error-message
    m_errorMessage.clearTable();
    m_errorMessage.addColumn("key");
    m_errorMessage.addColumn("value");
    m_errorMessage.addRow(std::vector<std::string>{"ERROR", " "});

    if(filePath != "") {
        m_errorMessage.addRow(std::vector<std::string>{"file", filePath});
    }

    // free and reset output to avoid memory-leak
    if(m_output != nullptr)
    {
        delete m_output;
        m_output = nullptr;
    }

    // run parser-code
    this->scan_begin(inputString);
    Kitsunemimi::Sakura::SakuraParser parser(*this);
    const int res = parser.parse();
    this->scan_end();

    if(res != 0) {
        return false;
    }

    return true;
}

/**
 * @brief setter for the output-variable
 */
void
SakuraParserInterface::setOutput(SakuraItem* output)
{
    m_output = output;
}

/**
 * @brief getter for the output-variable
 */
SakuraItem*
SakuraParserInterface::getOutput() const
{
    return m_output->copy();
}

/**
 * @brief Is called from the parser in case of an error
 *
 * @param location locaion-information of the error-position in the parsed file
 * @param message error-specific message from the parser
 * @param customError true to avoid a position-in line, because this would be missleading in
 *                    a custom error. (Default=false)
 */
void
SakuraParserInterface::error(const Kitsunemimi::Sakura::location& location,
                             const std::string& message,
                             const bool customError)
{
    // get the broken part of the parsed string
    const uint32_t errorStart = location.begin.column;
    const uint32_t errorLength = location.end.column - location.begin.column;
    const uint32_t linenumber = location.begin.line;

    std::vector<std::string> splittedContent;
    splitStringByDelimiter(splittedContent, m_inputString, '\n');

    // build error-message
    std::string errorString = "";
    m_errorMessage.addRow(std::vector<std::string>{"component", "parser"});
    m_errorMessage.addRow(std::vector<std::string>{"source", "while parsing sakura-file"});
    m_errorMessage.addRow(std::vector<std::string>{"message", message});
    m_errorMessage.addRow(std::vector<std::string>{"line-number", std::to_string(linenumber)});

    // add additional position information
    if(customError == false)
    {
        if(splittedContent[linenumber - 1].size() > errorStart-1+errorLength)
        {
            m_errorMessage.addRow(std::vector<std::string>
            {
                "position in line",
                std::to_string(location.begin.column)
            });
            m_errorMessage.addRow(std::vector<std::string>
            {
                "broken part in string",
                "\"" + splittedContent[linenumber - 1].substr(errorStart - 1, errorLength) + "\""
            });
        }
        else
        {
            m_errorMessage.addRow(std::vector<std::string>
            {
                "position in line",
                "UNKNOWN POSITION (maybe a string was not closed)"
            });
        }
    }
}

/**
 * getter for the error-message in case of an error while parsing
 *
 * @return error-message as table-item
 */
TableItem SakuraParserInterface::getErrorMessage() const
{
    return m_errorMessage;
}

/**
 * @brief check if a key is in the list of registerd key
 *
 * @param key key to check
 *
 * @return true if key in list, else false
 */
bool
SakuraParserInterface::isKeyRegistered(const std::string &key)
{
    std::vector<std::string>::const_iterator it;
    for(it = m_registeredKeys.begin();
        it != m_registeredKeys.end();
        it++)
    {
        if(*it == key) {
            return true;
        }
    }
    return false;
}

/**
 * @brief remove \" at the start and the end of a string
 *
 * @param input input-string which should be modified
 *
 * @return input-string without \" at the start and the end
 */
const std::string
SakuraParserInterface::removeQuotes(const std::string &input)
{
    // precheck
    if(input.length() == 0) {
        return input;
    }

    // remove double-quotes
    if(input[0] == '\"' && input[input.length()-1] == '\"')
    {
        std::string result = "";
        for(uint32_t i = 1; i < input.length()-1; i++) {
            result += input[i];
        }

        return result;
    }

    return input;
}


/**
 * @brief parse single string without storing into sakura-garden
 *
 * @param name name for identification in debug and error-output
 * @param content string to parse
 * @param error reference for error-output
 *
 * @return parsed tree-item, if successfule, else nullptr
 */
TreeItem*
SakuraParserInterface::parseTreeString(const std::string &name,
                                       const std::string &content,
                                       ErrorContainer &error)
{
    // parse
    const bool parserResult = parse(content, "");

    if(parserResult == false)
    {
        TableItem errorOutput = getErrorMessage();
        error.addMeesage(errorOutput.toString(200, true));
        errorOutput.clearTable();
        return nullptr;
    }

    TreeItem* parsetItem = dynamic_cast<TreeItem*>(getOutput());

    if(parsetItem == nullptr) {
        return nullptr;
    }

    // update content
    parsetItem->unparsedConent = content;
    parsetItem->relativePath = name;

    return parsetItem;
}

} // namespace Sakura
} // namespace Kitsunemimi

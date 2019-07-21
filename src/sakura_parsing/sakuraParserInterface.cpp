/**
 *  @file    sakuraParserInterface.cpp
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#include <sakura_parsing/sakuraParserInterface.h>
#include <sakuraparser.h>

# define YY_DECL \
    Kitsune::Sakura::SakuraParser::symbol_type sakuralex (Kitsune::Sakura::SakuraParserInterface& driver)
YY_DECL;

namespace Kitsune
{
namespace Sakura
{

/**
 * The class is the interface for the bison-generated parser.
 * It starts the parsing-process and store the returned values.
 */
SakuraParserInterface::SakuraParserInterface(const bool traceParsing)
{
    m_traceParsing = traceParsing;
    m_errorMessage = new JsonObject();
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
    m_errorMessage = new JsonObject();
    m_output = nullptr;

    // run parser-code
    this->scan_begin(inputString);
    Kitsune::Sakura::SakuraParser parser(*this);
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
SakuraParserInterface::setOutput(JsonObject *output)
{
    m_output = output;
}

/**
 * @brief SakuraParserInterface::getOutput
 * @return
 */
JsonObject*
SakuraParserInterface::getOutput() const
{
    return m_output;
}

/**
 * Is called from the parser in case of an error
 *
 * @param message error-specific message from the parser
 */
void
SakuraParserInterface::error(const Kitsune::Sakura::location& location,
                             const std::string& message)
{
    // get the broken part of the parsed string
    const uint32_t errorStart = location.begin.column;
    const uint32_t errorLength = location.end.column - location.begin.column;
    const std::string errorStringPart = m_inputString.substr(errorStart, errorLength);

    // build error-message
    std::string errorString = "";
    errorString += "error while parsing jinja2-template \n";
    errorString += "parser-message: " + message + " \n";
    errorString += "line-number: " + std::to_string(location.begin.line) + " \n";
    errorString += "position in line: " + std::to_string(location.begin.column) + " \n";
    errorString += "broken part in template: \"" + errorStringPart + "\" \n";

    m_errorMessage->insert("error", new JsonValue(errorString));
}

/**
 * getter fot the error-message in case of an error while parsing
 *
 * @return error-message
 */
JsonObject*
SakuraParserInterface::getErrorMessage() const
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
}  // namespace Kitsune

/**
 *  @file    libKitsuneSakuraParser.cpp
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#include <libKitsuneSakuraParser.h>

#include <sakura_parsing/sakuraParserInterface.h>

namespace Kitsune
{
namespace Sakura
{

/**
 * constructor
 */
LibKitsuneSakuraParser::LibKitsuneSakuraParser(const bool traceParsing)
{
    m_parser = new SakuraParserInterface(traceParsing);
}

/**
 * destructor
 */
LibKitsuneSakuraParser::~LibKitsuneSakuraParser()
{
    delete m_parser;
}

/**
 * Public convert-method for the external using. At first it parse the template-string
 * and then it merge the parsed information with the content of the json-input.
 *
 * @return Pair of string and boolean where the boolean shows
 *         if the parsing and converting were successful
 *         and the string contains the output-string, if the search was successful
 *         else the string contains the error-message
 */
std::pair<Json::JsonObject*, bool>
LibKitsuneSakuraParser::convert(const std::string &templateString)
{
    std::pair<Json::JsonObject*, bool> result;

    // parse sakura-template into a json-tree
    result.second = m_parser->parse(templateString);

    // process a failure
    if(result.second) {
        result.first = m_parser->getOutput();
    } else {
        result.first = m_parser->getErrorMessage();
    }

    return result;
}

}  // namespace Sakura
}  // namespace Kitsune

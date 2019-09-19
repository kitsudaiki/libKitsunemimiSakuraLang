/**
 *  @file    sakura_converter.cpp
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#include <sakura_converter.h>

#include <sakura_parsing/sakura_parser_interface.h>
#include <common_items/data_items.h>

namespace Kitsune
{
namespace Sakura
{

/**
 * constructor
 */
SakuraConverter::SakuraConverter(const bool traceParsing)
{
    m_parser = new SakuraParserInterface(traceParsing);
}

/**
 * destructor
 */
SakuraConverter::~SakuraConverter()
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
SakuraConverter::parse(const std::string &inputString)
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
}  // namespace Kitsune

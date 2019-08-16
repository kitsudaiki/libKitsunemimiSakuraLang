/**
 *  @file    sakura_converter.hpp
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#ifndef LIBKITSUNE_SAKURA_PARSER_HPP
#define LIBKITSUNE_SAKURA_PARSER_HPP

#include <utility>
#include <string>

namespace Kitsune
{
namespace Common {
class DataItem;
}
namespace Sakura
{
class SakuraParserInterface;

class SakuraConverter
{
public:
    SakuraConverter(const bool traceParsing);
    ~SakuraConverter();

    std::pair<Common::DataItem*, bool> parse(const std::string &inputString);

private:
    SakuraParserInterface* m_parser = nullptr;
};

}  // namespace Sakura
}  // namespace Kitsune

#endif // LIBKITSUNE_SAKURA_PARSER_HPP

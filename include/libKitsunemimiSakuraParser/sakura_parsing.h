/**
 *  @file    sakura_converter.h
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#ifndef LIBKITSUNE_SAKURA_PARSER_H
#define LIBKITSUNE_SAKURA_PARSER_H

#include <utility>
#include <string>

namespace Kitsunemimi
{
namespace Common {
class DataItem;
}
namespace Sakura
{
class SakuraParserInterface;

class SakuraParsing
{
public:
    SakuraParsing(const bool traceParsing);
    ~SakuraParsing();

    std::pair<Common::DataItem*, bool> parse(const std::string &inputString);

private:
    SakuraParserInterface* m_parser = nullptr;
};

}  // namespace Sakura
}  // namespace Kitsunemimi

#endif // LIBKITSUNE_SAKURA_PARSER_H

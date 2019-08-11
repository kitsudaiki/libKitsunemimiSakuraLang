/**
 *  @file    libKitsuneSakuraParser.h
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#ifndef LIBKITSUNESAKURAPARSER_H
#define LIBKITSUNESAKURAPARSER_H

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

class LibKitsuneSakuraParser
{
public:
    LibKitsuneSakuraParser(const bool traceParsing);
    ~LibKitsuneSakuraParser();

    std::pair<Common::DataItem*, bool> convert(const std::string &inputString);

private:
    SakuraParserInterface* m_parser = nullptr;
};

}  // namespace Sakura
}  // namespace Kitsune

#endif // LIBKITSUNESAKURAPARSER_H

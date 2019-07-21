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
#include <jsonObjects.h>

namespace Kitsune
{
namespace Sakura
{
class SakuraParserInterface;

class LibKitsuneSakuraParser
{
public:
    LibKitsuneSakuraParser(const bool traceParsing);
    ~LibKitsuneSakuraParser();

    std::pair<Json::JsonObject*, bool> convert(const std::string &templateString);

private:
    SakuraParserInterface* m_parser = nullptr;
};

}  // namespace Sakura
}  // namespace Kitsune

#endif // LIBKITSUNESAKURAPARSER_H

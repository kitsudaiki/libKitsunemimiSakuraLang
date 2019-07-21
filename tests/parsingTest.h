/**
 *  @file    parsingTest.h
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#ifndef PARSERTEST_H
#define PARSERTEST_H

#include <testing/commonTest.h>

namespace Kitsune
{
namespace Sakura
{
class LibKitsuneSakuraParser;

class ParsingTest : public Kitsune::CommonTest
{
public:
    ParsingTest();

private:
    void initTestCase();
    void parseBranchTest();
    void parseTreeTest();
    void parseForestTest();
    void cleanupTestCase();

    LibKitsuneSakuraParser* m_parser = nullptr;
};

}
}  // namespace Kitsune

#endif // PARSERTEST_H

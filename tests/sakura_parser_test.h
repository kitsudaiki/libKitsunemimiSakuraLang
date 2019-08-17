/**
 *  @file    sakura_parser_test.h
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#ifndef SAKURA_PARSER_TEST_H
#define SAKURA_PARSER_TEST_H

#include <testing/test.h>

namespace Kitsune
{
namespace Sakura
{
class SakuraConverter;

class ParsingTest : public Kitsune::Common::Test
{
public:
    ParsingTest();

private:
    void initTestCase();
    void parseBranchTest();
    void parseTreeTest();
    void parseForestTest();
    void cleanupTestCase();

    SakuraConverter* m_parser = nullptr;
};

}  // namespace Sakura
}  // namespace Kitsune

#endif // SAKURA_PARSER_TEST_H

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

#include <test.h>

namespace Kitsunemimi
{
namespace Sakura
{
class SakuraConverter;

class ParsingTest : public Kitsunemimi::Common::Test
{
public:
    ParsingTest();

private:
    void initTestCase();
    void parseBranchTest();
    void parseTreeTest();
    void cleanupTestCase();

    SakuraConverter* m_parser = nullptr;
};

}  // namespace Sakura
}  // namespace Kitsunemimi

#endif // SAKURA_PARSER_TEST_H

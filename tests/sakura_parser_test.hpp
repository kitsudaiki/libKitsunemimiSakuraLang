/**
 *  @file    sakura_parser_test.hpp
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#ifndef SAKURA_PARSER_TEST_HPP
#define SAKURA_PARSER_TEST_HPP

#include <testing/test.hpp>

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

#endif // SAKURA_PARSER_TEST_HPP

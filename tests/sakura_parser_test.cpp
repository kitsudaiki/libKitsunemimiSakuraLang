/**
 *  @file    sakura_parser_test.cpp
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#include "sakura_parser_test.hpp"

#include <sakura_converter.hpp>
#include <test_strings/branch_test_string.hpp>
#include <test_strings/forest_test_string.hpp>
#include <test_strings/tree_test_string.hpp>

#include <data_structure/dataItems.hpp>

using Kitsune::Common::DataItem;
using Kitsune::Common::DataArray;
using Kitsune::Common::DataValue;
using Kitsune::Common::DataObject;

namespace Kitsune
{
namespace Sakura
{

ParsingTest::ParsingTest() : Kitsune::Common::Test("ParsingTest")
{
    initTestCase();
    parseBranchTest();
    parseTreeTest();
    parseForestTest();
    cleanupTestCase();
}

void ParsingTest::initTestCase()
{
    m_parser = new SakuraConverter(false);
}

void ParsingTest::parseBranchTest()
{
    std::pair<DataItem*, bool> result = m_parser->parse(testBranchString);
    UNITTEST(result.second, true);
    std::string output = "";
    result.first->print(&output, true);
    std::cout<<"output: "<<output<<std::endl;;

}

void ParsingTest::parseTreeTest()
{
    std::pair<DataItem*, bool> result = m_parser->parse(testTreeString);
    UNITTEST(result.second, true);
    std::string output = "";
    result.first->print(&output, true);
    std::cout<<"output: "<<output<<std::endl;;
}

void ParsingTest::parseForestTest()
{
    std::pair<DataItem*, bool> result = m_parser->parse(testForestString);
    UNITTEST(result.second, true);
    std::string output = "";
    result.first->print(&output, true);
    std::cout<<"output: "<<output<<std::endl;;
}

void ParsingTest::cleanupTestCase()
{
    delete m_parser;
}

}  // namespace Sakura
}  // namespace Kitsune


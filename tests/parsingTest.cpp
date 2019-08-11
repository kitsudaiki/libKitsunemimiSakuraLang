/**
 *  @file    parsingTest.cpp
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#include "parsingTest.h"

#include <libKitsuneSakuraParser.hpp>
#include <testStrings/branchTestString.h>
#include <testStrings/forestTestString.h>
#include <testStrings/treeTestString.h>

#include <data_structure/dataItems.hpp>

using namespace Kitsune::Data;

namespace Kitsune
{
namespace Sakura
{

ParsingTest::ParsingTest() : Kitsune::CommonTest("ParsingTest")
{
    initTestCase();
    parseBranchTest();
    parseTreeTest();
    parseForestTest();
    cleanupTestCase();
}

void ParsingTest::initTestCase()
{
    m_parser = new LibKitsuneSakuraParser(false);
}

void ParsingTest::parseBranchTest()
{
    std::pair<Data::DataItem*, bool> result = m_parser->convert(testBranchString);
    UNITTEST(result.second, true);
    std::string output = "";
    result.first->print(&output, true);
    std::cout<<"output: "<<output<<std::endl;;

}

void ParsingTest::parseTreeTest()
{
    std::pair<Data::DataItem*, bool> result = m_parser->convert(testTreeString);
    UNITTEST(result.second, true);
    std::string output = "";
    result.first->print(&output, true);
    std::cout<<"output: "<<output<<std::endl;;
}

void ParsingTest::parseForestTest()
{
    std::pair<Data::DataItem*, bool> result = m_parser->convert(testForestString);
    UNITTEST(result.second, true);
    std::string output = "";
    result.first->print(&output, true);
    std::cout<<"output: "<<output<<std::endl;;
}

void ParsingTest::cleanupTestCase()
{
    delete m_parser;
}

}
}  // namespace Kitsune


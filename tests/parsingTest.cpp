/**
 *  @file    parsingTest.cpp
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#include "parsingTest.h"

#include <libKitsuneSakuraParser.h>
#include <testStrings/branchTestString.h>
#include <testStrings/forestTestString.h>
#include <testStrings/treeTestString.h>

#include <jsonItems.hpp>

using namespace Kitsune::Json;

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
    std::pair<Json::JsonItem*, bool> result = m_parser->convert(testBranchString);
    UNITTEST(result.second, true);
    std::string output = "";
    result.first->print(&output, true);
    std::cout<<"output: "<<output<<std::endl;;

}

void ParsingTest::parseTreeTest()
{
    std::pair<Json::JsonItem*, bool> result = m_parser->convert(testTreeString);
    UNITTEST(result.second, true);
    std::string output = "";
    result.first->print(&output, true);
    std::cout<<"output: "<<output<<std::endl;;
}

void ParsingTest::parseForestTest()
{
    std::pair<Json::JsonItem*, bool> result = m_parser->convert(testForestString);
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


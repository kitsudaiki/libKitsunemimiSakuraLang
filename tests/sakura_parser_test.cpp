/**
 * @file        sakura_parser_test.cpp
 *
 * @author      Tobias Anker <tobias.anker@kitsunemimi.moe>
 *
 * @copyright   Apache License Version 2.0
 *
 *      Copyright 2019 Tobias Anker
 *
 *      Licensed under the Apache License, Version 2.0 (the "License");
 *      you may not use this file except in compliance with the License.
 *      You may obtain a copy of the License at
 *
 *          http://www.apache.org/licenses/LICENSE-2.0
 *
 *      Unless required by applicable law or agreed to in writing, software
 *      distributed under the License is distributed on an "AS IS" BASIS,
 *      WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *      See the License for the specific language governing permissions and
 *      limitations under the License.
 */

#include "sakura_parser_test.h"

#include <sakura_parsing.h>
#include <test_strings/branch_test_string.h>

#include <common_items/data_items.h>

using Kitsunemimi::Common::DataItem;
using Kitsunemimi::Common::DataArray;
using Kitsunemimi::Common::DataValue;
using Kitsunemimi::Common::DataMap;

namespace Kitsunemimi
{
namespace Sakura
{

ParsingTest::ParsingTest() : Kitsunemimi::Common::Test("ParsingTest")
{
    initTestCase();
    parseBranchTest();
    cleanupTestCase();
}

void ParsingTest::initTestCase()
{
    m_parser = new SakuraParsing(true);
}

void ParsingTest::parseBranchTest()
{
    std::pair<DataItem*, bool> result = m_parser->parse(testBranchString);
    TEST_EQUAL(result.second, true);
    std::string output = result.first->toString(true);
    std::cout<<"output: "<<output<<std::endl;;

}

void ParsingTest::cleanupTestCase()
{
    delete m_parser;
}

}  // namespace Sakura
}  // namespace Kitsunemimi


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

#include <parsing/sakura_parsing.h>
#include <items/sakura_items.h>
#include <sakura_garden.h>

#include <test_strings/test_strings.h>

#include <libKitsunemimiCommon/common_items/data_items.h>
#include <libKitsunemimiPersistence/files/text_file.h>

namespace Kitsunemimi
{
namespace Sakura
{

ParsingTest::ParsingTest()
    : Kitsunemimi::CompareTestHelper("ParsingTest")
{
    initTestCase();
    parseTree_Test();
    cleanupTestCase();
}

void
ParsingTest::initTestCase()
{
    m_parser = new SakuraParsing(true);
}

void
ParsingTest::parseTree_Test()
{
    std::string errorMessage = "";
    Kitsunemimi::Persistence::writeFile("/tmp/sakura_parser_test.sakura",
                                        testTreeString,
                                        errorMessage,
                                        true);
    SakuraGarden garden(true);
    bool success = m_parser->parseTreeString(garden,
                                             "sakura_parser_test.sakura",
                                             testTreeString,
                                             errorMessage);

    TEST_EQUAL(success, true);
    if(success == false) {
        std::cout<<errorMessage<<std::endl;
    }
}

void
ParsingTest::cleanupTestCase()
{
    delete m_parser;
}

}  // namespace Sakura
}  // namespace Kitsunemimi


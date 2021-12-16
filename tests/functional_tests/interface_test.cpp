/**
 * @file       interface_test.cpp
 *
 * @author     Tobias Anker <tobias.anker@kitsunemimi.moe>
 *
 * @copyright  Apache License Version 2.0
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

#include "interface_test.h"

#include <test_blossom.h>
#include <standalone_blossom.h>

#include <libKitsunemimiSakuraLang/sakura_lang_interface.h>
#include <libKitsunemimiSakuraLang/blossom.h>

#include <libKitsunemimiCommon/files/text_file.h>
#include <libKitsunemimiCommon/buffer/data_buffer.h>

namespace Kitsunemimi
{
namespace Sakura
{

/**
 * @brief Interface_Test::Interface_Test
 */
Interface_Test::Interface_Test() :
    Kitsunemimi::CompareTestHelper("Interface_Test")
{
    blossomMethods_test();
    addAndGet_test();
    runAndTriggerTree_test();
    runAndTriggerBlossom_test();
}

/**
 * @brief Interface_Test::blossomMethods_test
 */
void
Interface_Test::blossomMethods_test()
{
    SakuraLangInterface* interface = SakuraLangInterface::getInstance();
    TestBlossom* testBlossom = new TestBlossom(this);
    StandaloneBlossom* standaloneBlossom = new StandaloneBlossom(this);

    // test addBlossom
    TEST_EQUAL(interface->addBlossom("test1", "test2", testBlossom), true);
    TEST_EQUAL(interface->addBlossom("test1", "test2", testBlossom), false);
    TEST_EQUAL(interface->addBlossom("-", "standalone", standaloneBlossom), true);

    // test doesBlossomExist
    TEST_EQUAL(interface->doesBlossomExist("test1", "test2"), true);
    TEST_EQUAL(interface->doesBlossomExist("test1", "fail"), false);
    TEST_EQUAL(interface->doesBlossomExist("fail", "test2"), false);

    // test getBlossom
    bool isNullptr = false;
    Blossom* returnBlossom = interface->getBlossom("test1", "test2");
    isNullptr = returnBlossom == nullptr;
    TEST_EQUAL(isNullptr, false);

    if(isNullptr) {
        return;
    }

    TestBlossom* convertedBlossom = dynamic_cast<TestBlossom*>(returnBlossom);
    isNullptr = convertedBlossom == nullptr;
    TEST_EQUAL(isNullptr, false);
}

/**
 * @brief Interface_Test::addAndGet_test
 */
void
Interface_Test::addAndGet_test()
{
    SakuraLangInterface* interface = SakuraLangInterface::getInstance();
    ErrorContainer error;

    // test addTree
    TEST_EQUAL(interface->addTree("test-tree", getTestTree(), error), true);
    std::cout<<"errorMessage: "<<error.toString()<<std::endl;

    TEST_EQUAL(interface->addTree("test-tree", getTestTree(), error), false);

    // test addTemplate
    TEST_EQUAL(interface->addTemplate("test-template", getTestTemplate()), true);
    TEST_EQUAL(interface->addTemplate("test-template", getTestTemplate()), false);

    // test getTemplate
    TEST_EQUAL(interface->getTemplate("test-template"), getTestTemplate());
    TEST_EQUAL(interface->getTemplate("fail"), "");

    // test addFile
    TEST_EQUAL(interface->addFile("test-file", getTestFile()), true);
    TEST_EQUAL(interface->addFile("test-file", getTestFile()), false);

    bool isNullptr = false;
    DataBuffer* returnBuffer = interface->getFile("test-file");
    isNullptr = returnBuffer == nullptr;
    TEST_EQUAL(isNullptr, false);

    if(isNullptr) {
        return;
    }

    TEST_EQUAL(returnBuffer->usedBufferSize, getTestFile()->usedBufferSize);
}

/**
 * @brief runAndTriggerTree_test
 */
void
Interface_Test::runAndTriggerTree_test()
{
    ErrorContainer error;
    DataMap inputValues;
    inputValues.insert("input", new DataValue(42));
    inputValues.insert("test_output", new DataValue(""));
    SakuraLangInterface* interface = SakuraLangInterface::getInstance();
    DataMap context;
    context.insert("test-key", new DataValue("asdf"));

    //----------------------------------------------------------------------------------------------
    // positiv test
    DataMap result;
    BlossomStatus status;
    TEST_EQUAL(interface->triggerTree(result,
                                      "test-tree",
                                      context,
                                      inputValues,
                                      status,
                                      error), true);
    TEST_EQUAL(result.size(), 1);
    TEST_EQUAL(status.statusCode, 0);
    TEST_EQUAL(status.errorMessage, "");

    if(result.size() == 0) {
        return;
    }
    TEST_EQUAL(result.get("test_output")->toValue()->getInt(), 42);

    //----------------------------------------------------------------------------------------------
    // test non-existing tree
    TEST_EQUAL(interface->triggerTree(result,
                                      "fail",
                                      context,
                                      inputValues,
                                      status,
                                      error), false);
    TEST_EQUAL(status.statusCode, 0);
    TEST_EQUAL(status.errorMessage, "");

    //----------------------------------------------------------------------------------------------
    // test invalid input-values
    DataMap falseMap;
    TEST_EQUAL(interface->triggerTree(result,
                                      "test-tree",
                                      context,
                                      falseMap,
                                      status,
                                      error), false);
    TEST_EQUAL(status.statusCode, 0);
    TEST_EQUAL(status.errorMessage, "");

    //----------------------------------------------------------------------------------------------
    // test invalid input-type
    DataMap brokenInput = inputValues;
    // correct type would be int
    brokenInput.insert("input", new DataValue(true), true);
    TEST_EQUAL(interface->triggerTree(result,
                                      "test-tree",
                                      context,
                                      brokenInput,
                                      status,
                                      error), false);
    TEST_EQUAL(status.statusCode, 0);
    TEST_EQUAL(status.errorMessage, "");

    //----------------------------------------------------------------------------------------------
    // test fail within blossom
    error._errorMessages.clear();
    inputValues.insert("should_fail", new DataValue(true));
    TEST_EQUAL(interface->triggerTree(result,
                                      "test-tree",
                                      context,
                                      inputValues,
                                      status,
                                      error), false);
    TEST_EQUAL(status.statusCode, 1337);
    const std::string expectedStatus =
            "+---------------------+---------------------+\n"
            "| Error-Message Nr. 0 | successfully failed |\n"
            "+---------------------+---------------------+\n";
    TEST_EQUAL(status.errorMessage, expectedStatus);

    const std::string expectedError =
            "+---------------------+-----------------------------------------+\n"
            "| Error-Message Nr. 1 | Error in location:                      |\n"
            "|                     | +-------------------+-----------------+ |\n"
            "|                     | | location          | blossom execute | |\n"
            "|                     | +-------------------+-----------------+ |\n"
            "|                     | | blossom-file-path | /test-tree      | |\n"
            "|                     | +-------------------+-----------------+ |\n"
            "+---------------------+-----------------------------------------+\n"
            "| Error-Message Nr. 0 | successfully failed                     |\n"
            "+---------------------+-----------------------------------------+\n";

    TEST_EQUAL(error.toString(), expectedError);
}

/**
 * @brief runAndTriggerBlossom_test
 */
void
Interface_Test::runAndTriggerBlossom_test()
{
    ErrorContainer error;
    DataMap inputValues;
    inputValues.insert("input", new DataValue(42));
    inputValues.insert("output", new DataValue(""));
    SakuraLangInterface* interface = SakuraLangInterface::getInstance();
    DataMap context;
    context.insert("test-key", new DataValue("asdf"));

    //----------------------------------------------------------------------------------------------
    // positiv test
    DataMap result;
    BlossomStatus status;
    TEST_EQUAL(interface->triggerBlossom(result,
                                         "standalone",
                                         "-",
                                         context,
                                         inputValues,
                                         status,
                                         error), true);
    TEST_EQUAL(status.statusCode, 0);
    TEST_EQUAL(status.errorMessage, "");

    //----------------------------------------------------------------------------------------------
    // test non-existing blossom
    TEST_EQUAL(result.get("output")->toValue()->getInt(), 42);
    TEST_EQUAL(interface->triggerBlossom(result,
                                         "fail",
                                         "-",
                                         context,
                                         inputValues,
                                         status,
                                         error), false);
    TEST_EQUAL(status.statusCode, 0);
    TEST_EQUAL(status.errorMessage, "");

    //----------------------------------------------------------------------------------------------
    // test invalid input-values
    DataMap falseMap;
    TEST_EQUAL(interface->triggerBlossom(result,
                                         "standalone",
                                         "-",
                                         context,
                                         falseMap,
                                         status,
                                         error), false);
    TEST_EQUAL(status.statusCode, 0);
    TEST_EQUAL(status.errorMessage, "");

    //----------------------------------------------------------------------------------------------
    // test invalid input-type
    DataMap brokenInput = inputValues;
    // correct type would be int
    brokenInput.insert("input", new DataValue(true), true);
    TEST_EQUAL(interface->triggerBlossom(result,
                                         "standalone",
                                         "-",
                                         context,
                                         brokenInput,
                                         status,
                                         error), false);
    TEST_EQUAL(status.statusCode, 0);
    TEST_EQUAL(status.errorMessage, "");

    //----------------------------------------------------------------------------------------------
    // test fail within blossom
    error._errorMessages.clear();
    inputValues.insert("should_fail", new DataValue(true));
    TEST_EQUAL(interface->triggerBlossom(result,
                                         "standalone",
                                         "-",
                                         context,
                                         inputValues,
                                         status,
                                         error), false);
    TEST_EQUAL(status.statusCode, 1337);
    const std::string expectedStatus =
            "+---------------------+---------------------+\n"
            "| Error-Message Nr. 0 | successfully failed |\n"
            "+---------------------+---------------------+\n";
    TEST_EQUAL(status.errorMessage, expectedStatus);

    const std::string expectedError =
            "+---------------------+------------------------------------------+\n"
            "| Error-Message Nr. 2 | trigger blossom failed.                  |\n"
            "+---------------------+------------------------------------------+\n"
            "| Error-Message Nr. 1 | Error in location:                       |\n"
            "|                     | +--------------------+-----------------+ |\n"
            "|                     | | location           | blossom execute | |\n"
            "|                     | +--------------------+-----------------+ |\n"
            "|                     | | blossom-group-type | -               | |\n"
            "|                     | +--------------------+-----------------+ |\n"
            "|                     | | blossom-name       | standalone      | |\n"
            "|                     | +--------------------+-----------------+ |\n"
            "|                     | | blossom-file-path  | standalone      | |\n"
            "|                     | +--------------------+-----------------+ |\n"
            "+---------------------+------------------------------------------+\n"
            "| Error-Message Nr. 0 | successfully failed                      |\n"
            "+---------------------+------------------------------------------+\n";
    TEST_EQUAL(error.toString(), expectedError);
}

/**
 * @brief Session_Test::getTestTree
 * @return
 */
const std::string
Interface_Test::getTestTree()
{
    const std::string tree = "[\"test\"]\n"
                             "(\"tree-comment\")\n"
                             "\n"
                             "- input = ?[int]\n"
                             "  (\"test-comment1\")\n"
                             "- should_fail = false\n"
                             "  (\"test-comment2\")\n"
                             "- test_output = >> [int]\n"
                             "  (\"test-comment3\")\n"
                             "\n"
                             "test1(\"this is a test\")\n"
                             "->test2:\n"
                             "   - input = input\n"
                             "   - should_fail = should_fail\n"
                             "   - output >> test_output\n";
    return tree;
}

/**
 * @brief Interface_Test::getTestTemplate
 * @return
 */
const std::string
Interface_Test::getTestTemplate()
{
    const std::string file = "{{testa_value}}\n";
    return file;
}

/**
 * @brief Interface_Test::getTestFile
 * @return
 */
DataBuffer*
Interface_Test::getTestFile()
{
    DataBuffer* buffer = new DataBuffer();

    uint64_t value = 42;
    addObject_DataBuffer(*buffer, &value);

    return buffer;
}

} // namespace Sakura
} // namespace Kitsunemimi

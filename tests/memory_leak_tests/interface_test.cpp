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
Interface_Test::Interface_Test()
    : Kitsunemimi::MemoryLeakTestHelpter("Interface_Test")
{
    SakuraLangInterface* interface = SakuraLangInterface::getInstance();
    TestBlossom* testBlossom = new TestBlossom(this);
    StandaloneBlossom* standaloneBlossom = new StandaloneBlossom(this);
    ErrorContainer error;

    // test addBlossom
    interface->addBlossom("test1", "test2", testBlossom);
    interface->addBlossom("-", "standalone", standaloneBlossom);

    interface->addTree("test-tree", getTestTree(), error);
    interface->addTemplate("test-template", getTestTemplate());
    interface->addFile("test-file", getTestFile());

    runAndTriggerTree_test();
    runAndTriggerBlossom_test();
}

/**
 * @brief runAndTriggerTree_test
 */
void
Interface_Test::runAndTriggerTree_test()
{
    SakuraLangInterface* interface = SakuraLangInterface::getInstance();
    REINIT_TEST();

    ErrorContainer* error = new ErrorContainer();
    DataMap* input = new DataMap();
    DataMap* result = new DataMap();
    BlossomStatus* status = new BlossomStatus();
    DataMap context;

    input->insert("input", new DataValue(42));
    input->insert("output", new DataValue(""));

    //----------------------------------------------------------------------------------------------
    // positiv test
    interface->triggerTree(*result, "test-tree", context, *input, *status, *error);
    interface->triggerTree(*result, "fail", context, *input, *status, *error);
    DataMap falseMap;
    interface->triggerTree(*result, "test-tree", context, falseMap, *status, *error);
    input->insert("should_fail", new DataValue(true));
    interface->triggerTree(*result, "test-tree", context, *input, *status, *error);

    delete error;
    delete input;
    delete result;
    delete status;

    CHECK_MEMORY();
}

/**
 * @brief runAndTriggerBlossom_test
 */
void
Interface_Test::runAndTriggerBlossom_test()
{
    SakuraLangInterface* interface = SakuraLangInterface::getInstance();
    REINIT_TEST();

    ErrorContainer* error = new ErrorContainer();
    DataMap* input = new DataMap();
    DataMap* result = new DataMap();
    BlossomStatus* status = new BlossomStatus();
    DataMap context;

    input->insert("input", new DataValue(42));
    input->insert("output", new DataValue(""));


    interface->triggerBlossom(*result, "standalone", "-", context, *input, *status, *error);
    interface->triggerBlossom(*result, "fail", "-", context, *input, *status, *error);
    DataMap falseMap;
    interface->triggerBlossom(*result, "standalone", "-", context, falseMap, *status, *error);
    input->insert("should_fail", new DataValue(true));
    interface->triggerBlossom(*result, "standalone", "-", context, *input, *status, *error);

    delete error;
    delete input;
    delete result;
    delete status;

    CHECK_MEMORY();
}

/**
 * @brief Session_Test::getTestTree
 * @return
 */
const std::string
Interface_Test::getTestTree()
{
    const std::string tree = "[\"test\"]\n"
                             "- input = \"{{}}\"\n"
                             "- should_fail = false\n"
                             "- test_output = >>\n"
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

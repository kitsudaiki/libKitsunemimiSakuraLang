/**
 * @file       interface_test.h
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

#ifndef SESSION_TEST_H
#define SESSION_TEST_H

#include <iostream>


#include <libKitsunemimiCommon/test_helper/compare_test_helper.h>

namespace Kitsunemimi
{
struct DataBuffer;
namespace Sakura
{

class Interface_Test
        : public Kitsunemimi::CompareTestHelper
{
public:
    Interface_Test();

    void blossomMethods_test();
    void addAndGet_test();
    void runAndTriggerTree_test();
    void runAndTriggerBlossom_test();

    template<typename  T>
    void compare(T isValue, T shouldValue)
    {
        TEST_EQUAL(isValue, shouldValue);
    }

private:
    const std::string getTestTree();
    const std::string getTestTemplate();

    DataBuffer* getTestFile();
    const std::string getExpectedError();
};

} // namespace Sakura
} // namespace Kitsunemimi

#endif // SESSION_TEST_H

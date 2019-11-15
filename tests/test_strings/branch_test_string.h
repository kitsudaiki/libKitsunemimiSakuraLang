/**
 * @file        branch_test_string.h
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

#ifndef BRANCH_TEST_STRING_H
#define BRANCH_TEST_STRING_H

#include <iostream>

std::string testBranchString = "[install_branch]\n"
                               "- packages = {{}}\n"
                               "\n"
                               "apt (\"this is a test\")\n"
                               "-> update \n"
                               "-> present:\n"
                               "   - names = \"{{packages}}\"\n"
                               "\n"
                               "apt (apt_blossom1)\n"
                               "-> absent:\n"
                               "   - names = \"{{packages}}\"\n"
                               "\n";

#endif // BRANCH_TEST_STRING_H

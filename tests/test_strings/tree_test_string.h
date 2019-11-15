/**
 * @file        tree_test_string.h
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

#ifndef TREE_TEST_STRING_H
#define TREE_TEST_STRING_H

#include <iostream>

std::string testTreeString = "[test_tree]\n"
                             "- packages = nano\n"
                             "\n"
                             "{\n"
                             "	 seed (sakura)\n"
                             "   - address = \"127.0.0.1\"\n"
                             "   - user = testuser\n"
                             "   - port = 22\n"
                             "   - ssh_key = \"~/.ssh/test_key\"\n"
                             "   {\n"
                             "	     branch (install_branch)\n"
                             "	     - packages = \"{{packages}}\"\n"
                             "   }\n"
                             "\n"
                             "}\n\n";

#endif // TREE_TEST_STRING_H

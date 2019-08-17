/**
 *  @file    tree_test_string.h
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#ifndef TREE_TEST_STRING_H
#define TREE_TEST_STRING_H

#include <iostream>

std::string testTreeString = "[TREE: test_tree]\n"
                             "packages: nano\n"
                             "\n"
                             "{\n"
                             "	 [BRANCH: install_branch]\n"
                             "	 packages: \"{{packages}}\"\n"
                             "}\n\n";

#endif // TREE_TEST_STRING_H

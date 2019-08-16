/**
 *  @file    tree_test_string.hpp
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#ifndef TREE_TEST_STRING_HPP
#define TREE_TEST_STRING_HPP

#include <iostream>

std::string testTreeString = "[TREE: test_tree]\n"
                             "packages: nano\n"
                             "\n"
                             "{\n"
                             "	 [BRANCH: install_branch]\n"
                             "	 packages: \"{{packages}}\"\n"
                             "}\n\n";

#endif // TREE_TEST_STRING_HPP

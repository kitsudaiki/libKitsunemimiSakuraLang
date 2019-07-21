/**
 *  @file    treeTestString.h
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#ifndef TREETESTSTRING_H
#define TREETESTSTRING_H

#include <iostream>

std::string testTreeString = "[TREE: test_tree]\n"
                             "packages: nano\n"
                             "\n"
                             "{\n"
                             "	 [BRANCH: install_branch]\n"
                             "	 packages: \"{{packages}}\"\n"
                             "}\n\n";

#endif // TREETESTSTRING_H

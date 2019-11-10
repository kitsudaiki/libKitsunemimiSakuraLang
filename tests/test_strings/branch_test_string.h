/**
 *  @file    branch_test_string.h
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#ifndef BRANCH_TEST_STRING_H
#define BRANCH_TEST_STRING_H

#include <iostream>

std::string testBranchString = "[install_branch]\n"
                               "- packages: {{}}\n"
                               "\n"
                               "apt (\"this is a test\")\n"
                               "-> update \n"
                               "-> present:\n"
                               "    - names: \"{{packages}}\"\n"
                               "\n"
                               "apt (apt_blossom1)\n"
                               "-> absent:\n"
                               "    - names: \"{{packages}}\"\n"
                               "\n";

#endif // BRANCH_TEST_STRING_H

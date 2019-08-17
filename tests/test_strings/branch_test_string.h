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

std::string testBranchString = "[BRANCH: install_branch]\n"
                               "packages: {{}}\n"
                               "\n"
                               "[apt_blossomX]\n"
                               "- apt \n "
                               "- update, present:\n"
                               "    names: \"{{packages}}\"\n"
                               "\n"
                               "[apt_blossom1]\n"
                               "- apt \n "
                               "- absent:\n"
                               "    names: \"{{packages}}\"\n"
                               "\n";

#endif // BRANCH_TEST_STRING_H
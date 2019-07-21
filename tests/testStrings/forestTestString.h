/**
 *  @file    forestTestString.h
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#ifndef FORESTTESTSTRING_H
#define FORESTTESTSTRING_H

#include <iostream>

std::string testForestString = "[FOREST: test_forest]\n"
                               "\n"
                               "[test_vm]\n"
                               "address: \"127.0.0.1\"\n"
                               "user: testuser\n"
                               "port: 22\n"
                               "ssh_key: \"~/.ssh/test_key\"\n"
                               "-TREE test_tree:\n"
                               "\n";

#endif // FORESTTESTSTRING_H

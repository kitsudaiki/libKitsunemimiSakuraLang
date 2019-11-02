include(../defaults.pri)

QT -= qt core gui

CONFIG -= app_bundle
CONFIG += c++14 console


LIBS += -L../../libKitsunemimiCommon/src -lKitsunemimiCommon
LIBS += -L../../libKitsunemimiCommon/src/debug -lKitsunemimiCommon
LIBS += -L../../libKitsunemimiCommon/src/release -lKitsunemimiCommon
INCLUDEPATH += ../../libKitsunemimiCommon/include/libKitsunemimiCommon

INCLUDEPATH += $$PWD

LIBS += -L../src -lKitsunemimiSakuraParser

SOURCES += \
        main.cpp \
    sakura_parser_test.cpp

HEADERS += \
    test_strings/branch_test_string.h \
    sakura_parser_test.h \
    test_strings/tree_test_string.h \
    test_strings/forest_test_string.h


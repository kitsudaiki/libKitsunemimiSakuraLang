include(../defaults.pri)

QT -= qt core gui

CONFIG -= app_bundle
CONFIG += c++14 console


LIBS += -L../../libKitsuneCommon/src -lKitsuneCommon
LIBS += -L../../libKitsuneCommon/src/debug -lKitsuneCommon
LIBS += -L../../libKitsuneCommon/src/release -lKitsuneCommon
INCLUDEPATH += ../../libKitsuneCommon/include/libKitsuneCommon

INCLUDEPATH += $$PWD

LIBS += -L../src -lKitsuneSakuraParser

SOURCES += \
        main.cpp \
    sakura_parser_test.cpp

HEADERS += \
    test_strings/branch_test_string.h \
    sakura_parser_test.h \
    test_strings/tree_test_string.h \
    test_strings/forest_test_string.h


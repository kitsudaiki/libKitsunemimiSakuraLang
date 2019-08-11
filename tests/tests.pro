include(../defaults.pri)

QT -= qt core gui

CONFIG -= app_bundle
CONFIG += c++14 console


LIBS += -L../../libKitsuneCommon/src -lKitsuneCommon
LIBS += -L../../libKitsuneCommon/src/debug -lKitsuneCommon
LIBS += -L../../libKitsuneCommon/src/release -lKitsuneCommon
INCLUDEPATH += ../../libKitsuneCommon/include/libKitsuneCommon

LIBS += -L../../libKitsuneData/src -lKitsuneData
LIBS += -L../../libKitsuneData/src/debug -lKitsuneData
LIBS += -L../../libKitsuneData/src/release -lKitsuneData
INCLUDEPATH += ../../libKitsuneData/include/libKitsuneData

INCLUDEPATH += $$PWD

LIBS += -L../src -lKitsuneSakuraParser

SOURCES += \
        main.cpp \
    parsingTest.cpp

HEADERS += \
    parsingTest.h \
    testStrings/treeTestString.h \
    testStrings/forestTestString.h \
    testStrings/branchTestString.h


QT -= qt core gui

TARGET = KitsunemimiSakuraLang
CONFIG += c++14
TEMPLATE = lib
VERSION = 0.7.2

LIBS += -L../../libKitsunemimiCommon/src -lKitsunemimiCommon
LIBS += -L../../libKitsunemimiCommon/src/debug -lKitsunemimiCommon
LIBS += -L../../libKitsunemimiCommon/src/release -lKitsunemimiCommon
INCLUDEPATH += ../../libKitsunemimiCommon/include

LIBS += -L../../libKitsunemimiPersistence/src -lKitsunemimiPersistence
LIBS += -L../../libKitsunemimiPersistence/src/debug -lKitsunemimiPersistence
LIBS += -L../../libKitsunemimiPersistence/src/release -lKitsunemimiPersistence
INCLUDEPATH += ../../libKitsunemimiPersistence/include

LIBS += -L../../libKitsunemimiJinja2/src -lKitsunemimiJinja2
LIBS += -L../../libKitsunemimiJinja2/src/debug -lKitsunemimiJinja2
LIBS += -L../../libKitsunemimiJinja2/src/release -lKitsunemimiJinja2
INCLUDEPATH += ../../libKitsunemimiJinja2/include

LIBS += -L../../libKitsunemimiJson/src -lKitsunemimiJson
LIBS += -L../../libKitsunemimiJson/src/debug -lKitsunemimiJson
LIBS += -L../../libKitsunemimiJson/src/release -lKitsunemimiJson
INCLUDEPATH += ../../libKitsunemimiJson/include

LIBS +=  -lboost_filesystem -lboost_system

INCLUDEPATH += $$PWD \
            $$PWD/../include


HEADERS += \
    ../include/libKitsunemimiSakuraLang/blossom.h \
    ../include/libKitsunemimiSakuraLang/sakura_lang_interface.h \
    sakura_garden.h \
    items/sakura_items.h \
    items/value_item_map.h \
    items/value_items.h \
    items/item_methods.h \
    items/value_item_functions.h \
    parsing/sakura_parser_interface.h \
    parsing/sakura_parsing.h \
    processing/sakura_thread.h \
    processing/subtree_queue.h \
    processing/thread_pool.h \
    validator.h

SOURCES += \
    items/item_methods.cpp \
    sakura_garden.cpp \
    items/sakura_items.cpp \
    items/value_item_functions.cpp \
    items/value_item_map.cpp \
    parsing/sakura_parser_interface.cpp \
    parsing/sakura_parsing.cpp \
    blossom.cpp \
    processing/sakura_thread.cpp \
    processing/subtree_queue.cpp \
    processing/thread_pool.cpp \
    validator.cpp \
    sakura_lang_interface.cpp


FLEXSOURCES = grammar/sakura_lexer.l
BISONSOURCES = grammar/sakura_parser.y

OTHER_FILES +=  \
    $$FLEXSOURCES \
    $$BISONSOURCES

# The following code calls the flex-lexer and bison-parser before compiling the
# cpp-code for automatic generation of the parser-code in each build-step.
# The resulting source-code-files are stored in the build-directory of the sakura-converter.
flexsource.input = FLEXSOURCES
flexsource.output = ${QMAKE_FILE_BASE}.cpp
flexsource.commands = flex --header-file=${QMAKE_FILE_BASE}.h -o ${QMAKE_FILE_BASE}.cpp ${QMAKE_FILE_IN}
flexsource.variable_out = SOURCES
flexsource.name = Flex Sources ${QMAKE_FILE_IN}
flexsource.CONFIG += target_predeps
flexsource.CONFIG += target_predeps

QMAKE_EXTRA_COMPILERS += flexsource

flexheader.input = FLEXSOURCES
flexheader.output = ${QMAKE_FILE_BASE}.hh
flexheader.commands = @true
flexheader.variable_out = HEADERS
flexheader.name = Flex Headers ${QMAKE_FILE_IN}
flexheader.CONFIG += target_predeps
flexheader.CONFIG += target_predeps no_link

QMAKE_EXTRA_COMPILERS += flexheader

bisonsource.input = BISONSOURCES
bisonsource.output = ${QMAKE_FILE_BASE}.cpp
bisonsource.commands = bison -d --defines=${QMAKE_FILE_BASE}.h -o ${QMAKE_FILE_BASE}.cpp ${QMAKE_FILE_IN}
bisonsource.variable_out = SOURCES
bisonsource.name = Bison Sources ${QMAKE_FILE_IN}
bisonsource.CONFIG += target_predeps
bisonsource.CONFIG += target_predeps

QMAKE_EXTRA_COMPILERS += bisonsource

bisonheader.input = BISONSOURCES
bisonheader.output = ${QMAKE_FILE_BASE}.hh
bisonheader.commands = @true
bisonheader.variable_out = HEADERS
bisonheader.name = Bison Headers ${QMAKE_FILE_IN}
bisonheader.CONFIG += target_predeps
bisonheader.CONFIG += target_predeps no_link

QMAKE_EXTRA_COMPILERS += bisonheader

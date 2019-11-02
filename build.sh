#!/bin/bash

# get current directory-path and the path of the parent-directory
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
PARENT_DIR="$(dirname "$DIR")"

# create build-directory
BUILD_DIR="$PARENT_DIR/build"
mkdir -p $BUILD_DIR

# create directory for the final result
RESULT_DIR="$PARENT_DIR/result"
mkdir -p $RESULT_DIR

#-----------------------------------------------------------------------------------------------------------------

# clone libKitsunemimiCommon
git clone  git@gitlab.com:tobiasanker/libKitsunemimiCommon.git "$PARENT_DIR/libKitsunemimiCommon"
cd "$PARENT_DIR/libKitsunemimiCommon"
git checkout v0.7.0

# create build directory for libKitsunemimiCommon and go into this directory
LIB_KITSUNE_COMMON_DIR="$BUILD_DIR/libKitsunemimiCommon"
mkdir -p $LIB_KITSUNE_COMMON_DIR
cd $LIB_KITSUNE_COMMON_DIR

# build libKitsunemimiCommon library with qmake
/usr/lib/x86_64-linux-gnu/qt5/bin/qmake "$PARENT_DIR/libKitsunemimiCommon/libKitsunemimiCommon.pro" -spec linux-g++ "CONFIG += optimize_full"
/usr/bin/make -j4

# copy build-result and include-files into the result-directory
cp "$LIB_KITSUNE_COMMON_DIR/src/libKitsunemimiCommon.so.0.7.0" "$RESULT_DIR/"
cp -r "$PARENT_DIR/libKitsunemimiCommon/include" "$RESULT_DIR/"

#-----------------------------------------------------------------------------------------------------------------

# create build directory for libKitsunemimiSakuraParser and go into this directory
LIB_KITSUNE_SAKURAPARSER_DIR="$BUILD_DIR/libKitsunemimiSakuraParser"
mkdir -p $LIB_KITSUNE_SAKURAPARSER_DIR
cd $LIB_KITSUNE_SAKURAPARSER_DIR

# build libKitsunemimiSakuraParser library with qmake
/usr/lib/x86_64-linux-gnu/qt5/bin/qmake "$PARENT_DIR/libKitsunemimiSakuraParser/libKitsunemimiSakuraParser.pro" -spec linux-g++ "CONFIG += optimize_full"
/usr/bin/make -j4

# copy build-result and include-files into the result-directory
cp "$LIB_KITSUNE_SAKURAPARSER_DIR/src/libKitsunemimiSakuraParser.so.0.1.0" "$RESULT_DIR/"
cp -r "$PARENT_DIR/libKitsunemimiSakuraParser/include" "$RESULT_DIR/"

#-----------------------------------------------------------------------------------------------------------------

# recreate symlinks
cd "$RESULT_DIR/"
ln -s libKitsunemimiCommon.so.0.7.0 libKitsunemimiCommon.so.0.7
ln -s libKitsunemimiCommon.so.0.7.0 libKitsunemimiCommon.so.0
ln -s libKitsunemimiCommon.so.0.7.0 libKitsunemimiCommon.so

ln -s libKitsunemimiSakuraParser.so.0.1.0 libKitsunemimiSakuraParser.so.0.1
ln -s libKitsunemimiSakuraParser.so.0.1.0 libKitsunemimiSakuraParser.so.0
ln -s libKitsunemimiSakuraParser.so.0.1.0 libKitsunemimiSakuraParser.so

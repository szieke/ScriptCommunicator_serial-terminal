#-------------------------------------------------
#
# Project created by QtCreator 2014-10-22T07:48:59
#
#-------------------------------------------------

#QT       -= gui

CONFIG += c++11

QT += script


INCLUDEPATH += ../
INCLUDEPATH += ../scriptClasses

TARGET = TestDll
TEMPLATE = lib

DEFINES += TESTDLL_LIBRARY

SOURCES += testdll.cpp

HEADERS += testdll.h\
        testdll_global.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}

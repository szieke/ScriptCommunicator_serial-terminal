QT += core
QT -= gui

TARGET = DeleteFolder
CONFIG -= console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp

unix{
QMAKE_RPATHDIR += lib
}



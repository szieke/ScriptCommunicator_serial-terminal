QT += core
QT -= gui

TARGET = TestProcess
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp

macx{
QMAKE_APPLE_DEVICE_ARCHS = x86_64 arm64
}


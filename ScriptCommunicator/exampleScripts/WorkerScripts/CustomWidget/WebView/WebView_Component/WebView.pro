CONFIG      += plugin debug_and_release
TARGET      = $$qtLibraryTarget(qwebviewplugin)
TEMPLATE    = lib

HEADERS     = \
    scriptwebwidget.h \
    qwebviewplugin.h
SOURCES     = qwebviewplugin.cpp \
    scriptwebwidget.cpp
RESOURCES   = icons.qrc
LIBS        += -L. 

greaterThan(QT_MAJOR_VERSION, 4) {
    QT += designer
} else {
    CONFIG += designer
}
QT += webkitwidgets
QT += printsupport

target.path = $$[QT_INSTALL_PLUGINS]/designer
INSTALLS    += target
_example2

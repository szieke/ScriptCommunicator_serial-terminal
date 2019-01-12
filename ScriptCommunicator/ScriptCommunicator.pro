greaterThan(QT_MAJOR_VERSION, 4) {
    QT       += widgets serialport
} else {
    include($$QTSERIALPORT_PROJECT_ROOT/src/serialport/qt4support/serialport.prf)
}

CONFIG += c++11

QMAKE_CXXFLAGS += -Wno-int-to-pointer-cast
QMAKE_CXXFLAGS +=-Wno-write-strings

TARGET = ScriptCommunicator
TEMPLATE = app
QT += xml
QT += script
QT += printsupport
QT += uitools
QT += network
QT += sql
QT += scripttools
QT += multimedia
win32{
QT += winextras
}

DEFINES += QUAZIP_STATIC
unix{
LIBS += -ldl
}


INCLUDEPATH += scriptClasses \
scriptClasses/scriptUiClasses \
cheetahSpi \
aardvarkI2cSpi \
pcan \
scriptClasses/canvas2D \
quazip \
ScriptEditor

RC_FILE = images/ScriptCommunicator.rc

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    settingsdialog.cpp \
    sendwindow.cpp \
    scriptClasses/scriptwindow.cpp \
    scriptClasses/qcustomplot.cpp \
    scriptClasses/plotwindow.cpp \
    scriptClasses/scriptThread.cpp \
    mainInterfaceThread.cpp \
    addmessagedialog.cpp \
    pcan/PCANBasicClass.cpp \
    canTab.cpp \
    scriptClasses/scriptSlots.cpp \
    searchconsole.cpp \
    scriptClasses/sequencetableview.cpp \
    scriptClasses/scriptsqldatabase.cpp \
    mainwindowHandleData.cpp \
    crc.cpp \
    scriptClasses/scriptUiClasses/scriptStandardDialogs.cpp \
    scriptClasses/scriptFile.cpp \
    colorWidgets/color_dialog.cpp \
    colorWidgets/color_line_edit.cpp \
    colorWidgets/color_preview.cpp \
    colorWidgets/color_wheel.cpp \
    colorWidgets/hue_slider.cpp \
    colorWidgets/gradient_slider.cpp \
    colorWidgets/color_names.cpp \
    colorWidgets/color_utils.cpp \
    colorWidgets/abstract_widget_list.cpp \
    colorWidgets/bound_color_selector.cpp \
    colorWidgets/color_2d_slider.cpp \
    colorWidgets/color_delegate.cpp \
    colorWidgets/color_list_widget.cpp \
    colorWidgets/color_palette.cpp \
    colorWidgets/color_palette_model.cpp \
    colorWidgets/color_palette_widget.cpp \
    colorWidgets/color_selector.cpp \
    colorWidgets/swatch.cpp \
    scriptClasses/scriptXml.cpp \
    scriptClasses/canvas2D/context2d.cpp \
    scriptClasses/canvas2D/qcontext2dcanvas.cpp \
    createSceFile.cpp \
    quazip/JlCompress.cpp \
    quazip/qioapi.cpp \
    quazip/quaadler32.cpp \
    quazip/quacrc32.cpp \
    quazip/quagzipfile.cpp \
    quazip/quaziodevice.cpp \
    quazip/quazip.cpp \
    quazip/quazipdir.cpp \
    quazip/quazipfile.cpp \
    quazip/quazipfileinfo.cpp \
    quazip/quazipnewinfo.cpp \
    quazip/zLib/adler32.c \
    quazip/zLib/compress.c \
    quazip/zLib/crc32.c \
    quazip/zLib/deflate.c \
    quazip/zLib/gzclose.c \
    quazip/zLib/gzlib.c \
    quazip/zLib/gzread.c \
    quazip/zLib/gzwrite.c \
    quazip/zLib/infback.c \
    quazip/zLib/inffast.c \
    quazip/zLib/inflate.c \
    quazip/zLib/inftrees.c \
    quazip/zLib/trees.c \
    quazip/zLib/uncompr.c \
    quazip/zLib/zutil.c \
    quazip/unzip.c \
    quazip/zip.c \
    scriptClasses/scriptUiClasses/scriptPlotwidget.cpp \
    scriptClasses/scriptConverter.cpp \
    aardvarkI2cSpi/aardvark.c \
    aardvarkI2cSpi/aardvarkI2cSpi.cpp \
    scriptClasses/scriptInf.cpp

HEADERS += \
    mainwindow.h \
    settingsdialog.h \
    sendwindow.h \
    scriptClasses/scriptwindow.h \
    scriptClasses/qcustomplot.h \
    scriptClasses/plotwindow.h \
    scriptClasses/scriptSerialPort.h \
    scriptClasses/scriptTcpClient.h \
    scriptClasses/scriptTcpServer.h \
    scriptClasses/scriptUdpSocket.h \
    scriptClasses/scriptThread.h \
    mainInterfaceThread.h \
    scriptClasses/scriptPlotWindow.h \
    scriptClasses/scriptUiClasses/scriptButton.h \
    scriptClasses/scriptUiClasses/scriptCheckBox.h \
    scriptClasses/scriptUiClasses/scriptComboBox.h \
    scriptClasses/scriptUiClasses/scriptDialog.h \
    scriptClasses/scriptUiClasses/scriptLabel.h \
    scriptClasses/scriptUiClasses/scriptLineEdit.h \
    scriptClasses/scriptUiClasses/scriptProgressBar.h \
    scriptClasses/scriptUiClasses/scriptTableWidget.h \
    scriptClasses/scriptUiClasses/scriptTextEdit.h \
    scriptClasses/scriptUiClasses/scriptWidget.h \
    scriptClasses/scriptUiClasses/scriptSlider.h \
    scriptClasses/scriptUiClasses/scriptMainWindow.h \
    scriptClasses/scriptUiClasses/scriptAction.h \
    scriptClasses/scriptUiClasses/scriptTabWidget.h \
    scriptClasses/scriptUiClasses/scriptGroupBox.h \
    scriptClasses/scriptUiClasses/scriptRadioButton.h \
    scriptClasses/scriptUiClasses/scriptSpinBox.h \
    addmessagedialog.h \
    scriptClasses/scriptUiClasses/scriptListWidget.h \
    scriptClasses/scriptUiClasses/scriptTreeWidget.h \
    scriptClasses/scriptUiClasses/scriptTreeWidgetItem.h \
    scriptClasses/scriptUiClasses/scriptToolButton.h \
    pcan/PCANBasic.h \
    pcan/PCANBasicClass.h \
    scriptClasses/scriptPcan.h \
    canTab.h \
    scriptClasses/scriptUiClasses/scriptSplitter.h \
    scriptClasses/scriptSlots.h \
    scriptClasses/scriptUiClasses/scriptDoubleSpinBox.h \
    scriptClasses/scriptUiClasses/scriptToolBox.h \
    scriptClasses/scriptUiClasses/scriptCalendarWidget.h \
    scriptClasses/scriptUiClasses/scriptDateTimeEdit.h \
    scriptClasses/scriptUiClasses/scriptDateEdit.h \
    scriptClasses/scriptUiClasses/scriptTimeEdit.h \
    searchconsole.h \
    scriptClasses/scriptUiClasses/scriptPlotwidget.h \
    scriptClasses/sequencetableview.h \
    scriptClasses/scriptsqldatabase.h \
    scriptClasses/scriptHelper.h \
    mainwindowHandleData.h \
    crc.h \
    scriptClasses/scriptUiClasses/scriptStandardDialogs.h \
    scriptClasses/scriptFile.h \
    colorWidgets/color_dialog.hpp \
    colorWidgets/color_line_edit.hpp \
    colorWidgets/color_list_widget.hpp \
    colorWidgets/color_preview.hpp \
    colorWidgets/color_wheel.hpp \
    colorWidgets/colorwidgets_global.hpp \
    colorWidgets/hue_slider.hpp \
    colorWidgets/gradient_slider.hpp \
    colorWidgets/color_names.hpp \
    colorWidgets/color_utils.hpp \
    colorWidgets/abstract_widget_list.hpp \
    colorWidgets/bound_color_selector.hpp \
    colorWidgets/color_2d_slider.hpp \
    colorWidgets/color_delegate.hpp \
    colorWidgets/color_palette.hpp \
    colorWidgets/color_palette_model.hpp \
    colorWidgets/color_palette_widget.hpp \
    colorWidgets/color_selector.hpp \
    colorWidgets/swatch.hpp \
    scriptClasses/scriptXml.h \
    scriptClasses/canvas2D/context2d.h \
    scriptClasses/canvas2D/qcontext2dcanvas.h \
    createSceFile.h \
    quazip/zLib/crc32.h \
    quazip/zLib/deflate.h \
    quazip/zLib/gzguts.h \
    quazip/zLib/inffast.h \
    quazip/zLib/inffixed.h \
    quazip/zLib/inflate.h \
    quazip/zLib/inftrees.h \
    quazip/zLib/trees.h \
    quazip/zLib/zconf.h \
    quazip/zLib/zlib.h \
    quazip/zLib/zutil.h \
    quazip/crypt.h \
    quazip/ioapi.h \
    quazip/JlCompress.h \
    quazip/quaadler32.h \
    quazip/quachecksum32.h \
    quazip/quacrc32.h \
    quazip/quagzipfile.h \
    quazip/quaziodevice.h \
    quazip/quazip.h \
    quazip/quazip_global.h \
    quazip/quazipdir.h \
    quazip/quazipfile.h \
    quazip/quazipfileinfo.h \
    quazip/quazipnewinfo.h \
    quazip/unzip.h \
    quazip/zip.h \
    scriptClasses/scriptObject.h \
    scriptClasses/scriptTimer.h \
    scriptClasses/scriptConverter.h \
    ScriptEditor/version.h \
    aardvarkI2cSpi/aardvark.h \
    aardvarkI2cSpi/aardvarkI2cSpi.h \
    scriptClasses/scriptAardvarkI2cSpi.h \
    scriptClasses/scriptInf.h \
    scriptClasses/scriptSound.h

FORMS += \
    mainwindow.ui \
    settingsdialog.ui \
    sendwindow.ui \
    scriptwindow.ui \
    addmessagedialog.ui \
    plotwindow.ui \
    colorWidgets/color_dialog.ui \
    colorWidgets/color_palette_widget.ui \
    createSceFile.ui

RESOURCES += \
    ScriptCommunicator.qrc

/***************************************************************************
**                                                                        **
**  ScriptCommunicator, is a tool for sending/receiving data with several **
**  interfaces.                                                           **
**  Copyright (C) 2014 Stefan Zieker                                      **
**                                                                        **
**  This program is free software: you can redistribute it and/or modify  **
**  it under the terms of the GNU General Public License as published by  **
**  the Free Software Foundation, either version 3 of the License, or     **
**  (at your option) any later version.                                   **
**                                                                        **
**  This program is distributed in the hope that it will be useful,       **
**  but WITHOUT ANY WARRANTY; without even the implied warranty of        **
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         **
**  GNU General Public License for more details.                          **
**                                                                        **
**  You should have received a copy of the GNU General Public License     **
**  along with this program.  If not, see http://www.gnu.org/licenses/.   **
**                                                                        **
****************************************************************************
**           Author: Stefan Zieker                                        **
**  Website/Contact: http://sourceforge.net/projects/scriptcommunicator/  **
****************************************************************************/

#ifndef SCRIPTWIDGET_H
#define SCRIPTWIDGET_H

#include <QObject>
#include <QWidget>
#include "scriptwindow.h"
#include "scriptThread.h"
#include "scriptObject.h"


///This wrapper class is used to access a QWidget object (located in a script gui/ui-file) from a script.
class ScriptWidget: public QObject, public ScriptObject
{
    Q_OBJECT

    ///Returns a semicolon separated list with all public functions, signals and properties.
    Q_PROPERTY(QString publicScriptElements READ getPublicScriptElements)

public:
    ScriptWidget(QWidget* widget, ScriptThread *scriptThread, ScriptWindow* scriptWindow) :
        m_scriptWindow(scriptWindow), m_scriptThread(scriptThread), m_widget(widget)
    {

        if(QThread::currentThread() == scriptThread->thread())
        {
            setParent(scriptThread);
        }
        else
        {
            setParent(widget);
        }

        if(m_widget)
        {
            Qt::ConnectionType directConnectionType = scriptThread->runsInDebugger() ? Qt::DirectConnection : Qt::BlockingQueuedConnection;

            if(!scriptThread->registerMetaTypeCalledinScriptWidget())
            {
                qRegisterMetaType<QPalette::ColorRole>("QPalette::ColorRole");
                scriptThread->setRegisterMetaTypeCalledinScriptWidget(true);
            }

            //connect the necessary signals with the wrapper slots (in this slots the
            //events of the wrapper class are generated, the script can connect to this
            //wrapper events)
            connect(this, SIGNAL(setEnabledSignal(bool)), widget, SLOT(setEnabled(bool)), Qt::QueuedConnection);
            connect(this, SIGNAL(updateSignal()), widget, SLOT(update()), Qt::QueuedConnection);
            connect(this, SIGNAL(repaintSignal()), widget, SLOT(repaint()), Qt::QueuedConnection);
            connect(this, SIGNAL(showSignal()), widget, SLOT(show()), directConnectionType);
            connect(this, SIGNAL(closeSignal()), widget, SLOT(close()), Qt::QueuedConnection);
            connect(this, SIGNAL(hideSignal()), widget, SLOT(hide()), Qt::QueuedConnection);
            connect(this, SIGNAL(setWindowTitleSignal(QString)), widget, SLOT(setWindowTitle(QString)), Qt::QueuedConnection);
            connect(this, SIGNAL(setWindowIconSignal(QString, QWidget*)), scriptWindow, SLOT(setWindowIconSlot(QString, QWidget*)), Qt::QueuedConnection);

            connect(this, SIGNAL(setWindowPositionAndSizeSignal(QString, QWidget*)),
                    scriptWindow, SLOT(setWindowPositionAndSizeSlot(QString, QWidget*)), directConnectionType);

            connect(this, SIGNAL(windowPositionAndSizeSignal(QString*,QWidget*)),
                    scriptWindow, SLOT(windowPositionAndSizeSlot(QString*,QWidget*)), directConnectionType);

            connect(this, SIGNAL(setScriptGuiElementColorSignal(QColor,QWidget*,QPalette::ColorRole)),
                    scriptWindow, SLOT(setScriptGuiElementColorSlot(QColor,QWidget*, QPalette::ColorRole)), Qt::QueuedConnection);

            connect(this, SIGNAL(setScriptGuiElementAutoFillBackgroundSignal(QWidget*,bool)),
                    scriptWindow, SLOT(setScriptGuiElementAutoFillBackgroundSlot(QWidget*,bool)), Qt::QueuedConnection);

            connect(this, SIGNAL(setScriptGuiElementBackgroundColorSignal(QColor,QWidget*)),
                    scriptWindow, SLOT(setScriptGuiElementBackgroundColorSlot(QColor,QWidget*)), Qt::QueuedConnection);

            connect(this, SIGNAL(setToolTipSignal(QString,int,QWidget*)),
                    scriptWindow, SLOT(setToolTipSlot(QString,int,QWidget*)), Qt::QueuedConnection);

            connect(this, SIGNAL(raiseSignal()),m_widget, SLOT(raise()), Qt::QueuedConnection);

            connect(this, SIGNAL(lowerSignal()), m_widget, SLOT(lower()), Qt::QueuedConnection);


            connect(this, SIGNAL(setWindowFlagsSignal(Qt::WindowFlags,QWidget*)),
                    scriptWindow, SLOT(setWindowFlagsSlot(Qt::WindowFlags,QWidget*)), directConnectionType);

            connect(this, SIGNAL(setFocusSignal()), m_widget, SLOT(setFocus()), Qt::QueuedConnection);

            connect(this, SIGNAL(createShortCutSignal(QString,QWidget*,QShortcut**)),
                    scriptWindow, SLOT(createShortCutSlot(QString,QWidget*,QShortcut**)), directConnectionType);
        }
    }
    ///Returns a semicolon separated list with all public functions, signals and properties.
    virtual QString getPublicScriptElements(void)
    {
        return MainWindow::parseApiFile("ScriptWidget.api");
    }

    ///Enables or disables the widget.
    Q_INVOKABLE void setEnabled(bool isEnabled){emit setEnabledSignal(isEnabled);}

    ///Updates the widget.
    Q_INVOKABLE void update(void){emit updateSignal();}

    ///Repaints the widget.
    Q_INVOKABLE void repaint(void){emit repaintSignal();}

    ///Shows the widget.
    Q_INVOKABLE void show(void){emit showSignal();}

    ///Closes the widget.
    Q_INVOKABLE void close(void){emit closeSignal();}

    ///Hides the widget.
    Q_INVOKABLE void hide(void){emit hideSignal();}

    ///Sets the window title.
    Q_INVOKABLE void setWindowTitle(QString title){emit setWindowTitleSignal(title);}

    ///Returns the window size and position (Pixel)
    ///The return string has following format:
    ///"top left x, top left y, width, height"
    Q_INVOKABLE QString windowPositionAndSize(void)
    {
        QString returnString;
        emit windowPositionAndSizeSignal(&returnString, m_widget);
        return returnString;
    }

    ///Sets the position and the size of a window). String format:
    ///"top left x, top left y, width, height".
    Q_INVOKABLE void setWindowPositionAndSize(QString positionAndSize){emit setWindowPositionAndSizeSignal(positionAndSize, m_widget);}

    ///Sets the background color of a script gui element.
    ///Possible colors are: black, white, gray, red, green, blue, cyan, magenta and yellow.
    Q_INVOKABLE void setBackgroundColor(QString color){emit setScriptGuiElementBackgroundColorSignal(QColor(ScriptSlots::stringToGlobalColor(color)), m_widget);}

    ///Sets the window text color of a script gui element.
    ///Possible colors are: black, white, gray, red, green, blue, cyan, magenta and yellow.
    Q_INVOKABLE void setWindowTextColor(QString color){emit setScriptGuiElementColorSignal(QColor(ScriptSlots::stringToGlobalColor(color)), m_widget,ScriptSlots::stringToPaletteColorRole("WindowText"));}

    ///Sets the text color of a script gui element.
    ///Possible colors are: black, white, gray, red, green, blue, cyan, magenta and yellow.
    Q_INVOKABLE void setTextColor(QString color){emit setScriptGuiElementColorSignal(QColor(ScriptSlots::stringToGlobalColor(color)), m_widget, ScriptSlots::stringToPaletteColorRole("Text"));}

    ///Sets a palette color of a script gui element.
    ///Possible palette values are: Base, Foreground, Background, WindowText, Window, Text and ButtonText.
    ///Possible colors are: black, white, gray, red, green, blue, cyan, magenta and yellow.
    Q_INVOKABLE void setPaletteColor(QString palette, QString color){emit setScriptGuiElementColorSignal(QColor(ScriptSlots::stringToGlobalColor(color)), m_widget, ScriptSlots::stringToPaletteColorRole(palette));}

    ///Sets a palette color of a script gui element.
    ///Possible palette values are: Base, Foreground, Background, WindowText, Window, Text and ButtonText.
    Q_INVOKABLE void setPaletteColorRgb(quint8 red, quint8 green, quint8 blue, QString palette){emit setScriptGuiElementColorSignal(QColor(red, green, blue), m_widget, ScriptSlots::stringToPaletteColorRole(palette));}

    Q_INVOKABLE void setAutoFillBackground(bool enabled){emit setScriptGuiElementAutoFillBackgroundSignal(m_widget, enabled);}


    ///Sets the tool tip of the script gui element.
    ///If the duration is -1 (default) the duration is calculated depending on the length of the tool tip.
    Q_INVOKABLE void setToolTip(QString text, int duration){emit setToolTipSignal(text, duration,m_widget);}

    ///Return true, if the widget is visible.
    Q_INVOKABLE bool isVisible(void){ return m_widget->isVisible();}

    ///Raises this widget to the top of the parent widget's stack.
    Q_INVOKABLE void raise(void){ emit raiseSignal();}

    ///Lowers the widget to the bottom of the parent widget's stack.
    Q_INVOKABLE void lower(void){ emit lowerSignal();}

    ///Sets the window flags.
    //////Note: ScriptWidget::show must be called after a setWindowFlags call.
    Q_INVOKABLE void setWindowFlags(quint32 flags){emit setWindowFlagsSignal((Qt::WindowFlags) (m_widget->windowFlags() | flags), m_widget);}

    ///Clears the given window flags.
    ///Note: ScriptWidget::show must be called after a clearWindowFlags call.
    Q_INVOKABLE void clearWindowFlags(quint32 flags){emit setWindowFlagsSignal(m_widget->windowFlags() & (~flags), m_widget);}

    ///Returns the window flags.
    Q_INVOKABLE quint32 windowFlags(void){return (quint32)m_widget->windowFlags();}

    ///Gives the keyboard input focus to this widget.
    Q_INVOKABLE void setFocus(void){emit setFocusSignal();}

    ///Returns the width of the widget excluding any window frame.
    Q_INVOKABLE int width(void){return m_widget->width();}

    ///Returns the height of the widget excluding any window frame.
    Q_INVOKABLE int height(void){return m_widget->height();}

    ///Returns the widget pointer.
    Q_INVOKABLE QWidget* getWidgetPointer(void){return m_widget;}

    ///Sets/stores an additional data entry.
    Q_INVOKABLE void setAdditionalData(int key, QString data){m_additionalData[key] = data;}

    ///Returns an additional data entry.
    Q_INVOKABLE QString getAdditionalData(int key)
    {
        QString data;
        if(m_additionalData.contains(key))
        {
            data = m_additionalData[key];
        }
        return data;
    }


    ///If block is true, signals emitted by this object are blocked (i.e., emitting a signal will not invoke anything connected to it).
    ///If block is false, no such blocking will occur.
    ///The return value is the previous value of the blocking state.
    Q_INVOKABLE bool blockSignals(bool block){return m_widget->blockSignals(block);}

    ///Returns the class name of this object.
    Q_INVOKABLE QString getClassName(void){return metaObject()->className();}

    ///Returns the name of this object (UI_'object name in the ui file').
    ///Note: This function returns only a not empty string for GUI elements from ui files (standard GUI elements).
    ///For GUI elements created with ScriptTableWidget::insertWidget the object name must be set with setObjectName.
    Q_INVOKABLE QString getObjectName(void){return QObject::objectName();}

    ///Sets the name of the current object (can be retrieved with getObjectName).
    Q_INVOKABLE void setObjectName(QString name){QObject::setObjectName(name);}

    ///Sets the window icon of a dialog or a main window.
    ///Supported formats: .ico, .gif, .png, .jpeg, .tiff, .bmp, .icns.
    Q_INVOKABLE void setWindowIcon(QString iconFile, bool isRelativePath=true)
    {
        iconFile = isRelativePath ? m_scriptThread->createAbsolutePath(iconFile) : iconFile;
        emit setWindowIconSignal(iconFile, m_widget);
    }

    ///Creates a shortcut and connects it to a script function.
    Q_INVOKABLE void createShortCut(QString keys, QScriptValue scriptFunction)
    {
        QShortcut* shortCut;
        emit createShortCutSignal(keys, m_widget, &shortCut);
        if (!qScriptConnect(shortCut, SIGNAL(activated()), QScriptValue(), scriptFunction))
        {
            m_scriptThread->getScriptEngine()->currentContext()->throwError("could not find function: " + scriptFunction.toString());

        }
    }

Q_SIGNALS:

    ///Is emitted by the setWindowFlag function,
    ///This signal is private and must not be used inside a script.
    void setFocusSignal(void);

    ///Is emitted by the setWindowFlag function,
    ///This signal is private and must not be used inside a script.
    void setWindowFlagsSignal(Qt::WindowFlags flags, QWidget* element);

    ///Is emitted by the raise function,
    ///This signal is private and must not be used inside a script.
    void raiseSignal(void);

    ///Is emitted by the lower function,
    ///This signal is private and must not be used inside a script.
    void lowerSignal(void);

    ///Is emitted by the setToolTip function,
    ///This signal is private and must not be used inside a script.
    void setToolTipSignal(QString text, int duration, QWidget* element);


    ///Is connected with ScriptWindow::windowPositionAndSizeSlot (sets the position and the size of a window).
    ///This signal is private and must not be used inside a script.
    void windowPositionAndSizeSignal(QString* string, QWidget* widget);

    ///Is connected with ScriptWindow::setWindowPositionAndSizeSlot (sets the position and the size of a window).
    ///This signal is private and must not be used inside a script.
    void setWindowPositionAndSizeSignal(QString positionAndSize, QWidget* widget);

    ///This signal is emitted if the setEnabled function is called.
    ///This signal is private and must not be used inside a script.
    void setEnabledSignal(bool);

    ///This signal is emitted if the update function is called.
    ///This signal is private and must not be used inside a script.
    void updateSignal();

    ///This signal is emitted if the repaint function is called.
    ///This signal is private and must not be used inside a script.
    void repaintSignal();

    ///This signal is emitted if the show function is called.
    ///This signal is private and must not be used inside a script.
    void showSignal();

    ///This signal is emitted if the close function is called.
    ///This signal is private and must not be used inside a script.
    void closeSignal();

    ///This signal is emitted if the hide function is called.
    ///This signal is private and must not be used inside a script.
    void hideSignal();

    ///This signal is emitted if the setWindowTitle function is called.
    ///This signal is private and must not be used inside a script.
    void setWindowTitleSignal(QString);

    ///This signal is emitted if the background color of a script gui element
    ///shall be changed.
    ///This signal is private and must not be used inside a script.
    void setScriptGuiElementBackgroundColorSignal(QColor color, QWidget* element);

    ///This signal is emitted if the color role color of a script gui element
    ///shall be changed.
    ///This signal is private and must not be used inside a script.
    void setScriptGuiElementColorSignal(QColor color, QWidget* element, QPalette::ColorRole colorRole);

    ///This signal is emitted if the auto fill background property of a script gui element
    ///shall be changed.
    ///This signal is private and must not be used inside a script.
    void setScriptGuiElementAutoFillBackgroundSignal(QWidget* element, bool enabled);

    ///This signal is emitted in setWindowIcon.
    ///This signal is private and must not be used inside a script.
    void setWindowIconSignal(QString iconFile, QWidget* widget);

    ///This signal is emitted in createShortCut.
    ///This signal is private and must not be used inside a script.
    void createShortCutSignal(QString key, QWidget* parent, QShortcut** shortCut);

protected:
    ///Script window pointer.
    ScriptWindow* m_scriptWindow;

    ///Pointer to the script thread;
    ScriptThread* m_scriptThread;
private:
    ///The wrapped widget.
    QWidget* m_widget;

    ///Map for storing additional data.
    QMap<int, QString> m_additionalData;

};

#endif // SCRIPTWIDGET_H

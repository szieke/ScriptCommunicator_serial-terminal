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

#ifndef SCRIPTTABEDWIDGET_H
#define SCRIPTTABEDWIDGET_H

#include <QObject>

#include "scriptWidget.h"

///This wrapper class is used to access a QTabWidget object (located in a script gui/ui-file) from a script.
class ScriptTabWidget : public ScriptWidget
{
    Q_OBJECT
public:
    explicit ScriptTabWidget(QTabWidget* tabWidget, ScriptThread *scriptThread) :
        ScriptWidget(tabWidget, scriptThread, scriptThread->getScriptWindow()), m_tabWidget(tabWidget)
    {
        //connect the necessary signals with the wrapper slots (in this slots the
        //events of the wrapper class are generated, the script can connect to this
        //wrapper events)
        Qt::ConnectionType directConnectionType = scriptThread->runsInDebugger() ? Qt::DirectConnection : Qt::BlockingQueuedConnection;

        connect(this, SIGNAL(setTabWidgetTabTextSignal(int,QString,QTabWidget*)), scriptThread->getScriptWindow(),
                SLOT(setTabWidgetTabTextSlot(int,QString,QTabWidget*)), directConnectionType);

        connect(this, SIGNAL(setCurrentIndexSignal(int,QTabWidget*)), scriptThread->getScriptWindow(),
                SLOT(setCurrentIndexSlot(int, QTabWidget*)), directConnectionType);

        connect(m_tabWidget, SIGNAL(currentChanged(int)), this, SIGNAL(currentTabChangedSignal(int)), Qt::QueuedConnection);


    }

    ///Returns a semicolon separated list with all public functions, signals and properties.
    virtual QString getPublicScriptElements(void)
    {
        return MainWindow::parseApiFile("ScriptTabWidget.api");
    }

    ///Sets the tab text.
    Q_INVOKABLE void setTabText(int index, QString text){emit setTabWidgetTabTextSignal(index, text, m_tabWidget);}

    ///Returns the tab text.
    Q_INVOKABLE QString tabText(int index){return m_tabWidget->tabText(index);}

    ///Sets the current tab index.
    Q_INVOKABLE void setCurrentIndex(int index){emit setCurrentIndexSignal(index, m_tabWidget);}

    ///Returns the current tab index.
    Q_INVOKABLE int currentIndex(void){return m_tabWidget->currentIndex();}



Q_SIGNALS:

    ///This signal is emitted if the current tab has been changed.
    ///Scripts can connect a function to this signal.
    void currentTabChangedSignal(int index);

    ///This signal is emitted in setTabWidgetTabText.
    ///This signal is private and must not be used inside a script.
    void setTabWidgetTabTextSignal(int index, QString text, QTabWidget* tabWidget);

    ///This signal is emitted in setCurrentIndex.
    ///This signal is private and must not be used inside a script.
    void setCurrentIndexSignal(int index, QTabWidget* tabWidget);

private:
    ///The wrapped push button.
    QTabWidget* m_tabWidget;
};

#endif // SCRIPTTABEDWIDGET_H

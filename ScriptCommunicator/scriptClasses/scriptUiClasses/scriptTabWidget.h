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
        ScriptWidget(tabWidget, scriptThread, scriptThread->getScriptWindow()), m_tabWidget(tabWidget),
        m_storedTabs(), m_storedTabIdCounter(0)
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

        connect(this, SIGNAL(removeTabSignal(QTabWidget*,int,QWidget**, QString*)), scriptThread->getScriptWindow(),
                SLOT(removeTabSlot(QTabWidget*,int,QWidget**, QString*)), directConnectionType);

        connect(this, SIGNAL(insertTabSignal(QTabWidget*,QWidget*,QString,int)), scriptThread->getScriptWindow(),
                SLOT(insertTabSlot(QTabWidget*,QWidget*,QString,int)), directConnectionType);

    }

    typedef struct
    {
        QWidget* tabPointer;
        QString tabText;
    }Tab;

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

    /**
     * Removes a tab and returns the tab id (can be used in insertTab).
     * @param index The index of the tab.
     * @return The tab id.
     */
    Q_INVOKABLE int removeTab(int index)
    {
        Tab tab;
        emit removeTabSignal(m_tabWidget, index, &tab.tabPointer, &tab.tabText);

        m_storedTabs[m_storedTabIdCounter] = tab;
        m_storedTabIdCounter++;
        return m_storedTabIdCounter - 1;
    }

    /**
     * Inserts a tab that was removed with removeTab.
     * @param tabId The tab id.
     * @param index The index at wich the tab shall be inserted.
     */
    Q_INVOKABLE void insertTab(int tabId, int index)
    {
       if(m_storedTabs.contains(tabId))
       {
           emit insertTabSignal(m_tabWidget, m_storedTabs[tabId].tabPointer, m_storedTabs[tabId].tabText, index);
           m_storedTabs.remove(tabId);
       }
    }



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

    ///This signal is emitted in removeTab.
    ///This signal is private and must not be used inside a script.
    void removeTabSignal(QTabWidget* tabWidget, int index, QWidget** tab, QString* tabText);

    ///This signal is emitted in addTab.
    ///This signal is private and must not be used inside a script.
    void insertTabSignal(QTabWidget* tabWidget, QWidget* tab, QString tabText, int index);

private:
    ///The wrapped push button.
    QTabWidget* m_tabWidget;

    QMap<int, Tab> m_storedTabs;
    int m_storedTabIdCounter;
};

#endif // SCRIPTTABEDWIDGET_H

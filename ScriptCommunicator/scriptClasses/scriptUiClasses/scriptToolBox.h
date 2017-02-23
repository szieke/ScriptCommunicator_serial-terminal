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

#ifndef SCRIPTTOOLBOXWIDGET_H
#define SCRIPTTOOLBOXWIDGET_H

#include <QObject>

#include "scriptWidget.h"

///This wrapper class is used to access a QToolBox object (located in a script gui/ui-file) from a script.
class ScriptToolBox : public ScriptWidget
{
    Q_OBJECT
public:
    explicit ScriptToolBox(QToolBox* box, ScriptThread *scriptThread) :
        ScriptWidget(box, scriptThread, scriptThread->getScriptWindow()), m_box(box)
    {
        //connect the necessary signals with the wrapper slots (in this slots the
        //events of the wrapper class are generated, the script can connect to this
        //wrapper events)
        Qt::ConnectionType directConnectionType = scriptThread->runsInDebugger() ? Qt::DirectConnection : Qt::BlockingQueuedConnection;

        connect(this, SIGNAL(setItemTextSignal(int,QString,QToolBox*)), scriptThread->getScriptWindow(),
                SLOT(setItemTextSlot(int,QString,QToolBox*)), directConnectionType);

        connect(this, SIGNAL(setCurrentIndexSignal(int,QToolBox*)), scriptThread->getScriptWindow(),
                SLOT(setCurrentIndex(int,QToolBox*)), directConnectionType);

        connect(m_box, SIGNAL(currentChanged(int)), this, SIGNAL(currentItemChangedSignal(int)), Qt::QueuedConnection);


    }

    ///Returns a semicolon separated list with all public functions, signals and properties.
    virtual QString getPublicScriptElements(void)
    {
        return MainWindow::parseApiFile("ScriptToolBox.api");
    }

    ///Sets the item text.
    Q_INVOKABLE void setItemText(int index, QString text){emit setItemTextSignal(index, text, m_box);}

    ///Returns the item text.
    Q_INVOKABLE QString itemText(int index){return m_box->itemText(index);}

    ///Sets the current item index.
    Q_INVOKABLE void setCurrentIndex(int index){emit setCurrentIndexSignal(index, m_box);}

    ///Returns the current item index.
    Q_INVOKABLE int currentIndex(void){return m_box->currentIndex();}



Q_SIGNALS:

    ///This signal is emitted if the current item has been changed.
    ///Scripts can connect a function to this signal.
    void currentItemChangedSignal(int index);

    ///This signal is emitted in setItemText.
    ///This signal is private and must not be used inside a script.
    void setItemTextSignal(int index, QString text, QToolBox* box);

    ///This signal is emitted in setCurrentIndex.
    ///This signal is private and must not be used inside a script.
    void setCurrentIndexSignal(int index, QToolBox* box);

private:
    ///The wrapped tool box.
    QToolBox* m_box;
};

#endif // SCRIPTTOOLBOXWIDGET_H

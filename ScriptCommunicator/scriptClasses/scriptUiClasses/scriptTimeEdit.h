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

#ifndef SCRIPTTIMEEDIT_H
#define SCRIPTTIMEEDIT_H

#include <QObject>
#include "scriptWidget.h"

///This wrapper class is used to access a QTimeEdit object (located in a script gui/ui-file) from a script.
class ScriptTimeEdit : public ScriptWidget
{
    Q_OBJECT
public:
    explicit ScriptTimeEdit(QTimeEdit* timeEdit, ScriptThread *scriptThread) :
        ScriptWidget(timeEdit,scriptThread, scriptThread->getScriptWindow()), m_timeEdit(timeEdit)
    {
        //connect the necessary signals with the wrapper slots (in this slots the
        //events of the wrapper class are generated, the script can connect to this
        //wrapper events)
        Qt::ConnectionType directConnectionType = scriptThread->runsInDebugger() ? Qt::DirectConnection : Qt::BlockingQueuedConnection;

        connect(this, SIGNAL(setTimeSignal(QTime)), m_timeEdit, SLOT(setTime(QTime)), directConnectionType);
        connect(m_timeEdit, SIGNAL(userTimeChanged(QTime)), this, SLOT(stub_timeChanged(QTime)));
        connect(this, SIGNAL(setDisplayFormat(QString,QDateTimeEdit*)), scriptThread->getScriptWindow(),
                SLOT(setDisplayFormat(QString,QDateTimeEdit*)), directConnectionType);

    }

    ///Returns a semicolon separated list with all public functions, signals and properties.
    virtual QString getPublicScriptElements(void)
    {
        return MainWindow::parseApiFile("ScriptTimeEdit.api");
    }

    ///Sets the time.
    Q_INVOKABLE void setTime(QString timeString)
    {
        QTime time = QTime::fromString(timeString, m_timeEdit->displayFormat());
        emit setTimeSignal(time);
    }

    ///Returns the time.
    Q_INVOKABLE QString getTime(void){ return m_timeEdit->time().toString(m_timeEdit->displayFormat());}

    ///Sets the display format.
    Q_INVOKABLE void setDisplayFormat(QString format){emit setDisplayFormat(format, m_timeEdit);}

    ///Returns the display format.
    Q_INVOKABLE QString getDisplayFormat(void){return m_timeEdit->displayFormat();}



Q_SIGNALS:
    ///This signal is emitted if the value of the time edit has been changed.
    ///Scripts can connect a function to this signal.
    void timeChangedSignal(QString time);

    ///Is emitted in setDisplayFormat.
    ///This signal is private and must not be used inside a script.
    void setDisplayFormat(QString time,QDateTimeEdit* element);

    ///Is emitted in setTime.
    ///This signal is private and must not be used inside a script.
    void setTimeSignal(QTime time);

private slots:

    ///This slot function is called if the user changes the time.
    ///In this slot function the timeChanged signal is generated
    void stub_timeChanged(const QTime &time)
    {
        emit timeChangedSignal(time.toString(m_timeEdit->displayFormat()));
    }
private:
    ///The wrapped check box.
    QTimeEdit* m_timeEdit;

};

#endif // SCRIPTTIMEEDIT_H

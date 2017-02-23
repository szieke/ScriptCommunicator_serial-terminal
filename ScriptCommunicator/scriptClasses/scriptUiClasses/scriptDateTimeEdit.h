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

#ifndef SCRIPTDATETIMEEDIT_H
#define SCRIPTDATETIMEEDIT_H

#include <QObject>
#include "scriptWidget.h"

///This wrapper class is used to access a QDateTimeEdit object (located in a script gui/ui-file) from a script.
class ScriptDateTimeEdit : public ScriptWidget
{
    Q_OBJECT
public:
    explicit ScriptDateTimeEdit(QDateTimeEdit* dateTimeEdit, ScriptThread *scriptThread) :
        ScriptWidget(dateTimeEdit,scriptThread, scriptThread->getScriptWindow()), m_dateTimeEdit(dateTimeEdit)
    {
        //connect the necessary signals with the wrapper slots (in this slots the
        //events of the wrapper class are generated, the script can connect to this
        //wrapper events)

        Qt::ConnectionType directConnectionType = scriptThread->runsInDebugger() ? Qt::DirectConnection : Qt::BlockingQueuedConnection;

        connect(this, SIGNAL(setDateTimeSignal(QDateTime)), m_dateTimeEdit, SLOT(setDateTime(QDateTime)), directConnectionType);
        connect(m_dateTimeEdit, SIGNAL(dateTimeChanged(QDateTime)), this, SLOT(stub_dateTimeChanged(QDateTime)));

        connect(this, SIGNAL(setDisplayFormat(QString,QDateTimeEdit*)), scriptThread->getScriptWindow(),
                SLOT(setDisplayFormat(QString,QDateTimeEdit*)), directConnectionType);


    }

    ///Returns a semicolon separated list with all public functions, signals and properties.
    virtual QString getPublicScriptElements(void)
    {
        return MainWindow::parseApiFile("ScriptDateTimeEdit.api");
    }

    ///Sets the date and the time.
    Q_INVOKABLE void setDateTime(QString dateTimeString)
    {
        QDateTime dateTime = QDateTime::fromString(dateTimeString, m_dateTimeEdit->displayFormat());
        emit setDateTimeSignal(dateTime);
    }

    ///Returns the date and the time.
    Q_INVOKABLE QString getDateTime(void){ return m_dateTimeEdit->dateTime().toString(m_dateTimeEdit->displayFormat());}



    ///Sets the display format.
    Q_INVOKABLE void setDisplayFormat(QString format){emit setDisplayFormat(format, m_dateTimeEdit);}

    ///Returns the display format.
    Q_INVOKABLE QString getDisplayFormat(void){return m_dateTimeEdit->displayFormat();}



Q_SIGNALS:
    ///This signal is emitted if the value of the date edit has been changed.
    ///Scripts can connect a function to this signal.
    void dateTimeChangedSignal(QString date);

    ///Is emitted in setDisplayFormat.
    ///This signal is private and must not be used inside a script.
    void setDisplayFormat(QString,QDateTimeEdit*);

    ///Is emitted in setDateTime.
    ///This signal is private and must not be used inside a script.
    void setDateTimeSignal(QDateTime dateTime);


private slots:

    ///This slot function is called if the user changes the date.
    ///In this slot function the timeChanged signal is generated
    void stub_dateTimeChanged(QDateTime dateTime)
    {
        emit dateTimeChangedSignal(dateTime.toString(m_dateTimeEdit->displayFormat()));
    }

private:
    ///The wrapped check box.
    QDateTimeEdit* m_dateTimeEdit;

};

#endif // SCRIPTDATETIMEEDIT_H

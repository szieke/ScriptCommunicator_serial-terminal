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

#ifndef SCRIPTDATEEDIT_H
#define SCRIPTDATEEDIT_H

#include <QObject>
#include "scriptWidget.h"

///This wrapper class is used to access a QDateEdit object (located in a script gui/ui-file) from a script.
class ScriptDateEdit : public ScriptWidget
{
    Q_OBJECT
public:
    explicit ScriptDateEdit(QDateEdit* dateEdit, ScriptThread *scriptThread) :
        ScriptWidget(dateEdit,scriptThread, scriptThread->getScriptWindow()), m_dateEdit(dateEdit)
    {
        //connect the necessary signals with the wrapper slots (in this slots the
        //events of the wrapper class are generated, the script can connect to this
        //wrapper events)
        Qt::ConnectionType directConnectionType = scriptThread->runsInDebugger() ? Qt::DirectConnection : Qt::BlockingQueuedConnection;

        connect(this, SIGNAL(setDateSignal(QDate)), m_dateEdit, SLOT(setDate(QDate)), directConnectionType);
        connect(m_dateEdit, SIGNAL(dateChanged(QDate)), this, SLOT(stub_dateChanged(QDate)));
        connect(this, SIGNAL(setDisplayFormat(QString,QDateTimeEdit*)), scriptThread->getScriptWindow(),
                SLOT(setDisplayFormat(QString,QDateTimeEdit*)), directConnectionType);

    }

    ///Returns a semicolon separated list with all public functions, signals and properties.
    virtual QString getPublicScriptElements(void)
    {
        return MainWindow::parseApiFile("ScriptDateEdit.api");
    }

    ///Sets the date.
    Q_INVOKABLE void setDate(QString dateString)
    {
        QDate date = QDate::fromString(dateString, m_dateEdit->displayFormat());
        emit setDateSignal(date);
    }

    ///Returns the date.
    Q_INVOKABLE QString getDate(void){ return m_dateEdit->date().toString(m_dateEdit->displayFormat());}

    ///Sets the display format.
    Q_INVOKABLE void setDisplayFormat(QString format){emit setDisplayFormat(format, m_dateEdit);}

    ///Returns the display format.
    Q_INVOKABLE QString getDisplayFormat(void){return m_dateEdit->displayFormat();}



Q_SIGNALS:
    ///This signal is emitted if the value of the date edit has been changed.
    ///Scripts can connect a function to this signal.
    void dateChangedSignal(QString date);

    ///Is emitted in setDisplayFormat.
    ///This signal is private and must not be used inside a script.
    void setDisplayFormat(QString,QDateTimeEdit*);

    ///Is emitted in setDate.
    ///This signal is private and must not be used inside a script.
    void setDateSignal(QDate date);

private slots:

    ///This slot function is called if the user changes the date.
    ///In this slot function the timeChanged signal is generated
    void stub_dateChanged(QDate date)
    {
        emit dateChangedSignal(date.toString(m_dateEdit->displayFormat()));
    }
private:
    ///The wrapped check box.
    QDateEdit* m_dateEdit;

};

#endif // SCRIPTDATEEDIT_H

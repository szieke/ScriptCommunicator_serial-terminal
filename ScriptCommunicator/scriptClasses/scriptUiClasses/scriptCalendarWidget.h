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

#ifndef SCRIPTCALENDARWIDGET_H
#define SCRIPTCALENDARWIDGET_H

#include <QObject>
#include "scriptWidget.h"
#include <QCalendarWidget>

///This wrapper class is used to access a QCalendarWidget object (located in a script gui/ui-file) from a script.
class ScriptCalendarWidget : public ScriptWidget
{
    Q_OBJECT
public:
    explicit ScriptCalendarWidget(QCalendarWidget* calendar, ScriptThread *scriptThread) :
        ScriptWidget(calendar,scriptThread, scriptThread->getScriptWindow()), m_calendar(calendar)
    {
        //connect the necessary signals with the wrapper slots (in this slots the
        //events of the wrapper class are generated, the script can connect to this
        //wrapper events)

        Qt::ConnectionType directConnectionType = scriptThread->runsInDebugger() ? Qt::DirectConnection : Qt::BlockingQueuedConnection;

        connect(this, SIGNAL(setSelectedDateSignal(QDate)), m_calendar, SLOT(setSelectedDate(QDate)), directConnectionType);
        connect(this, SIGNAL(setDateRangeSignal(QDate,QDate)), m_calendar, SLOT(setDateRange(QDate,QDate)), directConnectionType);

        connect(m_calendar, SIGNAL(selectionChanged()), this, SLOT(stub_selectionChanged()));

        m_dateFormat = "yyyy.MM.dd";


    }

    ///Returns a semicolon separated list with all public functions, signals and properties.
    virtual QString getPublicScriptElements(void)
    {
        return MainWindow::parseApiFile("ScriptCalendarWidget.api");
    }

    ///Sets the selected date.
    Q_INVOKABLE void setSelectedDate(QString dateString)
    {
        QDate date = QDate::fromString(dateString, m_dateFormat);
        emit setSelectedDateSignal(date);
    }

    ///Returns the selected date.
    Q_INVOKABLE QString getSelectedDate(void){ return m_calendar->selectedDate().toString(m_dateFormat);}


    ///Sets the date format.
    Q_INVOKABLE void setDateFormat(QString format){m_dateFormat = format;}

    ///Returns the date format.
    Q_INVOKABLE QString getDateFormat(void){return m_dateFormat;}

    ///Sets the minimum and the maximum date.
    Q_INVOKABLE void setDateRange(QString min, QString max){emit setDateRangeSignal(QDate::fromString(min, m_dateFormat), QDate::fromString(max, m_dateFormat));}


Q_SIGNALS:
    ///This signal is emitted if the selected date of has been changed.
    ///Scripts can connect a function to this signal.
    void selectionChangedSignal(QString date);


    ///Is emitted in setSelectedDate.
    ///This signal is private and must not be used inside a script.
    void setSelectedDateSignal(QDate date);

    ///Is emitted in setDateRange.
    ///This signal is private and must not be used inside a script.
    void setDateRangeSignal(const QDate &min, const QDate &max);

private slots:

    ///This slot function is called if the user changes the date.
    ///In this slot function the timeChanged signal is generated
    void stub_selectionChanged()
    {
        emit selectionChangedSignal(m_calendar->selectedDate().toString(m_dateFormat));
    }
private:
    ///The wrapped check box.
    QCalendarWidget* m_calendar;

    ///The date format of the calendar widget.
    QString m_dateFormat;

};

#endif // SCRIPTCALENDARWIDGET_H

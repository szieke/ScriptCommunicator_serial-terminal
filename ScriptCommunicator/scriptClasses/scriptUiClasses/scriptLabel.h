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

#ifndef SCRIPTLABEL_H
#define SCRIPTLABEL_H

#include <QObject>

#include "scriptWidget.h"

///This wrapper class is used to access a QLabel object (located in a script gui/ui-file) from a script.
class ScriptLabel : public ScriptWidget
{
    Q_OBJECT
public:
    explicit ScriptLabel(QLabel* label, ScriptThread *scriptThread) :
        ScriptWidget(label, scriptThread, scriptThread->getScriptWindow()), m_label(label)
    {
        //connect the necessary signals with the wrapper slots (in this slots the
        //events of the wrapper class are generated, the script can connect to this
        //wrapper events)
        Qt::ConnectionType directConnectionType = scriptThread->runsInDebugger() ? Qt::DirectConnection : Qt::BlockingQueuedConnection;

        connect(this, SIGNAL(setTextSignal(QString)), m_label, SLOT(setText(QString)), directConnectionType);

    }

    ///Returns a semicolon separated list with all public functions, signals and properties.
    virtual QString getPublicScriptElements(void)
    {
        return MainWindow::parseApiFile("ScriptLabel.api");
    }

    ///Sets the label text.
    Q_INVOKABLE void setText(QString text){emit setTextSignal(text);}

    ///Returns the label text.
    Q_INVOKABLE QString text(void){return m_label->text();}

Q_SIGNALS:

    ///Is emitted by the setText function.
    ///This signal is private and must not be used inside a script.
    void setTextSignal(QString text);

private:
    ///The wrapped push button.
    QLabel* m_label;
};

#endif // SCRIPTLABEL_H

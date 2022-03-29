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

#ifndef SCRIPTACTION_H
#define SCRIPTACTION_H

#include <QObject>

#include "scriptWidget.h"
#include "scriptObject.h"

///This wrapper class is used to access a QAction object (located in a script gui/ui-file) from a script.
class ScriptAction : public QObject, public ScriptObject
{
    Q_OBJECT

    ///Returns a semicolon separated list with all public functions, signals and properties.
    Q_PROPERTY(QString publicScriptElements READ getPublicScriptElements)
public:
    explicit ScriptAction(QAction* action, ScriptThread *scriptThread) :
        QObject(scriptThread), m_action(action)
    {
        //connect the necessary signals with the wrapper slots (in this slots the
        //events of the wrapper class are generated, the script can connect to this
        //wrapper events)
        Qt::ConnectionType directConnectionType = scriptThread->runsInDebugger() ? Qt::DirectConnection : Qt::BlockingQueuedConnection;

        connect(m_action, SIGNAL(triggered()), this, SIGNAL(clickedSignal()));

        connect(this, SIGNAL(setTextSignal(QString, QAction*)), scriptThread->getScriptWindow(),
                SLOT(setActionTextSlot(QString,QAction*)), directConnectionType);
        connect(this, SIGNAL(setCheckStateSignal(bool,QAction*)), scriptThread->getScriptWindow(), SLOT(setActionCheckStateSlot(bool,QAction*)));

        connect(this, SIGNAL(setEnabledSignal(bool)), m_action, SLOT(setEnabled(bool)), Qt::QueuedConnection);

    }
    ///Returns a semicolon separated list with all public functions, signals and properties.
    virtual QString getPublicScriptElements(void)
    {
        return MainWindow::parseApiFile("ScriptAction.api");
    }

    ///Sets the action text.
    Q_INVOKABLE void setText(const QString text){emit setTextSignal(text, m_action);}

    ///Returns the action text.
    Q_INVOKABLE QString text(void){return m_action->text();}

    ///Sets the checked state of the action.
    Q_INVOKABLE void setChecked(bool checked){emit setCheckStateSignal(checked, m_action);}

    ///Returns true of the action is checked.
    Q_INVOKABLE bool isChecked(void){return m_action->isChecked();}

    ///Enables or disables the widget.
    Q_INVOKABLE void setEnabled(bool isEnabled){emit setEnabledSignal(isEnabled);}

Q_SIGNALS:
    ///This signal is emitted if the user presses the action.
    ///Scripts can connect a function to this signal.
    void clickedSignal(void);

    ///Is emitted in setText.
    ///This signal is private and must not be used inside a script.
    void setTextSignal(QString text, QAction* action);

    ///Is emitted in setChecked.
    ///This signal is private and must not be used inside a script.
    void setCheckStateSignal(bool checked, QAction* action);

    ///This signal is emitted if the setEnabled function is called.
    ///This signal is private and must not be used inside a script.
    void setEnabledSignal(bool);

private:
    ///The wrapped action.
    QAction* m_action;
};

#endif // SCRIPTACTION_H

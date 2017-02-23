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

#ifndef SCRIPTCHECKBOX_H
#define SCRIPTCHECKBOX_H

#include <QObject>
#include "scriptWidget.h"

///This wrapper class is used to access a QCheckBox object (located in a script gui/ui-file) from a script.
class ScriptCheckBox : public ScriptWidget
{
    Q_OBJECT
public:
    explicit ScriptCheckBox(QCheckBox* checkBox, ScriptThread *scriptThread) :
        ScriptWidget(checkBox,scriptThread, scriptThread->getScriptWindow()), m_checkBox(checkBox)
    {
        //connect the necessary signals with the wrapper slots (in this slots the
        //events of the wrapper class are generated, the script can connect to this
        //wrapper events)

        Qt::ConnectionType directConnectionType = scriptThread->runsInDebugger() ? Qt::DirectConnection : Qt::BlockingQueuedConnection;

        connect(m_checkBox, SIGNAL(clicked(bool)), this, SIGNAL(clickedSignal(bool)));

        connect(this, SIGNAL(setTextSignal(QString,QCheckBox*)), scriptThread->getScriptWindow(),
                SLOT(setTextSlot(QString,QCheckBox*)), directConnectionType);
        connect(this, SIGNAL(setCheckedSignal(bool,QCheckBox*)), scriptThread->getScriptWindow(),
                SLOT(setCheckedSlot(bool,QCheckBox*)), directConnectionType);

    }

    ///Returns a semicolon separated list with all public functions, signals and properties.
    virtual QString getPublicScriptElements(void)
    {
        return MainWindow::parseApiFile("ScriptCheckBox.api");
    }

    ///Sets the check box text.
    Q_INVOKABLE void setText(const QString text){emit setTextSignal(text, m_checkBox);}

    ///Returns the check box text.
    Q_INVOKABLE QString text(void){return m_checkBox->text();}

    ///Sets the checked state of the check box.
    Q_INVOKABLE void setChecked(bool checked){emit setCheckedSignal(checked, m_checkBox);}

    ///Returns true of the check box is checked.
    Q_INVOKABLE bool isChecked(void){return m_checkBox->isChecked();}



Q_SIGNALS:
    ///This signal is emitted if the user presses the check box.
    ///Scripts can connect a function to this signal.
    void clickedSignal(bool checked);

    ///Is emitted in setText.
    ///This signal is private and must not be used inside a script.
    void setTextSignal(const QString text, QCheckBox* box);

    ///Is emitted in setChecked.
    ///This signal is private and must not be used inside a script.
    void setCheckedSignal(bool checked, QCheckBox* box);

private:
    ///The wrapped check box.
    QCheckBox* m_checkBox;

};

#endif // SCRIPTCHECKBOX_H

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

#ifndef SCRIPTRADIOBUTTON_H
#define SCRIPTRADIOBUTTON_H

#include <QObject>
#include "scriptWidget.h"

///This wrapper class is used to access a QRadioButton object (located in a script gui/ui-file) from a script.
class ScriptRadioButton : public ScriptWidget
{
    Q_OBJECT
public:
    explicit ScriptRadioButton(QRadioButton* radioButton, ScriptThread *scriptThread) :
        ScriptWidget(radioButton,scriptThread, scriptThread->getScriptWindow()), m_radioButton(radioButton)
    {
        //connect the necessary signals with the wrapper slots (in this slots the
        //events of the wrapper class are generated, the script can connect to this
        //wrapper events)
        Qt::ConnectionType directConnectionType = scriptThread->runsInDebugger() ? Qt::DirectConnection : Qt::BlockingQueuedConnection;

        connect(m_radioButton, SIGNAL(clicked(bool)), this, SIGNAL(clickedSignal(bool)));

        connect(this, SIGNAL(setTextSignal(QString,QRadioButton*)),scriptThread->getScriptWindow(),
                SLOT(setTextSlot(QString,QRadioButton*)), directConnectionType);

        connect(this, SIGNAL(setCheckedSignal(bool,QRadioButton*)),scriptThread->getScriptWindow(),
                SLOT(setCheckedSlot(bool,QRadioButton*)), directConnectionType);

    }

    ///Returns a semicolon separated list with all public functions, signals and properties.
    virtual QString getPublicScriptElements(void)
    {
        return MainWindow::parseApiFile("ScriptRadioButton.api");
    }

    ///Sets the radio button text.
    Q_INVOKABLE void setText(const QString text){emit setTextSignal(text, m_radioButton);}

    ///Returns the radio button text.
    Q_INVOKABLE QString text(void){return m_radioButton->text();}

    ///Sets the checked state of the radio button.
    Q_INVOKABLE void setChecked(bool checked){emit setCheckedSignal(checked, m_radioButton);}

    ///Returns true of the radio button is checked.
    Q_INVOKABLE bool isChecked(void){return m_radioButton->isChecked();}


Q_SIGNALS:
    ///This signal is emitted if the user presses the check box.
    void clickedSignal(bool checked);

    ///Is emitted by the setText function.
    ///This signal is private and must not be used inside a script.
    void setTextSignal(QString text, QRadioButton* button);

    ///Is emitted by the setChecked function.
    ///This signal is private and must not be used inside a script.
    void setCheckedSignal(bool checked, QRadioButton* button);

private:
    ///The wrapped check box.
    QRadioButton* m_radioButton;

};

#endif // SCRIPTRADIOBUTTON_H

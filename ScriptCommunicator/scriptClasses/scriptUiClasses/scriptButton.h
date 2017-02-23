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

#ifndef SCRIPTBUTTON_H
#define SCRIPTBUTTON_H

#include <QObject>

#include "scriptWidget.h"

///This wrapper class is used to access a QPushButton object (located in a script gui/ui-file) from a script.
class ScriptButton : public ScriptWidget
{
    Q_OBJECT
public:
    explicit ScriptButton(QPushButton* button, ScriptThread *scriptThread) :
        ScriptWidget(button, scriptThread, scriptThread->getScriptWindow()), m_button(button)
    {
        //connect the necessary signals with the wrapper slots (in this slots the
        //events of the wrapper class are generated, the script can connect to this
        //wrapper events)

        Qt::ConnectionType directConnectionType = scriptThread->runsInDebugger() ? Qt::DirectConnection : Qt::BlockingQueuedConnection;

        connect(m_button, SIGNAL(clicked()), this, SIGNAL(clickedSignal()), Qt::QueuedConnection);

        connect(this, SIGNAL(setItemIconSignal(QAbstractButton*,QString)), scriptThread->getScriptWindow(),
                SLOT(setItemIconSlot(QAbstractButton*,QString)), Qt::QueuedConnection);

        connect(this, SIGNAL(setCheckedSignal(bool)), m_button, SLOT(setChecked(bool)), Qt::QueuedConnection);

        connect(this, SIGNAL(setTextSignal(QString,QPushButton*)), scriptThread->getScriptWindow(),
                SLOT(setTextSlot(QString,QPushButton*)), directConnectionType);

    }

    ///Returns a semicolon separated list with all public functions, signals and properties.
    virtual QString getPublicScriptElements(void)
    {
        return MainWindow::parseApiFile("ScriptButton.api");
    }

    ///Sets the button text.
    Q_INVOKABLE void setText(QString text){emit setTextSignal(text, m_button);}

    ///Returns the button text.
    Q_INVOKABLE QString text(void){return m_button->text();}

    ///Sets the icon of the button.
    Q_INVOKABLE void setIcon(QString iconFileName){emit setItemIconSignal(m_button, iconFileName);}

    ///Sets the checkable property of the button.
    Q_INVOKABLE void setCheckable(bool checkable){m_button->setCheckable(checkable);}

    ///Returns true if the button is checkable and false if not.
    Q_INVOKABLE bool isCheckable(void){return m_button->isCheckable();}

    ///Returns true if the button is checked and false if not.
    Q_INVOKABLE bool isChecked(void){return m_button->isChecked();}

    ///Checks or unchecks the button.
    Q_INVOKABLE void setChecked(bool checked){emit setCheckedSignal(checked);}


Q_SIGNALS:
    ///This signal is emitted if the user presses the button.
    ///Scripts can connect a function to this signal.
    void clickedSignal(void);

    ///This signal is emitted in setItemIcon.
    ///This signal is private and must not be used inside a script.
    void setItemIconSignal(QAbstractButton* button, QString iconFileName);

    ///This signal is emitted in setText.
    ///This signal is private and must not be used inside a script.
    void setTextSignal(const QString text, QPushButton* button);

    ///This signal is emitted in setChecked.
    ///This signal is private and must not be used inside a script.
    void setCheckedSignal(bool checked);


private:
    ///The wrapped push button.
    QPushButton* m_button;
};

#endif // SCRIPTBUTTON_H

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

#ifndef SCRIPTSPINBOX_H
#define SCRIPTSPINBOX_H

#include <QObject>

#include "scriptWidget.h"

///This wrapper class is used to access a QSpinBox object (located in a script gui/ui-file) from a script.
class ScriptSpinBox : public ScriptWidget
{
    Q_OBJECT
public:
    explicit ScriptSpinBox(QSpinBox* spinBox, ScriptThread *scriptThread) :
        ScriptWidget(spinBox, scriptThread, scriptThread->getScriptWindow()), m_spinBox(spinBox)
    {
        //connect the necessary signals with the wrapper slots (in this slots the
        //events of the wrapper class are generated, the script can connect to this
        //wrapper events)
        Qt::ConnectionType directConnectionType = scriptThread->runsInDebugger() ? Qt::DirectConnection : Qt::BlockingQueuedConnection;

        connect(this, SIGNAL(setRangeSignal(int,int, QSpinBox*)),scriptThread->getScriptWindow(),
                SLOT(setRangeSlot(int,int, QSpinBox*)), directConnectionType);

        connect(this, SIGNAL(setValueSignal(int)),m_spinBox, SLOT(setValue(int)), directConnectionType);

        connect(this, SIGNAL(setSingleStepSignal(int,QSpinBox*)),scriptThread->getScriptWindow(),
                SLOT(setSingleStepSlot(int,QSpinBox*)), directConnectionType);

        connect(m_spinBox, SIGNAL(valueChanged(int)),this, SIGNAL(valueChangedSignal(int)));
    }

    ///Returns a semicolon separated list with all public functions, signals and properties.
    virtual QString getPublicScriptElements(void)
    {
        return MainWindow::parseApiFile("ScriptSpinBox.api");
    }

    ///Sets the spin box's minimum and maximum values to minimum and maximum respectively.
    Q_INVOKABLE void setRange(int minimum, int maximum){emit setRangeSignal(minimum, maximum, m_spinBox);}

    ///Sets the spin box's current value.
    Q_INVOKABLE void setValue(int value){emit setValueSignal(value);}

    ///Returns the spin box's current value.
    Q_INVOKABLE int value(void){return m_spinBox->value();}

    ///If the user uses the arrows to change the spin box's value the value will be
    ///incremented/decremented by the amount of the single step. The default value is 1.
    ///Setting a single step value of less than 0 does nothing.
    Q_INVOKABLE void setSingleStep(int value){emit setSingleStepSignal(value, m_spinBox);}

    ///Returns the single step value.
    Q_INVOKABLE int singleStep(void){return m_spinBox->singleStep();}



Q_SIGNALS:

    ///This signal is emitted if the value of the spin box has been changed.
    void valueChangedSignal(int currentValue);

    ///Is emitted by the setRange function.
    ///This signal is private and must not be used inside a script.
    void setRangeSignal(int minimum, int maximum, QSpinBox* spinBox);

    ///Is emitted by the setValue function.
    ///This signal is private and must not be used inside a script.
    void setValueSignal(int value);

    ///Is emitted by the setSingleStep function.
    ///This signal is private and must not be used inside a script.
    void setSingleStepSignal(int value, QSpinBox* spinBox);

private:
    ///The wrapped progress bar.
    QSpinBox* m_spinBox;
};

#endif // SCRIPTSPINBOX_H

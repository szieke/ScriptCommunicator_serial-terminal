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

#ifndef SCRIPTDOUBLESPINBOX_H
#define SCRIPTDOUBLESPINBOX_H

#include <QObject>

#include "scriptWidget.h"

///This wrapper class is used to access a QDoubleSpinBox object (located in a script gui/ui-file) from a script.
class ScriptDoubleSpinBox : public ScriptWidget
{
    Q_OBJECT
public:
    explicit ScriptDoubleSpinBox(QDoubleSpinBox* spinBox, ScriptThread *scriptThread) :
        ScriptWidget(spinBox, scriptThread, scriptThread->getScriptWindow()), m_spinBox(spinBox)
    {
        //connect the necessary signals with the wrapper slots (in this slots the
        //events of the wrapper class are generated, the script can connect to this
        //wrapper events)
        Qt::ConnectionType directConnectionType = scriptThread->runsInDebugger() ? Qt::DirectConnection : Qt::BlockingQueuedConnection;

        connect(this, SIGNAL(setRangeSignal(double,double, QDoubleSpinBox*)),scriptThread->getScriptWindow(),
                SLOT(setRangeSlot(double,double, QDoubleSpinBox*)), directConnectionType);

        connect(this, SIGNAL(setValueSignal(double)),m_spinBox, SLOT(setValue(double)), directConnectionType);

        connect(this, SIGNAL(setDecimalsSignal(int,QDoubleSpinBox*)),scriptThread->getScriptWindow(),
                SLOT(setDecimalsSlot(int,QDoubleSpinBox*)),directConnectionType);

        connect(this, SIGNAL(setSingleStepSignal(double,QDoubleSpinBox*)),scriptThread->getScriptWindow(),
                SLOT(setSingleStepSlot(double,QDoubleSpinBox*)), directConnectionType);

        connect(m_spinBox, SIGNAL(valueChanged(double)),this, SIGNAL(valueChangedSignal(double)));
    }

    ///Returns a semicolon separated list with all public functions, signals and properties.
    virtual QString getPublicScriptElements(void)
    {
        return MainWindow::parseApiFile("ScriptDoubleSpinBox.api");
    }

    ///Sets the spin box's minimum and maximum values to minimum and maximum respectively.
    Q_INVOKABLE void setRange(double minimum, double maximum){emit setRangeSignal(minimum, maximum, m_spinBox);}

    ///Sets the spin box's current value.
    Q_INVOKABLE void setValue(double value){emit setValueSignal(value);}

    ///Returns the spin box's current value.
    Q_INVOKABLE double value(void){return m_spinBox->value();}

    ///If the user uses the arrows to change the spin box's value the value will be
    ///incremented/decremented by the amount of the single step. The default value is 1.0.
    ///Setting a single step value of less than 0 does nothing.
    Q_INVOKABLE void setSingleStep(double value){emit setSingleStepSignal(value, m_spinBox);}

    ///Returns the single step value.
    Q_INVOKABLE double singleStep(void){return m_spinBox->singleStep();}

    ///Sets the precision of the spin box, in decimals.
    Q_INVOKABLE void setDecimals(int value){emit setDecimalsSignal(value, m_spinBox);}

    ///Returns the precision of the spin box, in decimals.
    Q_INVOKABLE int decimals(void){return m_spinBox->decimals();}



Q_SIGNALS:

    ///This signal is emitted if the value of the spin box has been changed.
    void valueChangedSignal(double);

    ///Is emitted by the setDecimals function.
    ///This signal is private and must not be used inside a script.
    void setDecimalsSignal(int value, QDoubleSpinBox* spinBox);

    ///Is emitted by the setRange function.
    ///This signal is private and must not be used inside a script.
    void setRangeSignal(double minimum, double maximum, QDoubleSpinBox* spinBox);

    ///Is emitted by the setValue function.
    ///This signal is private and must not be used inside a script.
    void setValueSignal(double value);

    ///Is emitted by the setSingleStep function.
    ///This signal is private and must not be used inside a script.
    void setSingleStepSignal(double value, QDoubleSpinBox* spinBox);

private:
    ///The wrapped progress bar.
    QDoubleSpinBox* m_spinBox;
};

#endif // SCRIPTDOUBLESPINBOX_H

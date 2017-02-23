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

#ifndef SCRIPTSLIDER_H
#define SCRIPTSLIDER_H

#include <QObject>

#include "scriptWidget.h"

///This wrapper class is used to access a QSlider object (located in a script gui/ui-file) from a script.
class ScriptSlider : public ScriptWidget
{
    Q_OBJECT
public:
    explicit ScriptSlider(QAbstractSlider* slider, ScriptThread *scriptThread) :
        ScriptWidget(slider, scriptThread, scriptThread->getScriptWindow()), m_slider(slider)
    {
        //connect the necessary signals with the wrapper slots (in this slots the
        //events of the wrapper class are generated, the script can connect to this
        //wrapper events)
        Qt::ConnectionType directConnectionType = scriptThread->runsInDebugger() ? Qt::DirectConnection : Qt::BlockingQueuedConnection;

        connect(this, SIGNAL(setRangeSignal(int,int)),m_slider, SLOT(setRange(int,int)), directConnectionType);
        connect(this, SIGNAL(setValueSignal(int)),m_slider, SLOT(setValue(int)), directConnectionType);
        connect(m_slider, SIGNAL(valueChanged(int)),this, SIGNAL(valueChangedSignal(int)));
    }

    ///Returns a semicolon separated list with all public functions, signals and properties.
    virtual QString getPublicScriptElements(void)
    {
        return MainWindow::parseApiFile("ScriptSlider.api");
    }

    ///Sets the slider's minimum to min and its maximum to max.
    Q_INVOKABLE void setRange(int min, int max){emit setRangeSignal(min, max);}

    ///Sets the slider's current value.
    Q_INVOKABLE void setValue(int value){emit setValueSignal(value);}

    ///Returns the slider's current value.
    Q_INVOKABLE int value(void){return m_slider->value();}


Q_SIGNALS:

    ///This signal is emitted if the value of the slider has been changed.
    ///Scripts can connect a function to this signal.
    void valueChangedSignal(int value);

    ///Is emitted by the setRange function.
    ///This signal is private and must not be used inside a script.
    void setRangeSignal(int minimum, int maximum);

    ///Is emitted by the setValue function.
    ///This signal is private and must not be used inside a script.
    void setValueSignal(int value);


private:
    ///The wrapped progress bar.
    QAbstractSlider* m_slider;
};

#endif // SCRIPTSLIDER_H

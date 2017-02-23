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

#ifndef SCRIPTPROGRESSBAR_H
#define SCRIPTPROGRESSBAR_H

#include <QObject>

#include "scriptWidget.h"

///This wrapper class is used to access a QProgressBar object (located in a script gui/ui-file) from a script.
class ScriptProgressBar : public ScriptWidget
{
    Q_OBJECT
public:
    explicit ScriptProgressBar(QProgressBar* progress, ScriptThread *scriptThread) :
        ScriptWidget(progress, scriptThread, scriptThread->getScriptWindow()), m_progress(progress)
    {
        //connect the necessary signals with the wrapper slots (in this slots the
        //events of the wrapper class are generated, the script can connect to this
        //wrapper events)
        connect(this, SIGNAL(setRangeSignal(int,int)),m_progress, SLOT(setRange(int,int)), Qt::QueuedConnection);
        connect(this, SIGNAL(resetSignal()),m_progress, SLOT(reset()), Qt::QueuedConnection);
        connect(this, SIGNAL(setMinimumSignal(int)),m_progress, SLOT(setMinimum(int)), Qt::QueuedConnection);
        connect(this, SIGNAL(setMaximumSignal(int)),m_progress, SLOT(setMaximum(int)), Qt::QueuedConnection);
        connect(this, SIGNAL(setValueSignal(int)),m_progress, SLOT(setValue(int)), Qt::QueuedConnection);


    }

    ///Returns a semicolon separated list with all public functions, signals and properties.
    virtual QString getPublicScriptElements(void)
    {
        return MainWindow::parseApiFile("ScriptProgressBar.api");
    }

    ///Resets the progress bar. The progress bar rewinds and shows no progress.
    Q_INVOKABLE void reset(void){emit resetSignal();}

    ///Sets the progress bar's minimum and maximum values to minimum and maximum respectively.
    Q_INVOKABLE void setRange(int minimum, int maximum){emit setRangeSignal(minimum, maximum);}

    ///Sets the progress bar's minimum value.
    Q_INVOKABLE void setMinimum(int minimum){emit setMinimumSignal(minimum);}

    ///Sets the progress bar's maximum value.
    Q_INVOKABLE void setMaximum(int maximum){emit setMaximumSignal(maximum);}

    ///Sets the progress bar's current value.
    Q_INVOKABLE void setValue(int value){emit setValueSignal(value);}


Q_SIGNALS:

    ///Is emitted by the reset function.
    ///This signal is private and must not be used inside a script.
    void resetSignal();

    ///Is emitted by the setRange function.
    ///This signal is private and must not be used inside a script.
    void setRangeSignal(int minimum, int maximum);

    ///Is emitted by the setMinimum function.
    ///This signal is private and must not be used inside a script.
    void setMinimumSignal(int minimum);

    ///Is emitted by the setMaximum function.
    ///This signal is private and must not be used inside a script.
    void setMaximumSignal(int maximum);

    ///Is emitted by the setValue function.
    ///This signal is private and must not be used inside a script.
    void setValueSignal(int value);


private:
    ///The wrapped progress bar.
    QProgressBar* m_progress;
};

#endif // SCRIPTPROGRESSBAR_H

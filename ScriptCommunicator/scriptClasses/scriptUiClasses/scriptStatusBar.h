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

#ifndef SCRIPTSTATUSBAR_H
#define SCRIPTSTATUSBAR_H

#include <QObject>

#include "scriptWidget.h"

///This wrapper class is used to access a QStatusBar object (located in a script gui/ui-file) from a script.
class ScriptStatusBar : public ScriptWidget
{
    Q_OBJECT
public:
    explicit ScriptStatusBar(QStatusBar* statusBar, ScriptThread *scriptThread) :
        ScriptWidget(statusBar, scriptThread, scriptThread->getScriptWindow()), m_statusBar(statusBar)
    {
        //connect the necessary signals with the wrapper slots (in this slots the
        //events of the wrapper class are generated, the script can connect to this
        //wrapper events)
       connect(this, SIGNAL(showMessageSignal(QString,int)), m_statusBar, SLOT(showMessage(QString,int)));

    }

    ///Returns a semicolon separated list with all public functions, signals and properties.
    virtual QString getPublicScriptElements(void)
    {
        return ScriptWidget::getPublicScriptElements() + ";void showMessage(QString text, int duration)";
    }

    ///Shows a message in the status bar.
    Q_INVOKABLE void showMessage(QString text, int duration){emit showMessageSignal(text, duration);}

Q_SIGNALS:

    ///Is emitted by the showMessage function.
    ///This signal is private and must not be used inside a script.
    void showMessageSignal(const QString &text, int timeout);

private:
    ///The wrapped push button.
    QStatusBar* m_statusBar;
};

#endif // SCRIPTSTATUSBAR_H

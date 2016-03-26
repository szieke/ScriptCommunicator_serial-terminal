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

#ifndef SCRIPTMAINWINDOW_H
#define SCRIPTMAINWINDOW_H

#include <QObject>

#include "scriptWidget.h"



///This wrapper class is used to access a QMainWindow object (located in a script gui/ui-file) from a script.
class ScriptMainWindow : public ScriptWidget
{
    Q_OBJECT
public:
    explicit ScriptMainWindow(QMainWindow* window, ScriptThread *scriptThread) :
        ScriptWidget(window, scriptThread, scriptThread->getScriptWindow()), m_window(window), m_wasVisible(window->isVisible())
    {
        //connect the necessary signals with the wrapper slots (in this slots the
        //events of the wrapper class are generated, the script can connect to this
        //wrapper events)
        connect(&m_timer, SIGNAL(timeout()), this, SLOT(checkIfClosedTimerFunctionSlot()));

        m_timer.setInterval(500);
        m_timer.start();

    }



Q_SIGNALS:
    ///This signal is emitted if the user presses the button.
    ///Scripts can connect a function to this signal.
    void finishedSignal(void);

public slots:

    ///Brings this window to foreground.
    ///Note: This is an internal function and must not be used by a script.
    void bringWindowsToFrontSlot(void)
    {
        if(m_window->isVisible())
        {
            m_window->setWindowState( (m_window->windowState() & ~Qt::WindowMinimized) | Qt::WindowActive);
            m_window->raise();  // for MacOS
            m_window->activateWindow(); // for Windows
        }
    }
private slots:

    ///This slot function checks if the window has been closed and emits the
    ///finishedSignal Signal.
    void checkIfClosedTimerFunctionSlot(void)
    {
        if(m_wasVisible && !m_window->isVisible())
        {//window has been closed
            emit finishedSignal();
        }
        m_wasVisible = m_window->isVisible();

    }

private:
    ///The wrapped dialog.
    QMainWindow* m_window;

    ///True if the window was visible.
    bool m_wasVisible;

    ///This timer calls checkIfClosedTimerFunctionSlot.
    QTimer m_timer;
};

#endif // SCRIPTMAINWINDOW_H

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

#ifndef SCRIPTDIALOG_H
#define SCRIPTDIALOG_H

#include <QObject>

#include "scriptWidget.h"

///This wrapper class is used to access a QPushButton object (located in a script gui/ui-file) from a script.
class ScriptDialog : public ScriptWidget
{
    Q_OBJECT
public:
    explicit ScriptDialog(QDialog* dialog, ScriptThread *scriptThread) :
        ScriptWidget(dialog, scriptThread, scriptThread->getScriptWindow()), m_dialog(dialog)
    {
        //connect the necessary signals with the wrapper slots (in this slots the
        //events of the wrapper class are generated, the script can connect to this
        //wrapper events)
        connect(m_dialog, SIGNAL(finished(int)), this, SIGNAL(finishedSignal()), Qt::QueuedConnection);
    }

    ///Returns a semicolon separated list with all public functions, signals and properties.
    virtual QString getPublicScriptElements(void)
    {
        return MainWindow::parseApiFile("ScriptDialog.api");
    }

Q_SIGNALS:
    ///This signal is emitted if the user closes the dialog.
    ///Scripts can connect a function to this signal.
    void finishedSignal(void);

public slots:
    ///Brings this window to foreground.
    ///Note: This is an internal function and must not be used by a script.
    void bringWindowsToFrontSlot(void)
    {
        if(m_dialog->isVisible())
        {
            m_dialog->setWindowState( (m_dialog->windowState() & ~Qt::WindowMinimized) | Qt::WindowActive);
            m_dialog->raise();  // for MacOS
            m_dialog->activateWindow(); // for Windows
        }
    }

private:
    ///The wrapped dialog.
    QDialog* m_dialog;
};

#endif // SCRIPTDIALOG_H

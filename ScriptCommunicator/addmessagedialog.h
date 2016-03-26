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

#ifndef ADDMESSAGEDIALOG_H
#define ADDMESSAGEDIALOG_H

#include <QDialog>
#include "settingsdialog.h"

namespace Ui {
class AddMessageDialog;
}

class AddMessageDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AddMessageDialog(QWidget *parent, SettingsDialog* settingsDialog);
    ~AddMessageDialog();

signals:
    void messageEnteredSignal(QString text, bool forceTimeStamp);

private slots:

    ///Enter button slot function.
    void enterButtonClicketSlot(void);

private:
    Ui::AddMessageDialog *ui;
    ///Pointer to the settings dialog.
    SettingsDialog *m_settingsDialog;
};

#endif // ADDMESSAGEDIALOG_H

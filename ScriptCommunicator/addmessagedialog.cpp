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

#include "addmessagedialog.h"
#include "ui_addmessagedialog.h"

AddMessageDialog::AddMessageDialog(QWidget *parent, SettingsDialog *settingsDialog) :
    QDialog(parent), ui(new Ui::AddMessageDialog), m_settingsDialog(settingsDialog)
{
    ui->setupUi(this);

    connect(ui->EnterPushButton, SIGNAL(clicked()), this, SLOT(enterButtonClicketSlot()));

}

AddMessageDialog::~AddMessageDialog()
{
    delete ui;
}

/**
 * Enter button slot function.
 */
void AddMessageDialog::enterButtonClicketSlot(void)
{
    const Settings* settings = m_settingsDialog->settings();

    emit messageEnteredSignal(ui->textEdit->toPlainText().toUtf8().replace("\n", settings->consoleSendOnEnter.toUtf8()), false);
}

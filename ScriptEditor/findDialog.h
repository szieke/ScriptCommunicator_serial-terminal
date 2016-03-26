/***************************************************************************
**                                                                        **
**  ScriptEditor is a simple script editor for ScriptCommunicator scripts.**
**  Copyright (C) 2016 Stefan Zieker                                      **
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

#ifndef FINDDIALOG_H
#define FINDDIALOG_H

#include <QDialog>
#include <QShowEvent>

namespace Ui {
class FindDialog;
}

///Class for the find ui.
class FindDialog : public QDialog
{
    Q_OBJECT

    friend class MainWindow;

public:
    explicit FindDialog(QWidget *parent = 0);
    ~FindDialog();

protected:
     void showEvent(QShowEvent *event);
private:
    Ui::FindDialog *ui;


};

#endif // FINDDIALOG_H

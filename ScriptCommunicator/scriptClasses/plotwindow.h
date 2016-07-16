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

#ifndef PLOTWINDOW_H
#define PLOTWINDOW_H

#include <QMainWindow>
#include "qcustomplot.h"
#include "scriptPlotwidget.h"

namespace Ui {
class PlotWindow;
}



class ScriptWindow;

///Class for plotting graphs via script.
class PlotWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit PlotWindow(ScriptThread *scriptThread, ScriptWindow* scriptWindow);
    virtual ~PlotWindow();

    ///The function is called if the plot window is resized.
    ///It repaints the complete window to prevent a black beam in the diagram.
    void resizeEvent(QResizeEvent * event);

    ///Returns the plot widget.
    ScriptPlotWidget* getWidget(void){return m_plotWidget;}

signals:

    ///This signals is emitted if the plot window has been close.
    void closedSignal(void);

private:

    ///This function is called if the plot window is called.
    void closeEvent(QCloseEvent * event);

    ///The user interface.
    Ui::PlotWindow *m_userInterface;

    ScriptPlotWidget* m_plotWidget;

};
#endif // PLOTWINDOW_H

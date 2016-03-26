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

#include "plotwindow.h"
#include "ui_plotwindow.h"
#include <QTimer>
#include "scriptwindow.h"

/**
 * Constructor
 *
 * @param parent
 *      Parent widget.
 */
PlotWindow::PlotWindow(ScriptThread* scriptThread, ScriptWindow *scriptWindow) :
    QMainWindow(0),
    m_userInterface(new Ui::PlotWindow)
{
    m_userInterface->setupUi(this);
    m_plotWidget = new ScriptPlotWidget(scriptThread, scriptWindow, m_userInterface->horizontalLayout);


}

/**
 * Destructor
 */
PlotWindow::~PlotWindow()
{
    delete m_plotWidget;
    delete m_userInterface;

}


/**
 * The function is called if the plot window is resized.
 * It repaints the complete window to prevent a black beam in the diagram.
 * @param event
 *      The resize event.
 */
void PlotWindow::resizeEvent(QResizeEvent * event)
{
    //call repaint to prevent a black beam in the diagram
    repaint();
    (void)event;
}


/**
 * This function is called if the plot window is called.
 * @param event
 *      The close event.
 */
void PlotWindow::closeEvent(QCloseEvent * event)
{
    emit closedSignal();
    event->accept();
}





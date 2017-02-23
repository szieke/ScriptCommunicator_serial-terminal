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

#ifndef SCRIPTPLOTWINDOW_H
#define SCRIPTPLOTWINDOW_H

#include <QObject>

#include "scriptWidget.h"
#include "plotwindow.h"


///This wrapper class is used to access a PlotWindow object from a script.
class ScriptPlotWindow : public ScriptWidget
{
    Q_OBJECT

public:
    explicit ScriptPlotWindow(PlotWindow* plotWindow, ScriptThread *scriptThread, ScriptWindow *scriptWindow) : ScriptWidget(plotWindow, scriptThread, scriptWindow),
        m_plotWindow(plotWindow)
    {
        //connect the necessary signals with the wrapper slots (in this slots the
        //events of the wrapper class are generated, the script can connect to this
        //wrapper events)

        //ScriptPlotWindow needs no parent.
         setParent(0);

        Qt::ConnectionType directConnectionType = scriptThread->runsInDebugger()  ? Qt::DirectConnection : Qt::BlockingQueuedConnection ;

        connect(this, SIGNAL(addGraphSignal(QString, QString, QString, int*)),
                m_plotWindow->getWidget(), SLOT(addGraphSlot(QString, QString, QString, int*)), directConnectionType);

        connect(this, SIGNAL(setInitialAxisRangesSignal(double, double, double)),
                m_plotWindow->getWidget(), SLOT(setInitialAxisRangesSlot(double, double, double)), Qt::QueuedConnection);

        connect(this, SIGNAL(addDataToGraphSignal(int, double, double)),
                m_plotWindow->getWidget(), SLOT(addDataToGraphSlot(int, double, double)), Qt::QueuedConnection);

        connect(this, SIGNAL(setAxisLabelsSignal(QString,QString)),
                m_plotWindow->getWidget(), SLOT(setAxisLabelsSlot(QString,QString)), Qt::QueuedConnection);

        connect(this, SIGNAL(showLegendSignal(bool)),
                m_plotWindow->getWidget(), SLOT(showLegendSlot(bool)), Qt::QueuedConnection);

        connect(m_plotWindow, SIGNAL(closedSignal()), this, SIGNAL(closedSignal()), Qt::QueuedConnection);

        connect(m_plotWindow->getWidget(), SIGNAL(clearButtonPressedSignal()),
                this, SLOT(clearButtonPressedSlot()), Qt::QueuedConnection);

        connect(m_plotWindow->getWidget(), SIGNAL(plotMousePressSignal(double,double,quint32)),
                this, SLOT(plotMousePressSlot(double,double,quint32)), Qt::QueuedConnection);

        connect(this, SIGNAL(clearGraphsSignal()),
                m_plotWindow->getWidget(), SLOT(clearGraphsSlot()), directConnectionType);

        connect(this, SIGNAL(removeAllGraphsSignal()),
                m_plotWindow->getWidget(), SLOT(removeAllGraphsSlot()), directConnectionType);

        connect(this, SIGNAL(showHelperElementsSignal(bool,bool,bool,bool,bool,bool,bool,quint32,bool)),
                m_plotWindow->getWidget(), SLOT(showHelperElementsSlot(bool,bool,bool,bool,bool,bool,bool,quint32,bool)), directConnectionType);

    }

    virtual ~ScriptPlotWindow()
    {
        delete m_plotWindow;
    }

    ///Returns a semicolon separated list with all public functions, signals and properties.
    virtual QString getPublicScriptElements(void)
    {
        return MainWindow::parseApiFile("ScriptPlotWindow.api");
    }

    ///This function adds a graph to the diagram.
    Q_INVOKABLE int addGraph(QString color, QString penStyle, QString name)
    {
        int graphIndex = 0;
        emit addGraphSignal(color, penStyle, name, &graphIndex);
        return graphIndex;
    }

    ///The slot function sets the initial ranges of the diagram.
    Q_INVOKABLE void setInitialAxisRanges(double xRange, double yMinValue, double ymaxValue){emit setInitialAxisRangesSignal(xRange, yMinValue, ymaxValue);}

    ///This slot function adds one point to a graph.
    Q_INVOKABLE void addDataToGraph(int graphIndex, double x, double y){emit addDataToGraphSignal(graphIndex, x, y);}


    ///Sets the axis label.
    Q_INVOKABLE void setAxisLabels(QString xAxisLabel, QString yAxisLabel){emit setAxisLabelsSignal(xAxisLabel, yAxisLabel);}

    ///This function shows or hides the diagram legend.
    Q_INVOKABLE void showLegend(bool show){emit showLegendSignal(show);}

    ///This function clears the data of all graphs.
    Q_INVOKABLE void clearGraphs(void){emit clearGraphsSignal();}

    ///This function removes all graphs.
    Q_INVOKABLE void removeAllGraphs(void){emit removeAllGraphsSignal();}

    ///Sets the visibility of several plot widget elements.
    Q_INVOKABLE void showHelperElements(bool showXRange, bool showYRange, bool showUpdate, bool showSave, bool showLoad, bool showClear, bool showGraphVisibility,
                                        quint32 graphVisibilityMaxSize=80, bool showLegend=true)
    {emit showHelperElementsSignal(showXRange, showYRange, showUpdate, showSave, showLoad, showClear, showGraphVisibility, graphVisibilityMaxSize, showLegend);}

    ///Sets tThe max. number of data points per graph (the default is 10.000.000).
    Q_INVOKABLE void setMaxDataPointsPerGraph(qint32 maxDataPointsPerGraph){m_plotWindow->getWidget()->setMaxDataPointsPerGraph(maxDataPointsPerGraph);}

    ///Sets the update-interval.
    Q_INVOKABLE void setUpdateInterval(quint32 updateInterval){m_plotWindow->getWidget()->setUpdateInterval(updateInterval);}


Q_SIGNALS:

    ///Is emitted if the user press a mouse button inside the plot.
    ///Note: mouseButton has the type Qt::MouseButton.
    ///Scripts can connect to this signal.
    void plotMousePressSignal(double xValue, double yValue, quint32 mouseButton);

    ///Is emitted if the user clicks the clear button.
    ///Scripts can connect to this signal.
    void clearButtonPressedSignal(void);

    ///This signal is emitted if the plot window has been closed.
    ///Scripts can connect a function to this signal.
    void closedSignal(void);

    ///Is emitted in clearGraphs.
    ///This signal is private and must not be used inside a script.
    void clearGraphsSignal(void);

    ///Is emitted in clearGraphs.
    ///This signal is private and must not be used inside a script.
    void removeAllGraphsSignal(void);

    ///Is connected with PlotWindow::addGraph (adds a graph).
    ///This signal is private and must not be used inside a script.
    void addGraphSignal(QString color, QString penStyle, QString name, int* graphIndex);

    ///Is connected with PlotWindow::setInitialAxisRanges (sets the ranges of the diagram).
    ///This signal is private and must not be used inside a script.
    void setInitialAxisRangesSignal(double xRange, double yMinValue, double ymaxValue);

    ///Is connected with PlotWindow::addDataToGraph (adds one point to a given specific graph).
    ///This signal is private and must not be used inside a script.
    void addDataToGraphSignal(int graphIndex, double x, double y);

    ///Is connected with PlotWindow::showFromScript (shows the plot window).
    ///This signal is private and must not be used inside a script.
    void showFromScriptSignal();

    ///Is connected with PlotWindow::showFromScript (Sets the axis labels).
    ///This signal is private and must not be used inside a script.
    void setAxisLabelsSignal(QString xAxisLabel, QString yAxisLabel);

    ///Is connected with PlotWindow::showLegendSlot (shows or hides the plot legend).
    ///This signal is private and must not be used inside a script.
    void showLegendSignal(bool show);

    ///Is emitted in showHelperElements.
    ///This signal is private and must not be used inside a script.
    void showHelperElementsSignal(bool showXRange, bool showYRange, bool showUpdate, bool showSave, bool showLoad, bool showClear, bool showGraphVisibility,
                                  quint32 graphVisibilityMaxSize, bool showLegend);

public slots:
    ///Brings this window to foreground.
    ///Note: This is an internal function and must not be used by a script.
    void bringWindowsToFrontSlot(void)
    {
        if(m_plotWindow->isVisible())
        {
            m_plotWindow->setWindowState( (m_plotWindow->windowState() & ~Qt::WindowMinimized) | Qt::WindowActive);
            m_plotWindow->raise();  // for MacOS
            m_plotWindow->activateWindow(); // for Windows
        }
    }

private Q_SLOTS:

    ///This function clears the plot widget.
    ///It is called if the clear button is pressed.
    void clearButtonPressedSlot(){ emit clearButtonPressedSignal();}

    ///Is called if the user press a mouse button inside the plot.
    ///Note: mouseButton has the type Qt::MouseButton.
    void plotMousePressSlot(double xValue, double yValue, quint32 mouseButton){emit plotMousePressSignal(xValue, yValue, mouseButton);}


private:
    ///The wrapped plot window.
    PlotWindow* m_plotWindow;
};

#endif // SCRIPTPLOTWINDOW_H

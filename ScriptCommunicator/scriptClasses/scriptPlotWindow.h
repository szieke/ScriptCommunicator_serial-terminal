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

        //ScriptPlotWindow needs no parent.
         setParent(0);


        connect(m_plotWindow, SIGNAL(closedSignal()), this, SIGNAL(closedSignal()), Qt::QueuedConnection);

        connect(m_plotWindow->getWidget(), SIGNAL(clearButtonPressedSignal()),
                this, SIGNAL(clearButtonPressedSignal()), Qt::QueuedConnection);

        connect(m_plotWindow->getWidget(), SIGNAL(plotMousePressSignal(double,double,quint32)),
                this, SIGNAL(plotMousePressSignal(double,double,quint32)), Qt::QueuedConnection);



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
        emit m_plotWindow->getWidget()->addGraphSignal(color, penStyle, name, &graphIndex);
        return graphIndex;
    }

    ///The slot function sets the initial ranges of the diagram.
    Q_INVOKABLE void setInitialAxisRanges(double xRange, double yMinValue, double ymaxValue){emit m_plotWindow->getWidget()->setInitialAxisRangesSignal(xRange, yMinValue, ymaxValue);}

    ///The function sets the current ranges of the diagram.
    Q_INVOKABLE void setCurrentAxisRanges(double xMinValue, double xMaxValue, double yMinValue, double yMaxValue){m_plotWindow->getWidget()->setCurrentAxisRanges(xMinValue, xMaxValue, yMinValue, yMaxValue);}

    ///The function gets the current ranges of the diagram.
    Q_INVOKABLE QScriptValue getCurrentAxisRanges(void){return m_plotWindow->getWidget()->getCurrentAxisRanges();}

    ///This slot function adds one point to a graph.
    Q_INVOKABLE void addDataToGraph(int graphIndex, double x, double y){emit m_plotWindow->getWidget()->addDataToGraphSignal(graphIndex, x, y);}


    /**
     * Removes all data points with (sort-)keys between xFrom and xTo. If
     * xFrom is greater or equal to xTo, the function does nothing.
     */
    Q_INVOKABLE void removeDataRangeFromGraph(int graphIndex, double xFrom, double xTo){emit m_plotWindow->getWidget()->removeDataRangeFromGraphSignal(graphIndex, xFrom, xTo);}

    /**
     * Sets the visual appearance of single data points in the plot.
     *
     * Possible values for style:
     *      - None: No scatter symbols are drawn.
     *      - Dot: A single pixel.
     *      - Cross: A cross.
     *      - Plus: A plus.
     *      - Circle: A circle.
     *      - Disc: A circle which is filled with the pen's color.
     *      - Square: A square.
     *      - Diamond: A diamond.
     *      - Star: A star with eight arms, i.e. a combination of cross and plus.
     *      - Triangle :An equilateral triangle, standing on baseline.
     *      - TriangleInverted: An equilateral triangle, standing on corner.
     *      - CrossSquare: A square with a cross inside.
     *      - PlusSquare: A square with a plus inside.
     *      - CrossCircle: A circle with a cross inside.
     *      - PlusCircle: A circle with a plus inside.
     *      - Peace: A circle, with one vertical and two downward diagonal lines.
     */
    Q_INVOKABLE void setScatterStyle(int graphIndex, QString style, double size){emit m_plotWindow->getWidget()->setScatterStyleSignal(graphIndex, style, size);}

    /**
     * This function adds one point to a graph.
     * Possible valuesfor style:
     *      - None: Data points are not connected with any lines (e.g. data only represented
     *              with symbols according to the scatter style (is set with setScatterStyle)).
     *      - Line: Data points are connected by a straight line.
     *      - StepLeft: Line is drawn as steps where the step height is the value of the left data point.
     *      - StepRight: Line is drawn as steps where the step height is the value of the right data point.
     *      - StepCenter: Line is drawn as steps where the step is in between two data points.
     *      - Impulse: Each data point is represented by a line parallel to the value axis, which reaches from the data point to the zero-value-line.
     */
    Q_INVOKABLE void setLineStyle(int graphIndex, QString style){emit m_plotWindow->getWidget()->setLineStyleSignal(graphIndex, style);}

    ///Sets the axis label.
    Q_INVOKABLE void setAxisLabels(QString xAxisLabel, QString yAxisLabel){emit m_plotWindow->getWidget()->setAxisLabelsSignal(xAxisLabel, yAxisLabel);}

    ///This function shows or hides the diagram legend.
    Q_INVOKABLE void showLegend(bool show){emit m_plotWindow->getWidget()->showLegendSignal(show);}

    ///This function clears the data of all graphs.
    Q_INVOKABLE void clearGraphs(void){emit m_plotWindow->getWidget()->clearGraphsSignal();}

    ///This function removes all graphs.
    Q_INVOKABLE void removeAllGraphs(void){emit m_plotWindow->getWidget()->removeAllGraphsSignal();}

    ///Sets the visibility of several plot widget elements.
    Q_INVOKABLE void showHelperElements(bool showXRange, bool showYRange, bool showUpdate, bool showSave, bool showLoad, bool showClear, bool showGraphVisibility,
                                        quint32 graphVisibilityMaxSize=80, bool showLegend=true)
    {emit m_plotWindow->getWidget()->showHelperElementsSignal(showXRange, showYRange, showUpdate, showSave, showLoad, showClear, showGraphVisibility, graphVisibilityMaxSize, showLegend);}

    ///Sets tThe max. number of data points per graph (the default is 10.000.000).
    Q_INVOKABLE void setMaxDataPointsPerGraph(qint32 maxDataPointsPerGraph){m_plotWindow->getWidget()->setMaxDataPointsPerGraph(maxDataPointsPerGraph);}

    ///Sets the automatic update enabled state.
    Q_INVOKABLE void setAutoUpdate(bool enabled){ m_plotWindow->getWidget()->setAutoUpdate(enabled);}

    ///Gets the automatic update enabled state.
    Q_INVOKABLE bool autoUpdate(void){return m_plotWindow->getWidget()->autoUpdate();}

    ///Sets the update-interval.
    Q_INVOKABLE void setUpdateInterval(quint32 updateInterval){m_plotWindow->getWidget()->setUpdateInterval(updateInterval);}

    ///Update the current plot view.
    Q_INVOKABLE void updatePlot(void){m_plotWindow->getWidget()->updatePlot();}

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

private:
    ///The wrapped plot window.
    PlotWindow* m_plotWindow;
};

#endif // SCRIPTPLOTWINDOW_H

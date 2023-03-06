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

        connect(m_plotWindow->getWidget(), SIGNAL(xRangeChangedSignal(double)),
                this, SIGNAL(xRangeChangedSignal(double)), Qt::QueuedConnection);

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
    Q_INVOKABLE int addGraph(QString color, QString penStyle, QString name, bool useYAxis2 = false)
    {
        int graphIndex = 0;
        emit m_plotWindow->getWidget()->addGraphSignal(color, penStyle, name, &graphIndex, useYAxis2);
        return graphIndex;
    }

    ///The slot function sets the initial ranges of the diagram.
    Q_INVOKABLE void setInitialAxisRanges(double xRange, double yMinValue, double yMaxValue, bool addSpaceAfterBiggestValues = true, double y2MinValue=0, double y2MaxValue=0)
    {emit m_plotWindow->getWidget()->setInitialAxisRangesSignal(xRange, yMinValue, yMaxValue, addSpaceAfterBiggestValues, y2MinValue, y2MaxValue);}

    ///The function sets the current ranges of the diagram.
    Q_INVOKABLE void setCurrentAxisRanges(double xMinValue, double xMaxValue, double yMinValue, double yMaxValue, double y2MinValue=0, double y2MaxValue=0)
    {m_plotWindow->getWidget()->setCurrentAxisRanges(xMinValue, xMaxValue, yMinValue, yMaxValue, y2MinValue, y2MaxValue);}

    ///The function gets the current ranges of the diagram.
    Q_INVOKABLE QJSValue getCurrentAxisRanges(void){return m_plotWindow->getWidget()->getCurrentAxisRanges();}

    ///This function adds one point to a graph.
    Q_INVOKABLE void addDataToGraph(int graphIndex, double x, double y, bool force = false){emit m_plotWindow->getWidget()->addDataToGraphSignal(graphIndex, x, y, force);}

    ///This function returns several data points from a graph.
    Q_INVOKABLE QJSValue getDataFromGraph(int graphIndex, double xStart, int count = 1){return m_plotWindow->getWidget()->getDataFromGraph(graphIndex, xStart, count);}

    /**
     * Removes all data points with (sort-)keys between xFrom and xTo. If
     * xFrom is greater or equal to xTo, the function does nothing.
     */
    Q_INVOKABLE void removeDataRangeFromGraph(int graphIndex, double xFrom, double xTo, bool force = false){emit m_plotWindow->getWidget()->removeDataRangeFromGraphSignal(graphIndex, xFrom, xTo, force);}

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
     * This function sets the line style of a graph.
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

    ///This function sets the line width of a graph.
    Q_INVOKABLE void setLineWidth(int graphIndex, int width){emit m_plotWindow->getWidget()->setLineWidthSignal(graphIndex, width);}

    ///Sets the axis label.
    Q_INVOKABLE void setAxisLabels(QString xAxisLabel, QString yAxisLabel, QString yAxis2Label=""){emit m_plotWindow->getWidget()->setAxisLabelsSignal(xAxisLabel, yAxisLabel, yAxis2Label);}

    ///This function shows or hides the diagram legend.
    Q_INVOKABLE void showLegend(bool show){emit m_plotWindow->getWidget()->showLegendSignal(show);}

    ///This function clears the data of all graphs.
    Q_INVOKABLE void clearGraphs(void){emit m_plotWindow->getWidget()->clearGraphsSignal();}

    ///This function removes all graphs.
    Q_INVOKABLE void removeAllGraphs(void){emit m_plotWindow->getWidget()->removeAllGraphsSignal();}

    ///Sets the visibility of several plot widget elements.
    Q_INVOKABLE void showHelperElements(bool showXRange, bool showYRange, bool showUpdate, bool showSave, bool showLoad, bool showClear, bool showGraphVisibility,
                                        quint32 graphVisibilityMaxSize=100, bool showLegend=true)
    {emit m_plotWindow->getWidget()->showHelperElementsSignal(showXRange, showYRange, showUpdate, showSave, showLoad, showClear, showGraphVisibility, graphVisibilityMaxSize, showLegend);}

    ///Sets tThe max. number of data points per graph (the default is 10.000.000).
    Q_INVOKABLE void setMaxDataPointsPerGraph(qint32 maxDataPointsPerGraph){m_plotWindow->getWidget()->setMaxDataPointsPerGraph(maxDataPointsPerGraph);}

    ///Sets the automatic update enabled state.
    Q_INVOKABLE void setAutoUpdateEnabled(bool enabled){ m_plotWindow->getWidget()->setAutoUpdateEnabled(enabled);}

    ///Gets the automatic update enabled state.
    Q_INVOKABLE bool isAutoUpdateEnabled(void){return m_plotWindow->getWidget()->isAutoUpdateEnabled();}

    ///Sets the update-interval.
    Q_INVOKABLE void setUpdateInterval(quint32 updateInterval){m_plotWindow->getWidget()->setUpdateInterval(updateInterval);}

    ///Update the current plot view.
    Q_INVOKABLE void updatePlot(void){emit m_plotWindow->getWidget()->updatePlotSignal();}

    Q_INVOKABLE bool saveAllGraphs(QString fileName){bool hasSucceeded;m_plotWindow->getWidget()->saveAllGraphsSignal(fileName, &hasSucceeded);return hasSucceeded;}

    ///Sets the locale of the script plot window (QLocale::Language, QLocale::Country).
    Q_INVOKABLE void setLocale(int language, int country){emit m_plotWindow->getWidget()->setLocaleSignal(language, country);}

    ///If called the x values are interpreted as seconds (the decimal places are the milliseconds) that have passed since
    ///1970-01-01T00:00:00.000, Coordinated Universal Time (and the corresponding date time is shown).
    ///See QDateTime::toString for more details on the format string.
    Q_INVOKABLE void showDateTimeAtXAxis(QString format){emit m_plotWindow->getWidget()->showDateTimeAtXAxisSignal(format);}

Q_SIGNALS:

    ///Is emitted if the user press a mouse button inside the plot.
    ///Note: mouseButton has the type Qt::MouseButton.
    ///Scripts can connect to this signal.
    void plotMousePressSignal(double xValue, double yValue, quint32 mouseButton);

    ///Is emitted if the user clicks the clear button.
    ///Scripts can connect to this signal.
    void clearButtonPressedSignal(void);

    ///Is emitted if the user changes the x-range textedit.
    ///Scripts can connect to this signal.
    void xRangeChangedSignal(double newValue);

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

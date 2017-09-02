#ifndef SCRIPTPLOTWIDGET_H
#define SCRIPTPLOTWIDGET_H

#include <QObject>
#include "qcustomplot.h"
#include "scriptThread.h"


class ScriptWindow;


///Struct which represents one point in a graph.
typedef struct
{
    double x;
    double y;
}PlotPoint;


///Script plot widget.
class ScriptPlotWidget : public QObject, public ScriptObject
{
    Q_OBJECT
    Q_PROPERTY(QString publicScriptElements READ getPublicScriptElements)
public:
    explicit ScriptPlotWidget(ScriptThread* scriptThread, ScriptWindow* scriptWindow, QHBoxLayout* hLayout );
    ~ScriptPlotWidget();

    ///Returns a semicolon separated list with all public functions, signals and properties.
    virtual QString getPublicScriptElements(void)
    {
        return MainWindow::parseApiFile("ScriptPlotWidget.api");
    }

    /**
    * This function adds a graph to the diagram.
    * @param color
    *      The color of the graph. Allowed values are:
    *      blue, red, yellow, green, black.
    * @param penStyle
    *      The pen style of the graph. Allowed values are:
    *      dash, dot and solid.
    * @param name
    *      The name of the graph.
    * @graphIndex
    *     The index off the added graph.
    */
    Q_INVOKABLE int addGraph(QString color, QString penStyle, QString name)
    {
        int graphIndex = 0;
        emit addGraphSignal(color, penStyle, name, &graphIndex);
        return graphIndex;
    }

    ///The function sets the initial ranges of the diagram.
    Q_INVOKABLE void setInitialAxisRanges(double xRange, double yMinValue, double ymaxValue){emit setInitialAxisRangesSignal(xRange, yMinValue, ymaxValue);}

    ///The function sets the current ranges of the diagram.
    Q_INVOKABLE void setCurrentAxisRanges(double xMinValue, double xMaxValue, double yMinValue, double yMaxValue);

    ///The function gets the current ranges of the diagram.
    Q_INVOKABLE QScriptValue getCurrentAxisRanges(void);

    ///This function sets the line style of a graph.
    Q_INVOKABLE void addDataToGraph(int graphIndex, double x, double y){emit addDataToGraphSignal(graphIndex, x, y);}


    /**
     * Removes all data points with (sort-)keys between xFrom and xTo. If
     * xFrom is greater or equal to xTo, the function does nothing.
     */
    Q_INVOKABLE void removeDataRangeFromGraph(int graphIndex, double xFrom, double xTo){emit removeDataRangeFromGraphSignal(graphIndex, xFrom, xTo);}

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
    Q_INVOKABLE void setScatterStyle(int graphIndex, QString style, double size){emit setScatterStyleSignal(graphIndex, style, size);}

    /**
     * This function adds one point to a graph.
     * Possible values for style:
     *      - None: Data points are not connected with any lines (e.g. data only represented
     *              with symbols according to the scatter style (is set with setScatterStyle)).
     *      - Line: Data points are connected by a straight line.
     *      - StepLeft: Line is drawn as steps where the step height is the value of the left data point.
     *      - StepRight: Line is drawn as steps where the step height is the value of the right data point.
     *      - StepCenter: Line is drawn as steps where the step is in between two data points.
     *      - Impulse: Each data point is represented by a line parallel to the value axis, which reaches from the data point to the zero-value-line.
     */
    Q_INVOKABLE void setLineStyle(int graphIndex, QString style){emit setLineStyleSignal(graphIndex, style);}

    ///Sets the axis label.
    Q_INVOKABLE void setAxisLabels(QString xAxisLabel, QString yAxisLabel){emit setAxisLabelsSignal(xAxisLabel, yAxisLabel);}

    ///This function shows or hides the diagram legend.
    Q_INVOKABLE void showLegend(bool show){emit showLegendSignal(show);}

    ///This function clears the data of all graphs.
    Q_INVOKABLE void clearGraphs(void){emit clearGraphsSignal();}

    ///This function removes all graphs.
    Q_INVOKABLE void removeAllGraphs(void){emit removeAllGraphsSignal();}


    ///Sets the visibility of several plot widget elements.
    Q_INVOKABLE void showHelperElements(bool showXRange, bool showYRange, bool showUpdate, bool showSave,
                                        bool showLoad, bool showClear, bool showGraphVisibility, quint32 graphVisibilityMaxSize=80, bool showLegend=true)
    {emit showHelperElementsSignal(showXRange, showYRange, showUpdate, showSave, showLoad, showClear, showGraphVisibility, graphVisibilityMaxSize, showLegend);}

    ///Sets tThe max. number of data points per graph (the default is 10.000.000.).
    Q_INVOKABLE void setMaxDataPointsPerGraph(qint32 maxDataPointsPerGraph){m_maxDataPointsPerGraph = maxDataPointsPerGraph;}

    ///Sets the automatic update enabled state.
    Q_INVOKABLE void setAutoUpdate(bool enabled){m_updatePlotCheckBox->setChecked(enabled);}

    ///Gets the automatic update enabled state.
    Q_INVOKABLE bool autoUpdate(void){return m_updatePlotCheckBox->isChecked();}

    ///Sets the update-interval.
    Q_INVOKABLE void setUpdateInterval(quint32 updateInterval){emit setUpdateIntervalSignal(updateInterval);}

    ///Update the current plot view.
    Q_INVOKABLE void updatePlot(void){m_plotWidget->replot();}

    ///The default value for the plot update timer (m_plotTimer).
    static const quint32 DEFAULT_PLOT_UPDATE_TIME_MS = 100;

signals:


    ///Is emitted if the user press a mouse button inside the plot.
    ///Note: mouseButton has the type Qt::MouseButton.
    ///Scripts can connect to this signal.
    void plotMousePressSignal(double xValue, double yValue, quint32 mouseButton);

    ///Is emitted if the user clicks the clear button.
    ///Scripts can connect to this signal.
    void clearButtonPressedSignal(void);

    ///Is emitted in clearGraphs.
    ///This signal is private and must not be used inside a script.
    void clearGraphsSignal(void);

    ///Is emitted in clearGraphs.
    ///This signal is private and must not be used inside a script.
    void removeAllGraphsSignal(void);

    ///Is connected with PlotWindow::addGraphSlot (adds a graph).
    ///This signal is private and must not be used inside a script.
    void addGraphSignal(QString color, QString penStyle, QString name, int* graphIndex);

    ///Is connected with PlotWindow::setInitialAxisRangesSlot (sets the ranges of the diagram).
    ///This signal is private and must not be used inside a script.
    void setInitialAxisRangesSignal(double xRange, double yMinValue, double ymaxValue);

    ///Is connected with PlotWindow::addDataToGraphSlot (adds one point to a given specific graph).
    ///This signal is private and must not be used inside a script.
    void addDataToGraphSignal(int graphIndex, double x, double y);

    ///Is connected with PlotWindow::removeDataRangeFromGraphSlot (removes all data points with (sort-)keys between xFrom and xTo).
    ///This signal is private and must not be used inside a script.
    void removeDataRangeFromGraphSignal(int graphIndex, double xFrom, double xTo);

    ///Is connected with PlotWindow::setScatterStyleSlot (sets the visual appearance of single data points in the plot).
    ///This signal is private and must not be used inside a script.
    void setScatterStyleSignal(int graphIndex, QString style, double size);

    ///Is connected with PlotWindow::setLineStyleSlot(sets the line style of a graph).
    ///This signal is private and must not be used inside a script.
    void setLineStyleSignal(int graphIndex, QString style);

    ///Is connected with PlotWindow::showFromScriptSlot (Sets the axis labels).
    ///This signal is private and must not be used inside a script.
    void setAxisLabelsSignal(QString xAxisLabel, QString yAxisLabel);

    ///Is connected with PlotWindow::showLegendSlot (shows or hides the plot legend).
    ///This signal is private and must not be used inside a script.
    void showLegendSignal(bool show);


    ///Is connected with ScriptWindow::appendTextToConsoleSlot (appends text to the console in the script window).
    ///This is an internal signal which is only used by the plot window.
    void appendTextToConsole(QString text);

    ///Is emitted in showHelperElements.
    ///This signal is private and must not be used inside a script.
    void showHelperElementsSignal(bool showXRange, bool showYRange, bool showUpdate, bool showSave, bool showLoad,
                                  bool showClear, bool showGraphVisibility, quint32 graphVisibilityMaxSize, bool showLegend);

    ///Is emitted in setUpdateInterval.
    ///This signal is private and must not be used inside a script.
    void setUpdateIntervalSignal(quint32 updateInterval);

public slots:


    ///Sets the update-interval.
    void setUpdateIntervalSlot(quint32 updateInterval){if(updateInterval > 0){m_plotTimer->setInterval(updateInterval);}}

    ///Is called if the state of the sript thread (to which this widget belong to) has been changed.
    void threadStateChangedSlot(ThreadSate state, ScriptThread* thread);

    ///This function adds a graph to the diagram.
    void addGraphSlot(QString color, QString penStyle, QString name, int* graphIndex);

    ///The slot function sets the initial ranges of the diagram.
    void setInitialAxisRangesSlot(double xRange, double yMinValue, double ymaxValue);

    ///This slot function adds one point to a graph.
    bool addDataToGraphSlot(int graphIndex, double x, double y);

    ///Removes all data points with (sort-)keys between xFrom and xTo.
    void removeDataRangeFromGraphSlot(int graphIndex, double xFrom, double xTo);

    ///Sets the visual appearance of single data points in the plot.
    void setScatterStyleSlot(int graphIndex, QString style, double size);

    ///This slot function sets the line style of a graph.
    void setLineStyleSlot(int graphIndex, QString style);

    ///Sets the axis labels.
    void setAxisLabelsSlot(QString xAxisLabel, QString yAxisLabel);

    ///This function clears the data of all graphs.
    void clearGraphsSlot(void);

    ///This function removes all graphs.
    void removeAllGraphsSlot(void);

    ///Sets the visibility of several plot widget elements.
    void showHelperElementsSlot(bool showXRange, bool showYRange, bool showUpdate, bool showSave,  bool showLoad, bool showClear, bool showGraphVisibility,
                                quint32 graphVisibilityMaxSize, bool showLegend);

private slots:

    ///This function adjusts the borders of diagram and replots all graphs.
    ///It is called periodically by m_plotTimer.
    void plotTimeoutSlot(void);

    ///This function enables or disables the save button (the diagram can only be
    ///saved if it is not updated).
    void updateCheckBoxSlot(int state);

    ///This function enables or disables the legend.
    void showLegendCheckBoxSlot(int state);

    ///This function is called if the user clicks a visibility checkbox.
    void visibiltyCheckBoxSlot(int state);

    ///This function shows or hides the diagram legend (true=show, false=hide).
    void showLegendSlot(bool show);

    ///This function saves the current displayed graphs.
    ///It is called if the save button is pressed.
    void saveButtonPressed();

    ///This function loads saved graphes.
    ///It is called if the load button is pressed.
    void loadButtonPressed();

    ///This function clears the plot widget.
    ///It is called if the clear button is pressed.
    void clearButtonPressedSlot();

    ///Is called if the value of a double line dit has been changed.
    ///This function replaces all ',' with '.'.
    void doubleLineEditChangedSlot(QString text);

    ///Is called if the user press a mouse button inside the plot.
    void plotMousePressSlot(QMouseEvent *event);

private:

    ///Pointer to the script thread.
    ScriptThread* m_scriptThread;

    ///This periodically timer calls the function plotTimeoutSlot.
    QTimer* m_plotTimer;

    ///In this vector all max. xAxis values are saved (one per graph).
    std::vector<double> m_xAxisMaxValues;

    ///In this vector all points which are added with addDataToGraphSlot during the update check box
    ///is not checked (plot window freeze) are saved (one vector per graph). After the update check box is checked again, all
    ///value are added with addDataToGraphSlot.
    std::vector<std::vector<PlotPoint>*> m_savePointDuringPlotFreeze;

    ///The plot area.
    QCustomPlot* m_plotWidget;

    ///The x range input field.
    QLineEdit* m_xRangeLineEdit;

    ///The x range input field label.
    QLabel* m_xRangeLabel;

    ///The min y range input field.
    QLineEdit* m_yMinRangeLineEdit;

    ///The min y range input field label.
    QLabel* m_yMinRangeLabel;

    ///The max y range input field.
    QLineEdit* m_yMaxRangeLineEdit;

    ///The max y range input field label.
    QLabel* m_yMaxRangeLabel;

    ///The update plot check box.
    QCheckBox* m_updatePlotCheckBox;

    ///The show legend check box.
    QCheckBox* m_showLegendCheckBox;

    ///The load button.
    QPushButton* m_loadPushButton;

    ///The save button.
    QPushButton* m_savePushButton;

    ///The clear button.
    QPushButton* m_clearPushButton;

    ///The layout of the group box in which the plot widget resides.
    QVBoxLayout* m_grouBoxLayout;

    ///The group box in which the plot widget resides.
    QGroupBox* m_groupBox;

    ///The visibility check boxes (one per graph).
    QVector<QCheckBox*> m_visibilityCheckBoxes;

    ///The max. number of data points per graph.
    qint32 m_maxDataPointsPerGraph;
};


#endif // SCRIPTPLOTWIDGET_H

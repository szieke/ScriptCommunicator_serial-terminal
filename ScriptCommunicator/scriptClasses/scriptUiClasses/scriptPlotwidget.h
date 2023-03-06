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

///Contains a saved operation.
typedef struct
{
 double value1;
 double value2;
 bool isAdded;//True if the saved opertaion is an add data point and false if the save operation is a delete range.
}SavedPlotOperation;


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
    * @param graphIndex
    *     The index off the added graph.
    * @param useYAxis2
    *     True if y axis 2 shall be used for this graph
    */
    Q_INVOKABLE int addGraph(QString color, QString penStyle, QString name, bool useYAxis2 = false)
    {
        int graphIndex = 0;
        emit addGraphSignal(color, penStyle, name, &graphIndex, useYAxis2);
        return graphIndex;
    }

    ///The function sets the initial ranges of the diagram.
    Q_INVOKABLE void setInitialAxisRanges(double xRange, double yMinValue, double yMaxValue, bool addSpaceAfterBiggestValues = true, double y2MinValue=0, double y2MaxValue=0)
    {emit setInitialAxisRangesSignal(xRange, yMinValue, yMaxValue, addSpaceAfterBiggestValues, y2MinValue, y2MaxValue);}

    ///The function sets the current ranges of the diagram.
    Q_INVOKABLE void setCurrentAxisRanges(double xMinValue, double xMaxValue, double yMinValue, double yMaxValue, double y2MinValue=0, double y2MaxValue=0);

    ///The function gets the current ranges of the diagram.
    Q_INVOKABLE QJSValue getCurrentAxisRanges(void);

    ///This function adds one point to a graph.
    Q_INVOKABLE void addDataToGraph(int graphIndex, double x, double y, bool force = false){emit addDataToGraphSignal(graphIndex, x, y, force);}

    ///This function returns several data points from a graph.
    Q_INVOKABLE QJSValue getDataFromGraph(int graphIndex, double xStart, int count = 1);

    /**
     * Removes all data points with (sort-)keys between xFrom and xTo. If
     * xFrom is greater or equal to xTo, the function does nothing.
     */
    Q_INVOKABLE void removeDataRangeFromGraph(int graphIndex, double xFrom, double xTo, bool force = false){emit removeDataRangeFromGraphSignal(graphIndex, xFrom, xTo, force);}

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
     * This function sets the line style of a graph.
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

    ///This function sets the line width of a graph.
    Q_INVOKABLE void setLineWidth(int graphIndex, int width){emit setLineWidthSignal(graphIndex, width);}

    ///Sets the axis label.
    Q_INVOKABLE void setAxisLabels(QString xAxisLabel, QString yAxisLabel, QString yAxis2Label=""){emit setAxisLabelsSignal(xAxisLabel, yAxisLabel, yAxis2Label);}

    ///This function shows or hides the diagram legend.
    Q_INVOKABLE void showLegend(bool show){emit showLegendSignal(show);}

    ///This function clears the data of all graphs.
    Q_INVOKABLE void clearGraphs(void){emit clearGraphsSignal();}

    ///This function removes all graphs.
    Q_INVOKABLE void removeAllGraphs(void){emit removeAllGraphsSignal();}


    ///Sets the visibility of several plot widget elements.
    Q_INVOKABLE void showHelperElements(bool showXRange, bool showYRange, bool showUpdate, bool showSave,
                                        bool showLoad, bool showClear, bool showGraphVisibility, quint32 graphVisibilityMaxSize=100, bool showLegend=true)
    {emit showHelperElementsSignal(showXRange, showYRange, showUpdate, showSave, showLoad, showClear, showGraphVisibility, graphVisibilityMaxSize, showLegend);}

    ///Sets tThe max. number of data points per graph (the default is 10.000.000.).
    Q_INVOKABLE void setMaxDataPointsPerGraph(qint32 maxDataPointsPerGraph){m_maxDataPointsPerGraph = maxDataPointsPerGraph;}

    ///Sets the automatic update enabled state.
    Q_INVOKABLE void setAutoUpdateEnabled(bool enabled){m_updatePlotCheckBox->setChecked(enabled);}

    ///Gets the automatic update enabled state.
    Q_INVOKABLE bool isAutoUpdateEnabled(void){return m_updatePlotCheckBox->isChecked();}

    ///Sets the update-interval.
    Q_INVOKABLE void setUpdateInterval(quint32 updateInterval){emit setUpdateIntervalSignal(updateInterval);}

    ///Update the current plot view.
    Q_INVOKABLE void updatePlot(void){emit updatePlotSignal();}

    Q_INVOKABLE bool saveAllGraphs(QString fileName){bool hasSucceed;emit saveAllGraphsSignal(fileName, &hasSucceed);return hasSucceed;}

    ///Sets the locale of the script plot widget (QLocale::Language, QLocale::Country).
    Q_INVOKABLE void setLocale(int language, int country){emit setLocaleSignal(language, country);}

    ///If called the x values are interpreted as seconds (the decimal places are the milliseconds) that have passed since
    ///1970-01-01T00:00:00.000, Coordinated Universal Time (and the corresponding date time is shown).
    ///See QDateTime::toString for more details on the format string.
    Q_INVOKABLE void showDateTimeAtXAxis(QString format){emit showDateTimeAtXAxisSignal(format);}


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

    ///Is emitted if the user changes the x-range textedit.
    ///Scripts can connect to this signal.
    void xRangeChangedSignal(double newValue);

    ///Is emitted in clearGraphs.
    ///This signal is private and must not be used inside a script.
    void clearGraphsSignal(void);

    ///Is emitted in clearGraphs.
    ///This signal is private and must not be used inside a script.
    void removeAllGraphsSignal(void);

    ///Is connected with PlotWindow::addGraphSlot (adds a graph).
    ///This signal is private and must not be used inside a script.
    void addGraphSignal(QString color, QString penStyle, QString name, int* graphIndex, bool useYAxis2);

    ///Is connected with PlotWindow::setInitialAxisRangesSlot (sets the ranges of the diagram).
    ///This signal is private and must not be used inside a script.
    void setInitialAxisRangesSignal(double xRange, double yMinValue, double ymaxValue, bool addSpaceAfterBiggestValues, double y2MinValue, double y2maxValue);

    ///Is connected with PlotWindow::addDataToGraphSlot (adds one point to a given specific graph).
    ///This signal is private and must not be used inside a script.
    void addDataToGraphSignal(int graphIndex, double x, double y, bool force);

    ///Is connected with PlotWindow::removeDataRangeFromGraphSlot (removes all data points with (sort-)keys between xFrom and xTo).
    ///This signal is private and must not be used inside a script.
    void removeDataRangeFromGraphSignal(int graphIndex, double xFrom, double xTo, bool force);

    ///Is connected with PlotWindow::setScatterStyleSlot (sets the visual appearance of single data points in the plot).
    ///This signal is private and must not be used inside a script.
    void setScatterStyleSignal(int graphIndex, QString style, double size);

    ///Is connected with PlotWindow::setLineStyleSlot(sets the line style of a graph).
    ///This signal is private and must not be used inside a script.
    void setLineStyleSignal(int graphIndex, QString style);

    ///Is connected with PlotWindow::setLineWidthSlot(sets the line width of a graph).
    ///This signal is private and must not be used inside a script.
    void setLineWidthSignal(int graphIndex, int width);

    ///Is connected with PlotWindow::showFromScriptSlot (Sets the axis labels).
    ///This signal is private and must not be used inside a script.
    void setAxisLabelsSignal(QString xAxisLabel, QString yAxisLabe, QString yAxis2Label);

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

    ///Is emitted in updatePlot();
    ///This signal is private and must not be used inside a script.
    void updatePlotSignal();

    ///Is emitted in saveAllGraphs;
    ///This signal is private and must not be used inside a script.
    void saveAllGraphsSignal(QString fileName, bool *hasSucceed);

    ///Is emitted in setLocale;
    ///This signal is private and must not be used inside a script.
    void setLocaleSignal(int language, int country);

    ///Is emitted in showDateTimeAtXAxis;
    ///This signal is private and must not be used inside a script.
    void showDateTimeAtXAxisSignal(QString format);

public slots:


    ///Sets the update-interval.
    void setUpdateIntervalSlot(quint32 updateInterval){if(updateInterval > 0){m_plotTimer->setInterval(updateInterval);}}

    ///Is called if the state of the sript thread (to which this widget belong to) has been changed.
    void threadStateChangedSlot(ThreadSate state, ScriptThread* thread);

    ///This function adds a graph to the diagram.
    void addGraphSlot(QString color, QString penStyle, QString name, int* graphIndex, bool useYAxis2);

    ///The slot function sets the initial ranges of the diagram.
    void setInitialAxisRangesSlot(double xRange, double yMinValue, double ymaxValue, bool addSpaceAfterBiggestValues, double y2MinValue, double y2MaxValue);

    ///This slot function adds one point to a graph.
    bool addDataToGraphSlot(int graphIndex, double x, double y, bool force=false);

    ///Removes all data points with (sort-)keys between xFrom and xTo.
    void removeDataRangeFromGraphSlot(int graphIndex, double xFrom, double xTo, bool force=false);

    ///Sets the visual appearance of single data points in the plot.
    void setScatterStyleSlot(int graphIndex, QString style, double size);

    ///This slot function sets the line style of a graph.
    void setLineStyleSlot(int graphIndex, QString style);

    ///This slot function sets the line width of a graph.
    void setLineWidthSlot(int graphIndex, int width);

    ///Sets the axis labels.
    void setAxisLabelsSlot(QString xAxisLabel, QString yAxisLabel, QString yAxis2Label);

    ///This function clears the data of all graphs.
    void clearGraphsSlot(void);

    ///This function removes all graphs.
    void removeAllGraphsSlot(void);

    ///Sets the visibility of several plot widget elements.
    void showHelperElementsSlot(bool showXRange, bool showYRange, bool showUpdate, bool showSave,  bool showLoad, bool showClear, bool showGraphVisibility,
                                quint32 graphVisibilityMaxSize, bool showLegend);


    ///This function saves all displayed graphs to a file.
    void saveAllGraphsSlot(QString fileName, bool *hasSucceed);

    ///If called the x values are interpreted as milliseconds that have passed since
    ///1970-01-01T00:00:00.000, Coordinated Universal Time (and the corresponding date time is shown).
    void showDateTimeAtXAxisSlot(QString format);

    ///Sets the locale of the script plot widget.
    void setLocaleSlot(int language, int country);

private slots:

    ///This function is called periodically by m_plotTimer (adjusts the borders of diagram and replots all graphs).
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

    ///This function adjusts the borders of diagram and replots all graphs.
    void adjustBordersAndReplot(void);

    ///Pointer to the script thread.
    ScriptThread* m_scriptThread;

    ///This periodically timer calls the function plotTimeoutSlot.
    QTimer* m_plotTimer;

    ///In this vector all max. xAxis values are saved (one per graph).
    std::vector<double> m_xAxisMaxValues;

    ///In this vector all operations are saved while the update check box is not checked (plot window freeze, one vector per graph). After the update check box is checked again, all
    ///saved operations are performed.
    std::vector<std::vector<SavedPlotOperation>*> m_savedOperationsDuringPlotFreeze;

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

    ///The min y range input field.
    QLineEdit* m_y2MinRangeLineEdit;

    ///The min y 2 range input field label.
    QLabel* m_y2MinRangeLabel;

    ///The max y 2 range input field.
    QLineEdit* m_y2MaxRangeLineEdit;

    ///The max y 2 range input field label.
    QLabel* m_y2MaxRangeLabel;

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

    ///True if a space shall be added after the biggest value of a graph.
    bool m_addSpaceAfterBiggestValues;

    ///True if y axis is visible.
    bool m_yAxis2IsVisible;

    ///True if the y ranger helper elements are shown.
    bool m_yRangeHelperVisible;
};


#endif // SCRIPTPLOTWIDGET_H

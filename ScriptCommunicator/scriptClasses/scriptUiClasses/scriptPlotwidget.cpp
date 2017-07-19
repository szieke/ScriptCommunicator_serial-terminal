#include "scriptPlotwidget.h"
#include "scriptwindow.h"


/**
 * Konstruktur
 * @param scriptWindow
 *      The script widget.
 * @param hLayout
 *      The layout of the group box in which the plot widget resides.
 */
ScriptPlotWidget::ScriptPlotWidget(ScriptThread* scriptThread, ScriptWindow *scriptWindow, QHBoxLayout *hLayout) :
    QObject(0), m_scriptThread(scriptThread), m_maxDataPointsPerGraph(10000000)
{

    Qt::ConnectionType directConnectionType = m_scriptThread->runsInDebugger() ? Qt::DirectConnection : Qt::BlockingQueuedConnection ;

    m_plotWidget = new QCustomPlot();
    hLayout->addWidget(m_plotWidget);


    QVBoxLayout* vLayout = new QVBoxLayout();
    hLayout->addLayout(vLayout);

    m_xRangeLabel = new QLabel("x Range",m_plotWidget);
    vLayout->addWidget(m_xRangeLabel);
    //Create and add the x range line edit.
    m_xRangeLineEdit = new QLineEdit(m_plotWidget);
    m_xRangeLineEdit->setValidator(new QDoubleValidator(INT_MIN, INT_MAX, 20, m_xRangeLineEdit));
    m_xRangeLineEdit->setMaximumWidth(80);
    vLayout->addWidget(m_xRangeLineEdit);

    m_yMinRangeLabel = new QLabel("y min Range",m_plotWidget);
    vLayout->addWidget(m_yMinRangeLabel);
    //Create and add the y min range line edit.
    m_yMinRangeLineEdit = new QLineEdit(m_plotWidget);
    m_yMinRangeLineEdit->setValidator(new QDoubleValidator(INT_MIN, INT_MAX, 20, m_yMinRangeLineEdit));
    m_yMinRangeLineEdit->setMaximumWidth(80);
    vLayout->addWidget(m_yMinRangeLineEdit);

    m_yMaxRangeLabel = new QLabel("y max Range",m_plotWidget);
    vLayout->addWidget(m_yMaxRangeLabel);
    //Create and add the y max range line edit.
    m_yMaxRangeLineEdit = new QLineEdit(m_plotWidget);
    m_yMaxRangeLineEdit->setValidator(new QDoubleValidator(INT_MIN, INT_MAX, 20, m_yMaxRangeLineEdit));
    m_yMaxRangeLineEdit->setMaximumWidth(80);
    vLayout->addWidget(m_yMaxRangeLineEdit);

    //Create and add the update check box.
    m_updatePlotCheckBox = new QCheckBox(m_plotWidget);
    m_updatePlotCheckBox->setChecked(true);
    m_updatePlotCheckBox->setText("update");
    vLayout->addWidget(m_updatePlotCheckBox);

    //Create and add the show legend check box.
    m_showLegendCheckBox = new QCheckBox(m_plotWidget);
    m_showLegendCheckBox->setChecked(true);
    m_showLegendCheckBox->setText("show legend");
    vLayout->addWidget(m_showLegendCheckBox);



    m_savePushButton = new QPushButton(m_plotWidget);
    m_savePushButton->setText("save");
    m_savePushButton->setEnabled(false);
    m_savePushButton->setMaximumWidth(80);
    vLayout->addWidget(m_savePushButton);

    m_loadPushButton = new QPushButton(m_plotWidget);
    m_loadPushButton->setText("load");
    m_loadPushButton->setEnabled(true);
    m_loadPushButton->setMaximumWidth(80);
    vLayout->addWidget(m_loadPushButton);

    m_clearPushButton = new QPushButton(m_plotWidget);
    m_clearPushButton->setText("clear");
    m_clearPushButton->setMaximumWidth(80);
    vLayout->addWidget(m_clearPushButton);

    m_groupBox = new QGroupBox("show", m_plotWidget);
    m_grouBoxLayout = new QVBoxLayout(m_plotWidget);
    m_groupBox->setLayout(m_grouBoxLayout);
    vLayout->addWidget(m_groupBox);



    vLayout->addStretch(1);
    hLayout->setStretch(0, 1);
    hLayout->setStretch(1, 0);

    connect(this, SIGNAL(appendTextToConsole(QString)),
            scriptWindow, SLOT(appendTextToConsoleSlot(QString)), Qt::QueuedConnection);

    //configure right and top axis to show ticks but no labels
    m_plotWidget->xAxis2->setVisible(true);
    m_plotWidget->xAxis2->setTickLabels(false);
    m_plotWidget->yAxis2->setVisible(true);
    m_plotWidget->yAxis2->setTickLabels(false);


    //make left and bottom axes always transfer their ranges to right and top axes
    connect(m_plotWidget->xAxis, SIGNAL(rangeChanged(QCPRange)), m_plotWidget->xAxis2, SLOT(setRange(QCPRange)));
    connect(m_plotWidget->yAxis, SIGNAL(rangeChanged(QCPRange)), m_plotWidget->yAxis2, SLOT(setRange(QCPRange)));

    //allow user to drag axis ranges with mouse, zoom with mouse wheel and select graphs by clicking
    m_plotWidget->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);


    connect(m_updatePlotCheckBox, SIGNAL(stateChanged(int)), this, SLOT(updateCheckBoxSlot(int)));
    connect(m_showLegendCheckBox, SIGNAL(stateChanged(int)), this, SLOT(showLegendCheckBoxSlot(int)));
    connect(m_savePushButton, SIGNAL(clicked()), this, SLOT(saveButtonPressed()));
    connect(m_loadPushButton, SIGNAL(clicked()), this, SLOT(loadButtonPressed()));
    connect(m_clearPushButton, SIGNAL(clicked()), this, SLOT(clearButtonPressedSlot()));


    connect(this, SIGNAL(addGraphSignal(QString, QString, QString, int*)),
            this, SLOT(addGraphSlot(QString, QString, QString, int*)), directConnectionType);

    connect(this, SIGNAL(setInitialAxisRangesSignal(double, double, double)),
            this, SLOT(setInitialAxisRangesSlot(double, double, double)), Qt::QueuedConnection);

    connect(this, SIGNAL(addDataToGraphSignal(int, double, double)),
            this, SLOT(addDataToGraphSlot(int, double, double)), Qt::QueuedConnection);

    connect(this, SIGNAL(setScatterStyleSignal(int,QString,double)),
            this, SLOT(setScatterStyleSlot(int,QString,double)), Qt::QueuedConnection);

    connect(this, SIGNAL(setLineStyleSignal(int,QString)),
            this, SLOT(setLineStyleSlot(int,QString)), Qt::QueuedConnection);

    connect(this, SIGNAL(setAxisLabelsSignal(QString,QString)),
            this, SLOT(setAxisLabelsSlot(QString,QString)), Qt::QueuedConnection);

    connect(this, SIGNAL(showLegendSignal(bool)),
            this, SLOT(showLegendSlot(bool)), Qt::QueuedConnection);

    connect(this, SIGNAL(clearGraphsSignal()),
            this, SLOT(clearGraphsSlot()), directConnectionType);

    connect(this, SIGNAL(removeAllGraphsSignal()),
            this, SLOT(removeAllGraphsSlot()), directConnectionType);

    connect(this, SIGNAL(showHelperElementsSignal(bool,bool,bool,bool,bool,bool,bool, quint32,bool)),
            this, SLOT(showHelperElementsSlot(bool,bool,bool,bool,bool,bool,bool, quint32,bool)), directConnectionType);

    connect(m_xRangeLineEdit, SIGNAL(textChanged(QString)),
            this, SLOT(doubleLineEditChangedSlot(QString)), Qt::QueuedConnection);

    connect(m_yMinRangeLineEdit, SIGNAL(textChanged(QString)),
            this, SLOT(doubleLineEditChangedSlot(QString)), Qt::QueuedConnection);

    connect(m_yMaxRangeLineEdit, SIGNAL(textChanged(QString)),
            this, SLOT(doubleLineEditChangedSlot(QString)), Qt::QueuedConnection);

    connect(m_plotWidget, SIGNAL(mousePress(QMouseEvent*)),
            this, SLOT(plotMousePressSlot(QMouseEvent*)), Qt::QueuedConnection);

    connect(this, SIGNAL(setUpdateIntervalSignal(quint32)),
            this, SLOT(setUpdateIntervalSlot(quint32)), Qt::QueuedConnection);

    m_plotTimer = new QTimer(this);
    connect(m_plotTimer, SIGNAL(timeout()), this, SLOT(plotTimeoutSlot()));
    m_plotTimer->start(DEFAULT_PLOT_UPDATE_TIME_MS);


    connect(m_scriptThread, SIGNAL(threadStateChangedSignal(ThreadSate,ScriptThread*)),
            this, SLOT(threadStateChangedSlot(ThreadSate,ScriptThread*)), Qt::QueuedConnection);
}

/**
 * Destruktor.
 */
ScriptPlotWidget::~ScriptPlotWidget()
{
    ///delete all vector vectors in the freeze vector
    for(uint i = 0; i < m_savePointDuringPlotFreeze.size(); i++)
    {
        delete m_savePointDuringPlotFreeze[i];
    }

    m_plotWidget->deleteLater();
}


/**
 * Is called if the state of the sript thread (to which this widget belong to) has been changed.
 * @param state
 *      The new script state.
 * @param thread
 *      The script thread.
 */
void ScriptPlotWidget::threadStateChangedSlot(ThreadSate state, ScriptThread* thread)
{

    (void)thread;

    if(state == PAUSED)
    {
        m_plotTimer->stop();
    }
    else
    {
        if(!m_plotTimer->isActive())
        {
            m_plotTimer->start();
        }
    }
}


/**
 * This function clears the data of all graphs.
 */
void ScriptPlotWidget::clearGraphsSlot(void)
{
    for (qint32 i = 0; i < m_plotWidget->graphCount(); i++)
    {
        m_plotWidget->graph(i)->clearData();
        m_xAxisMaxValues[i] = 0;
    }
}

/**
 * This function removes all graphs.
 */
void ScriptPlotWidget::removeAllGraphsSlot(void)
{
    m_plotWidget->clearGraphs();
    m_xAxisMaxValues.clear();
    m_plotWidget->xAxis->setRange(m_xRangeLineEdit->text().toDouble(),  m_xRangeLineEdit->text().toDouble() / 10);
    m_plotWidget->yAxis->setRange(m_yMinRangeLineEdit->text().toDouble(), m_yMaxRangeLineEdit->text().toDouble());
    m_plotWidget->replot();

    for(auto el : m_visibilityCheckBoxes)
    {
        m_grouBoxLayout->removeWidget(el);
        el->deleteLater();
    }
    m_visibilityCheckBoxes.clear();
}

///Is called if the user press a mouse button inside the plot.
void ScriptPlotWidget::plotMousePressSlot(QMouseEvent *event)
{

    (void)event;
    double xValue = m_plotWidget->xAxis->pixelToCoord(event->x());
    double yValue = m_plotWidget->yAxis->pixelToCoord(event->y());

    //event->button() did not work on some PC's.
    Qt::MouseButtons button = QApplication::mouseButtons();

    emit plotMousePressSignal(xValue, yValue, static_cast<quint32>(button));

    return;
}

/**
 * Is called if the value of a double line dit has been changed.
 * This function replaces all ',' with '.'.
 * @param text
 *      The new text/value.
 */
void ScriptPlotWidget::doubleLineEditChangedSlot(QString text)
{
    QLineEdit* lineEdit = dynamic_cast<QLineEdit*>(sender());
    if( lineEdit != NULL )
    {
        if(text.contains(","))
        {//The text contains at least one ','.

            text.replace(",", ".");
            lineEdit->blockSignals(true);
            lineEdit->setText(text);
            lineEdit->blockSignals(false);
        }

        int index = text.indexOf(".", 0);
        if(index != -1)
        {//The string contains at least one '.'.

            index = text.indexOf(".", index + 1);
            if(index != -1)
            {//The string contains a second '.'.

                //Remove the second '.'.
                text.remove(index, 1);
                lineEdit->blockSignals(true);
                lineEdit->setText(text);
                lineEdit->blockSignals(false);
            }
        }

    }
}

/**
 * This function clears the plot widget.
 * It is called if the clear button is pressed.
 */
void ScriptPlotWidget::clearButtonPressedSlot()
{
    emit clearButtonPressedSignal();
}

/**
 * This function saves the current displayed graphs.
 * It is called if the save button is pressed.
 */
void ScriptPlotWidget::saveButtonPressed()
{

    QString selectedFilter;
    QString tmpFileName = QFileDialog::getSaveFileName(m_plotWidget, tr("save as image/csv"),
                                                      "",tr("PNG (*.png);;JPG (*.jpg);;BMP (*.bmp);;PDF (*.pdf);;CSV (*.csv)"), &selectedFilter);
    if(!tmpFileName.isEmpty())
    {
        QStringList tmpList = tmpFileName.split(".");
        QString suffix;

        if(tmpList.length() < 2)
        {//No suffix entered.

            suffix = selectedFilter.remove(0, selectedFilter.indexOf(".") + 1);
            suffix.remove(")");
            tmpFileName += "." + suffix;
        }
        else
        {
            suffix = tmpList[1];
        }

        if(suffix == "png")
        {
            m_plotWidget->savePng(tmpFileName);
        }
        else if(suffix == "jpg")
        {
            m_plotWidget->saveJpg(tmpFileName);
        }
        else if(suffix == "bmp")
        {
            m_plotWidget->saveBmp(tmpFileName);
        }
        else if(suffix == "pdf")
        {
            m_plotWidget->savePdf(tmpFileName);
        }
       else if(suffix == "csv")
        {
            QString content = "";

            for (int i = 0; i < m_plotWidget->graphCount(); i++)
            {
                if(m_plotWidget->graph(i)->visible())
                {
                    content += m_plotWidget->graph(i)->name() + ",";
                    content += QString("%1,").arg(m_plotWidget->graph(i)->pen().color().name());
                    content += QString("%1,").arg(m_plotWidget->graph(i)->pen().style());

                    const QCPDataMap* map = m_plotWidget->graph(i)->data();
                    QMap<double, QCPData>::const_iterator iter;
                    for (iter = map->begin(); iter != map->end(); ++iter)
                    {
                        content += QString("%1:%2,").arg(iter.key()).arg(iter.value().value);
                    }
                    //Remove the last ,
                    content.remove(content.length() - 1, 1);
                    content += "\n";
                }

            }

             m_scriptThread->writeFile(tmpFileName, false,content,true);

        }
        else
        {
            QMessageBox::critical(m_plotWidget, "invalid file-extension", QString("invalid file-extension: %1").arg("." + suffix));
        }
    }
}

/**
 * This function loads saved graphes.
 * It is called if the load button is pressed.
 */
void ScriptPlotWidget::loadButtonPressed()
{
    QString tmpFileName = QFileDialog::getOpenFileName(m_plotWidget, tr("load saved graphes"),
                                                       "", tr("CSV (*.csv);;Files (*.*)"));

    if(!tmpFileName.isEmpty())
    {
        //Remove all graphs.
        removeAllGraphsSlot();

        QString fileContent = m_scriptThread->readFile(tmpFileName, false);
        //The singles graphs are separated with a \n
        QStringList graphs = fileContent.split("\n");


        //The update check box must be checked for addDataToGraphSlot.
        bool updateCheckBoxState = m_updatePlotCheckBox->isChecked();
        m_updatePlotCheckBox->setChecked(true);

        for(qint32 i = 0; i < graphs.size(); i++)
        {
            //The single values are separated with ,.
            QStringList values = graphs[i].split(",");

            if(values.size() > 3)
            {
                int graphIndex = 0;
                addGraphSlot("blue", "solid", values[0], &graphIndex);

                QPen pen;
                pen.setColor(values[1]);
                pen.setStyle((Qt::PenStyle)values[2].toUInt());
                m_plotWidget->graph(graphIndex)->setPen(pen);


                for(qint32 index = 3; index < values.size(); index++)
                {
                    QStringList valuePair = values[index].split(":");
                    if(valuePair.size() == 2)
                    {
                        bool isOk = false;
                        addDataToGraphSlot(graphIndex, valuePair[0].toDouble(&isOk), valuePair[1].toDouble(&isOk));
                    }
                }
            }

        }

        plotTimeoutSlot();
        m_updatePlotCheckBox->setChecked(updateCheckBoxState);
    }

}

/**
 * This function shows or hides the diagram legend (true=show, false=hide).
 * @param show
 *      True for show and false for hiding the legend.
 */
void ScriptPlotWidget::showLegendSlot(bool show)
{
    m_plotWidget->legend->setVisible(show);
    m_showLegendCheckBox->setChecked(show);
}

/**
 * This function is called if the user clicks a visibility checkbox.
 * @param state
 *      The state of the check box (1== checked).
 */
void ScriptPlotWidget::visibiltyCheckBoxSlot(int state)
{
    (void)state;

    for(qint32 i = 0; i < m_visibilityCheckBoxes.size(); i++)
    {
        if(m_visibilityCheckBoxes[i]->isChecked())
        {
            m_plotWidget->graph(i)->setVisible(true);
        }
        else
        {
            m_plotWidget->graph(i)->setVisible(false);
        }
    }
    m_plotWidget->replot();

}


/**
 * This function enables or disables the legend.
 * @param state
 *      The state of the update check box (1== checked).
 */
void ScriptPlotWidget::showLegendCheckBoxSlot(int state)
{
    (void)state;
    m_plotWidget->legend->setVisible(m_showLegendCheckBox->isChecked());
}

/**
 * This function enables or disables the save button (the diagram can only be
 * saved if it is not updated).
 * @param state
 *      The state of the update check box (1== checked).
 */
void ScriptPlotWidget::updateCheckBoxSlot(int state)
{
    if(state != 0)
    {
        //add all saved points (during the plot window freeze)
        for(uint i = 0; i < m_savePointDuringPlotFreeze.size(); i++)
        {
            for(uint j = 0; j < m_savePointDuringPlotFreeze[i]->size(); j++)
            {
                addDataToGraphSlot(i, m_savePointDuringPlotFreeze[i]->at(j).x, m_savePointDuringPlotFreeze[i]->at(j).y);
            }
            m_savePointDuringPlotFreeze[i]->clear();
        }

        m_savePushButton->setEnabled(false);
    }
    else
    {
        m_savePushButton->setEnabled(true);
    }
}

/**
 * The slot function sets the initial ranges of the diagram.
 * @param xRange
 *      The range of the x axis (the x axis starts always with 0).
 * @param yMinValue
 *      The min. values of the y axis.
 * @param yMaxValue
 *      The max. value of the y axis.
 */
void ScriptPlotWidget::setInitialAxisRangesSlot(double xRange, double yMinValue, double yMaxValue)
{
    m_xRangeLineEdit->setText(QString("%1").arg(xRange));
    m_yMinRangeLineEdit->setText(QString("%1").arg(yMinValue));
    m_yMaxRangeLineEdit->setText(QString("%1").arg(yMaxValue));
    m_plotWidget->yAxis->setRange(yMinValue, yMaxValue);
    m_plotWidget->xAxis->setRange(0, xRange);
}

/**
 * Sets the axis labels.
 * @param xAxisLabel
 *      The label for the x axis.
 * @param yAxisLabel
 *      The label for the y axis.
 */
void ScriptPlotWidget::setAxisLabelsSlot(QString xAxisLabel, QString yAxisLabel)
{
    m_plotWidget->xAxis->setLabel(xAxisLabel);
    m_plotWidget->yAxis->setLabel(yAxisLabel);
}


/**
 * This slot function adds one point to a graph.
 * @param graphIndex
 *      The graph index.
 * @param x
 *      The x value of the point.
 * @param y
 *      The y value of the point
 * @return
 *      True for success.
 */
bool ScriptPlotWidget::addDataToGraphSlot(int graphIndex, double x, double y)
{
    bool hasSucceeded = true;

    if(m_updatePlotCheckBox->isChecked())
    {
        if (graphIndex >= 0 && graphIndex < m_plotWidget->graphCount())
        {
            m_plotWidget->graph(graphIndex)->addData(x,y);

            if(x > m_xAxisMaxValues[graphIndex])
            {
                m_xAxisMaxValues[graphIndex] = x;
                m_plotWidget->xAxis->setRange(x - m_xRangeLineEdit->text().toDouble(), x+ m_xRangeLineEdit->text().toDouble() / 10);
            }
        }
        else
        {//invalid index
            emit appendTextToConsole(Q_FUNC_INFO + QString("graph index is out of bounds:%1").arg(graphIndex));
            hasSucceeded = false;

        }
    }
    else
    {//the plot window is freezed

        //save the point in the freeze array
        PlotPoint point;
        point.x = x;
        point.y = y;
        m_savePointDuringPlotFreeze[graphIndex]->push_back(point);
    }

    return hasSucceeded;
}

/**
 * Sets the visual appearance of single data points in the plot.
 *
 * @param graphIndex
 *      The graph index.
 * @param style
 *      The style of the single data points. Possible values:
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
 * @param size
 *      The size of the single data points.
 */
void ScriptPlotWidget::setScatterStyleSlot(int graphIndex, QString style, double size)
{
    QCPScatterStyle::ScatterShape scatterStyle = QCPScatterStyle::ssNone;

    if (graphIndex >= 0 && graphIndex < m_plotWidget->graphCount())
    {
        if(style == "None")
        {
            scatterStyle = QCPScatterStyle::ssNone;
        }
        else if(style == "Dot")
        {
            scatterStyle = QCPScatterStyle::ssDot;
        }
        else if(style == "Cross")
        {
            scatterStyle = QCPScatterStyle::ssCross;
        }
        else if(style == "Plus")
        {
            scatterStyle = QCPScatterStyle::ssPlus;
        }
        else if(style == "Circle")
        {
            scatterStyle = QCPScatterStyle::ssCircle;
        }
        else if(style == "Disc")
        {
            scatterStyle = QCPScatterStyle::ssDisc;
        }
        else if(style == "Square")
        {
            scatterStyle = QCPScatterStyle::ssSquare;
        }
        else if(style == "Diamond")
        {
            scatterStyle = QCPScatterStyle::ssDiamond;
        }
        else if(style == "Star")
        {
            scatterStyle = QCPScatterStyle::ssStar;
        }
        else if(style == "Triangle")
        {
            scatterStyle = QCPScatterStyle::ssTriangle;
        }
        else if(style == "TriangleInverted")
        {
            scatterStyle = QCPScatterStyle::ssTriangleInverted;
        }
        else if(style == "CrossSquare")
        {
            scatterStyle = QCPScatterStyle::ssCrossSquare;
        }
        else if(style == "PlusSquare")
        {
            scatterStyle = QCPScatterStyle::ssPlusSquare;
        }
        else if(style == "CrossCircle")
        {
            scatterStyle = QCPScatterStyle::ssCrossCircle;
        }
        else if(style == "PlusCircle")
        {
            scatterStyle = QCPScatterStyle::ssPlusCircle;
        }
        else if(style == "Peace")
        {
            scatterStyle = QCPScatterStyle::ssPeace;
        }
        else
        {
            emit appendTextToConsole(Q_FUNC_INFO + QString("invalid style in setScattertyle:%1").arg(style));
        }

        m_plotWidget->graph(graphIndex)->setScatterStyle(QCPScatterStyle(scatterStyle, size));

    }
    else
    {//invalid index
        emit appendTextToConsole(Q_FUNC_INFO + QString("graph index is out of bounds:%1").arg(graphIndex));
    }
}

/**
 * This slot function sets the line style of a graph.
 *
 * @param graphIndex
 *      The graph index.
 * @param style
 *      The line style. Possible values:
 *      - None: Data points are not connected with any lines (e.g. data only represented
 *              with symbols according to the scatter style (is set with setScatterStyle)).
 *      - Line: Data points are connected by a straight line.
 *      - StepLeft: Line is drawn as steps where the step height is the value of the left data point.
 *      - StepRight: Line is drawn as steps where the step height is the value of the right data point.
 *      - StepCenter: Line is drawn as steps where the step is in between two data points.
 *      - Impulse: Each data point is represented by a line parallel to the value axis, which reaches from the data point to the zero-value-line.
 */
void ScriptPlotWidget::setLineStyleSlot(int graphIndex, QString style)
{
    QCPGraph::LineStyle lineStyle = QCPGraph::lsNone;

    if (graphIndex >= 0 && graphIndex < m_plotWidget->graphCount())
    {
        if(style == "None")
        {
            lineStyle = QCPGraph::lsNone;
        }
        else if(style == "Line")
        {
            lineStyle = QCPGraph::lsLine;
        }
        else if(style == "StepLeft")
        {
            lineStyle = QCPGraph::lsStepLeft;
        }
        else if(style == "StepRight")
        {
            lineStyle = QCPGraph::lsStepRight;
        }
        else if(style == "StepCenter")
        {
            lineStyle = QCPGraph::lsStepCenter;
        }
        else if(style == "Impulse")
        {
            lineStyle = QCPGraph::lsImpulse;
        }
        else
        {
            emit appendTextToConsole(Q_FUNC_INFO + QString("invalid style in setLineStyle:%1").arg(style));
        }

        m_plotWidget->graph(graphIndex)->setLineStyle(lineStyle);
    }
    else
    {//invalid index
        emit appendTextToConsole(Q_FUNC_INFO + QString("graph index is out of bounds:%1").arg(graphIndex));
    }
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
void ScriptPlotWidget::addGraphSlot(QString color, QString penStyle, QString name, int *graphIndex)
{
    m_plotWidget->addGraph();
    int index = m_plotWidget->graphCount()- 1;

    QPen pen;

    if(color == "blue")
    {
        pen.setColor(Qt::blue);
    }
    else if(color == "red")
    {
        pen.setColor(Qt::red);
    }
    else if(color == "yellow")
    {
        pen.setColor(Qt::yellow);
    }
    else if(color == "green")
    {
        pen.setColor(Qt::green);
    }
    else if(color == "black")
    {
        pen.setColor(Qt::black);
    }
    else
    {
        emit appendTextToConsole(Q_FUNC_INFO + QString("invalid color in addGraphSlot:%1").arg(color));

        //user black as default
        pen.setColor(Qt::black);
    }

    if(penStyle == "dash")
    {
        pen.setStyle(Qt::DashLine);
    }
    else if(penStyle == "dot")
    {
        pen.setStyle(Qt::DotLine);
    }
    else if(penStyle == "solid")
    {
        pen.setStyle(Qt::SolidLine);
    }
    else
    {
        emit appendTextToConsole(Q_FUNC_INFO + QString("invalid pen style in addGraphSlot:%1").arg(penStyle));

        //use slid as default
        pen.setStyle(Qt::SolidLine);
    }

    m_plotWidget->graph(index)->setPen(pen);

    m_plotWidget->graph(index)->rescaleAxes(true);

    if(!name.isEmpty())
    {
        m_plotWidget->graph(index)->setName(name);
    }
    else
    {
        m_plotWidget->graph(index)->setName(QString("graph %1").arg(index + 1));
    }


    m_xAxisMaxValues.push_back(0);

    std::vector<PlotPoint>* vec = new std::vector<PlotPoint>();
    m_savePointDuringPlotFreeze.push_back(vec);

    //Create and add the visibility check box.
    QCheckBox* box = new QCheckBox(m_plotWidget);
    box->setChecked(true);
    box->setText(m_plotWidget->graph(index)->name());
    m_grouBoxLayout->addWidget(box);
    connect(box, SIGNAL(stateChanged(int)), this, SLOT(visibiltyCheckBoxSlot(int)));
    m_visibilityCheckBoxes.push_back(box);

    *graphIndex = index;
}

/**
 * Sets the visibility of several plot widget elements.
 * @param showXRange
 *      True if the x range input field shall be visible.
 * @param showYRange
 *      True if the y range input fields shall be visible.
 * @param showUpdate
 *      True if the x update check box shall be visible.
 * @param showSave
 *      True if the save button shall be visible.
 * @param showLoad
 *      True if the load button shall be visible.
 * @param showClear
 *      True if the clear button shall be visible.
 * @param showGraphVisibility
 *      True if the show group box.
 */
void ScriptPlotWidget::showHelperElementsSlot(bool showXRange, bool showYRange, bool showUpdate, bool showSave, bool showLoad, bool showClear, bool showGraphVisibility,
                                              quint32 graphVisibilityMaxSize, bool showLegend)
{
    m_xRangeLineEdit->setVisible(showXRange);
    m_xRangeLabel->setVisible(showXRange);

    m_yMinRangeLabel->setVisible(showYRange);
    m_yMaxRangeLabel->setVisible(showYRange);
    m_yMinRangeLineEdit->setVisible(showYRange);
    m_yMaxRangeLineEdit->setVisible(showYRange);

    m_updatePlotCheckBox->setVisible(showUpdate);
    m_savePushButton->setVisible(showSave);
    m_loadPushButton->setVisible(showLoad);

    m_clearPushButton->setVisible(showClear);

    m_groupBox->setVisible(showGraphVisibility);
    m_groupBox->setMaximumWidth(graphVisibilityMaxSize);

    m_showLegendCheckBox->setVisible(showLegend);

}

/**
 * This function adjusts the borders of diagram and replots all graphs.
 * It is called periodically by m_plotTimer.
 */
void ScriptPlotWidget::plotTimeoutSlot()
{

    if(m_updatePlotCheckBox->isChecked())
    {
        for(auto x : m_xAxisMaxValues)
        {
            m_plotWidget->xAxis->setRange(x - m_xRangeLineEdit->text().toDouble(), x+ m_xRangeLineEdit->text().toDouble() / 10);
        }
        m_plotWidget->yAxis->setRange(m_yMinRangeLineEdit->text().toDouble(), m_yMaxRangeLineEdit->text().toDouble());
        m_plotWidget->replot();
    }

    static quint32 checkMaxNumberOfValuesCounter = 0;
    checkMaxNumberOfValuesCounter++;
    if(checkMaxNumberOfValuesCounter >= 10)
    {//The following code is called every tenth call of plotTimeoutSlot.


        //Limit the number of data points in all graphs.
        for (int graphIndex = 0; graphIndex < m_plotWidget->graphCount(); graphIndex++)
        {
            const QCPDataMap* data = m_plotWidget->graph(graphIndex)->data();
            if(data->size() > m_maxDataPointsPerGraph)
            {
                QCPDataMap::const_iterator it = data->begin();
                int numberOfElements = data->size() - m_maxDataPointsPerGraph;

                while(numberOfElements > 0)
                {
                    it++;
                    numberOfElements--;
                }
                m_plotWidget->graph(graphIndex)->removeDataBefore(it.key());
            }
        }
    }
}

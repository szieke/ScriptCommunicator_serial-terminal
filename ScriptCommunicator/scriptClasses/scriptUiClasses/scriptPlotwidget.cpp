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
    QObject(0), m_scriptThread(scriptThread), m_maxDataPointsPerGraph(10000000), m_addSpaceAfterBiggestValues(false), m_yAxis2IsVisible(false),
    m_yRangeHelperVisible(false)
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

    m_y2MinRangeLabel = new QLabel("y 2 min Range",m_plotWidget);
    vLayout->addWidget(m_y2MinRangeLabel);
    //Create and add the y 2 min range line edit.
    m_y2MinRangeLineEdit = new QLineEdit(m_plotWidget);
    m_y2MinRangeLineEdit->setValidator(new QDoubleValidator(INT_MIN, INT_MAX, 20, m_y2MinRangeLineEdit));
    m_y2MinRangeLineEdit->setMaximumWidth(80);
    vLayout->addWidget(m_y2MinRangeLineEdit);

    m_y2MaxRangeLabel = new QLabel("y 2 max Range",m_plotWidget);
    vLayout->addWidget(m_y2MaxRangeLabel);
    //Create and add the y 2 max range line edit.
    m_y2MaxRangeLineEdit = new QLineEdit(m_plotWidget);
    m_y2MaxRangeLineEdit->setValidator(new QDoubleValidator(INT_MIN, INT_MAX, 20, m_y2MaxRangeLineEdit));
    m_y2MaxRangeLineEdit->setMaximumWidth(80);
    vLayout->addWidget(m_y2MaxRangeLineEdit);

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


    connect(this, SIGNAL(updatePlotSignal()), m_plotWidget, SLOT(replot()), Qt::QueuedConnection);


    connect(this, SIGNAL(addGraphSignal(QString, QString, QString, int*,bool)),
            this, SLOT(addGraphSlot(QString, QString, QString, int*,bool)), directConnectionType);

    connect(this, SIGNAL(setInitialAxisRangesSignal(double, double, double, bool,double,double)),
            this, SLOT(setInitialAxisRangesSlot(double, double, double, bool, double,double)), Qt::QueuedConnection);

    connect(this, SIGNAL(addDataToGraphSignal(int, double, double, bool)),
            this, SLOT(addDataToGraphSlot(int, double, double, bool)), Qt::QueuedConnection);

    connect(this, SIGNAL(removeDataRangeFromGraphSignal(int,double,double,bool)),
            this, SLOT(removeDataRangeFromGraphSlot(int,double,double,bool)), Qt::QueuedConnection);

    connect(this, SIGNAL(setScatterStyleSignal(int,QString,double)),
            this, SLOT(setScatterStyleSlot(int,QString,double)), Qt::QueuedConnection);

    connect(this, SIGNAL(setLineStyleSignal(int,QString)),
            this, SLOT(setLineStyleSlot(int,QString)), Qt::QueuedConnection);

    connect(this, SIGNAL(setLineWidthSignal(int,int)),
            this, SLOT(setLineWidthSlot(int,int)), Qt::QueuedConnection);

    connect(this, SIGNAL(setAxisLabelsSignal(QString,QString,QString)),
            this, SLOT(setAxisLabelsSlot(QString,QString,QString)), Qt::QueuedConnection);

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

    connect(m_y2MinRangeLineEdit, SIGNAL(textChanged(QString)),
            this, SLOT(doubleLineEditChangedSlot(QString)), Qt::QueuedConnection);

    connect(m_y2MaxRangeLineEdit, SIGNAL(textChanged(QString)),
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

    connect(this, SIGNAL(saveAllGraphsSignal(QString,bool*)),
            this, SLOT(saveAllGraphsSlot(QString,bool*)), directConnectionType);

    connect(this, SIGNAL(setLocaleSignal(int,int)),this, SLOT(setLocaleSlot(int,int)), Qt::QueuedConnection);

    connect(this, SIGNAL(showDateTimeAtXAxisSignal(QString)),
            this, SLOT(showDateTimeAtXAxisSlot(QString)), Qt::QueuedConnection);

}

/**
 * Destruktor.
 */
ScriptPlotWidget::~ScriptPlotWidget()
{
    ///delete all vector vectors in the freeze vector
    for(uint i = 0; i < m_savedOperationsDuringPlotFreeze.size(); i++)
    {
        delete m_savedOperationsDuringPlotFreeze[i];
    }

    m_plotWidget->deleteLater();
}

/**
 * Sets the locale of the script plot widget.
 * @param language
 *      The language (QLocale::Language).
 * @param country
 *      The country (QLocale::Country)
 */
void ScriptPlotWidget::setLocaleSlot(int language, int country)
{
    m_plotWidget->setLocale(QLocale(static_cast<QLocale::Language>(language), static_cast<QLocale::Country>(country)));
}

/**
 * If called the x values are interpreted as seconds that have passed since
 * 970-01-01T00:00:00.000, Coordinated Universal Time (and the corresponding date time is shown).
 * @param format
 *      The date time format (see QDateTime::toString for more details).
 */
void ScriptPlotWidget::showDateTimeAtXAxisSlot(QString format)
{
    QSharedPointer<QCPAxisTickerDateTime> dateTicker(new QCPAxisTickerDateTime);
    dateTicker->setDateTimeFormat(format);
    m_plotWidget->xAxis->setTicker(dateTicker);
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
        m_plotWidget->graph(i)->data()->clear();
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
    m_plotWidget->xAxis->setRange(m_xRangeLineEdit->text().toDouble() * -1,  0);
    m_plotWidget->yAxis->setRange(m_yMinRangeLineEdit->text().toDouble(), m_yMaxRangeLineEdit->text().toDouble());
    m_plotWidget->yAxis2->setRange(m_y2MinRangeLineEdit->text().toDouble(), m_y2MaxRangeLineEdit->text().toDouble());
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

    // event has sometimes wrong x and y position values (maybe related to the issue below)
    // get global mouse position and translate them to widget position
    QPoint rpos = m_plotWidget->mapFromGlobal(QCursor::pos());

    double xValue = m_plotWidget->xAxis->pixelToCoord(rpos.x());
    double yValue = m_plotWidget->yAxis->pixelToCoord(rpos.y());

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

    if(sender() == m_xRangeLineEdit)
    {
        bool isOk;
        emit xRangeChangedSignal(m_xRangeLineEdit->text().toDouble(&isOk));
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
 * This function saves all displayed graphs to a file.
 * @param fileName
 *  The file name. Possible file extensions are: png, jpg, bmp, pdf, csv. If fileName has no or an invalid file
 *  extension then a file with comma separated values is created.
 * @param hasSucceed
 *  True if saving has succeeded.
 */
void ScriptPlotWidget::saveAllGraphsSlot(QString fileName, bool* hasSucceed)
{
    *hasSucceed = false;

    if(!fileName.isEmpty())
    {
        QFileInfo fileInfo (fileName);
        QString suffix = fileInfo.suffix();


        if(suffix.isEmpty())
        {//No suffix.

            fileName += ".csv";
        }


        if(suffix == "png")
        {
            *hasSucceed = m_plotWidget->savePng(fileName);
        }
        else if(suffix == "jpg")
        {
            *hasSucceed = m_plotWidget->saveJpg(fileName);
        }
        else if(suffix == "bmp")
        {
            *hasSucceed = m_plotWidget->saveBmp(fileName);
        }
        else if(suffix == "pdf")
        {
           *hasSucceed =  m_plotWidget->savePdf(fileName);
        }
       else //Invalid suffix and csv
        {
            QString content = "plot version=2\n";

            for (int i = 0; i < m_plotWidget->graphCount(); i++)
            {
                if(m_plotWidget->graph(i)->visible())
                {

                    content += m_plotWidget->graph(i)->name() + ",";
                    content += QString("%1,").arg("color=" + m_plotWidget->graph(i)->pen().color().name());
                    content += QString("%1,").arg("style=" + QString::number(m_plotWidget->graph(i)->pen().style()));
                    content += QString("%1,").arg((m_plotWidget->graph(i)->valueAxis() == m_plotWidget->yAxis2) ? "useYAxis2=true" : "useYAxis2=false");
                    content += QString("%1,").arg("shape=" + QString::number(m_plotWidget->graph(i)->scatterStyle().shape()));
                    content += QString("%1,").arg("scatterStyle=" + QString::number(m_plotWidget->graph(i)->scatterStyle().size(), 'g', 20));
                    content += QString("%1,").arg("lineStyle=" + QString::number(m_plotWidget->graph(i)->lineStyle()));

                    QSharedPointer<QCPGraphDataContainer>  map = m_plotWidget->graph(i)->data();
                    QCPDataContainer<QCPGraphData>::const_iterator iter;
                    for (iter = map->begin(); iter != map->end(); ++iter)
                    {
                        content += QString("%1:%2,").arg(QString::number(iter->key, 'g', 20)).arg(QString::number(iter->value, 'g', 20));
                    }

                    //Remove the last ','
                    content.remove(content.length() - 1, 1);
                    content += "\n";
                }

            }

             *hasSucceed = m_scriptThread->writeFile(fileName, false,content,true);

        }
    }

}
/**
 * This function saves the current displayed graphs.
 * It is called if the save button is pressed.
 */
void ScriptPlotWidget::saveButtonPressed()
{

    bool hasSucceed;
    QString selectedFilter;
    QString fileName = QFileDialog::getSaveFileName(m_plotWidget, tr("save as image/csv"),
                                                      "",tr("PNG (*.png);;JPG (*.jpg);;BMP (*.bmp);;PDF (*.pdf);;CSV (*.csv)"), &selectedFilter);
    if(!fileName.isEmpty())
    {
        QFileInfo fileInfo (fileName);

        if(fileInfo.suffix().isEmpty())
        {//No suffix entered.

            QString suffix = selectedFilter.remove(0, selectedFilter.indexOf(".") + 1);
            suffix.remove(")");
            fileName += "." + suffix;
        }
       saveAllGraphsSlot(fileName, &hasSucceed);

       if(!hasSucceed)
       {
           QMessageBox::critical(m_plotWidget, "error", QString("error while saving graphs to file: %1").arg(fileName));

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
        int dataStartIndex = 4;

        bool containsUseYAxis2 = false;
        int versionIndex = graphs[0].indexOf("plot version=");
        int version = 0;
        if(versionIndex != -1)
        {
            containsUseYAxis2 = true;
            version = graphs[0].remove("plot version=").toUInt();
            graphs.removeFirst();
        }

        if(version > 1)
        {
            dataStartIndex = 7;
        }


        //The update check box must be checked for addDataToGraphSlot.
        bool updateCheckBoxState = m_updatePlotCheckBox->isChecked();
        m_updatePlotCheckBox->setChecked(true);

        for(qint32 i = 0; i < graphs.size(); i++)
        {
            //The single values are separated with ,.
            QStringList values = graphs[i].split(",");

            int minNumberOfValues = (version > 1) ? 7 : 4;
            if(values.size() >= minNumberOfValues)
            {
                int graphIndex = 0;
                bool useYAxis2 = false;
                if(containsUseYAxis2)
                {
                    useYAxis2 = (values[3] == "useYAxis2=true") ? true : false;;
                }

                addGraphSlot("blue", "solid", values[0], &graphIndex, useYAxis2);

                QString colorString = (version > 1) ? values[1].remove("color=") : values[1];
                QString styleString = (version > 1) ? values[2].remove("style=") : values[2];
                QPen pen;
                pen.setColor(colorString);
                pen.setStyle((Qt::PenStyle)styleString.toUInt());
                m_plotWidget->graph(graphIndex)->setPen(pen);

                if(version > 1)
                {
                    QCPScatterStyle::ScatterShape shape = (QCPScatterStyle::ScatterShape)values[4].remove("shape=").toUInt();
                    double size = values[5].remove("scatterStyle=").toDouble();
                    m_plotWidget->graph(graphIndex)->setScatterStyle(QCPScatterStyle(shape, size));

                    QCPGraph::LineStyle lineStyle = (QCPGraph::LineStyle)values[6].remove("lineStyle=").toUInt();
                    m_plotWidget->graph(graphIndex)->setLineStyle(lineStyle);
                }


                for(qint32 index = dataStartIndex; index < values.size(); index++)
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
    //Adjust the borders of diagram and replot all graphs.
    adjustBordersAndReplot();

    if(state != 0)
    {
        //add all saved points (during the plot window freeze)
        for(uint i = 0; i < m_savedOperationsDuringPlotFreeze.size(); i++)
        {
            for(uint j = 0; j < m_savedOperationsDuringPlotFreeze[i]->size(); j++)
            {
                if(m_savedOperationsDuringPlotFreeze[i]->at(j).isAdded)
                {
                    addDataToGraphSlot(i, m_savedOperationsDuringPlotFreeze[i]->at(j).value1, m_savedOperationsDuringPlotFreeze[i]->at(j).value2);
                }
                else
                {
                    removeDataRangeFromGraphSlot(i, m_savedOperationsDuringPlotFreeze[i]->at(j).value1, m_savedOperationsDuringPlotFreeze[i]->at(j).value2);
                }
            }
            m_savedOperationsDuringPlotFreeze[i]->clear();
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
 * @param y2MinValue
 *      The min. values of the y axis 2.
 * @param y2MaxValue
 *      The max. value of the y axis 2.
 * @param addSpaceAfterBiggestValues
 *      True if a space shall be added after the biggest value of a graph.
 */
void ScriptPlotWidget::setInitialAxisRangesSlot(double xRange, double yMinValue, double yMaxValue, bool addSpaceAfterBiggestValues, double y2MinValue, double y2MaxValue)
{
    m_addSpaceAfterBiggestValues = addSpaceAfterBiggestValues;
    m_xRangeLineEdit->setText(QString("%1").arg(xRange));
    m_yMinRangeLineEdit->setText(QString("%1").arg(yMinValue));
    m_yMaxRangeLineEdit->setText(QString("%1").arg(yMaxValue));
    m_y2MinRangeLineEdit->setText(QString("%1").arg(y2MinValue));
    m_y2MaxRangeLineEdit->setText(QString("%1").arg(y2MaxValue));
    m_plotWidget->yAxis->setRange(yMinValue, yMaxValue);
    m_plotWidget->yAxis2->setRange(y2MinValue, y2MaxValue);
    m_plotWidget->xAxis->setRange(xRange * -1, 0);
}

/**
 * The function sets the current ranges of the diagram.
 * @param xMinValue
 *      The min. values of the x axis.
 * @param xMaxValue
 *      The max. value of the x axis.
 * @param yMinValue
 *      The min. values of the y axis.
 * @param yMaxValue
 *      The max. value of the y axis.
 * @param y2MinValue
 *      The min. values of the y axis 2.
 * @param y2MaxValue
 *      The max. value of the y axis 2.
 */
void ScriptPlotWidget::setCurrentAxisRanges(double xMinValue, double xMaxValue, double yMinValue, double yMaxValue, double y2MinValue, double y2MaxValue)
{
    m_plotWidget->yAxis->setRange(yMinValue, yMaxValue);
    m_plotWidget->yAxis2->setRange(y2MinValue, y2MaxValue);
    m_plotWidget->xAxis->setRange(xMinValue, xMaxValue);
    m_plotWidget->replot();
}

/**
 * The function gets the current axis ranges of the diagram.
 * @return
 *      Current view ranges.
 */
QJSValue ScriptPlotWidget::getCurrentAxisRanges(void)
{
    const QCPRange xrange = m_plotWidget->xAxis->range();
    const QCPRange yrange = m_plotWidget->yAxis->range();
    const QCPRange y2range = m_plotWidget->yAxis2->range();

    QJSValue ret = m_scriptThread->getScriptEngine()->newObject();
    ret.setProperty("xMinValue", xrange.lower);
    ret.setProperty("xMaxValue", xrange.upper);
    ret.setProperty("yMinValue", yrange.lower);
    ret.setProperty("yMaxValue", yrange.upper);
    ret.setProperty("y2MinValue", y2range.lower);
    ret.setProperty("y2MaxValue", y2range.upper);
    return ret;
}

/**
 * Sets the axis labels.
 * @param xAxisLabel
 *      The label for the x axis.
 * @param yAxisLabel
 *      The label for the y axis.
 */
void ScriptPlotWidget::setAxisLabelsSlot(QString xAxisLabel, QString yAxisLabel, QString yAxis2Label)
{
    m_plotWidget->xAxis->setLabel(xAxisLabel);
    m_plotWidget->yAxis->setLabel(yAxisLabel);
    m_plotWidget->yAxis2->setLabel(yAxis2Label);
}


/**
 * Removes all data points with (sort-)keys between xFrom and xTo. If
 * xFrom is greater or equal to xTo, the function does nothing.
 *
 * @param graphIndex
 *      The graph index.
 * @param xFrom
 *      The start position.
 * @param xTo
 *      The end position.
 * @param force
 *      If true then the data is removed even if auto update is disabled (setAutoUpdateEnabled).
 */
void ScriptPlotWidget::removeDataRangeFromGraphSlot(int graphIndex, double xFrom, double xTo, bool force)
{
    if(m_updatePlotCheckBox->isChecked() || force)
    {
        if (graphIndex >= 0 && graphIndex < m_plotWidget->graphCount())
        {
            m_plotWidget->graph(graphIndex)->data()->remove(xFrom, xTo);

            if(xTo >= m_xAxisMaxValues[graphIndex])
            {
                bool foundRange;
                QCPRange range = m_plotWidget->graph(graphIndex)->data()->keyRange(foundRange);
                m_xAxisMaxValues[graphIndex] = range.upper;
            }

        }
        else
        {//invalid index
            emit appendTextToConsole(Q_FUNC_INFO + QString("graph index is out of bounds:%1").arg(graphIndex));
        }
    }
    else
   {//the plot window is freezed

       //save the point in the freeze array
       SavedPlotOperation savedOperation;
       savedOperation.value1 = xFrom;
       savedOperation.value2 = xTo;
       savedOperation.isAdded = false;
       m_savedOperationsDuringPlotFreeze[graphIndex]->push_back(savedOperation);
   }
}

/**
 * This slot function adds one point to a graph.
 * @param graphIndex
 *      The graph index.
 * @param x
 *      The x value of the point.
 * @param y
 *      The y value of the point
 * @param force
 *      If true then the data is added even ifauto update is disabled (setAutoUpdateEnabled).
 * @return
 *      True for success.
 */
bool ScriptPlotWidget::addDataToGraphSlot(int graphIndex, double x, double y, bool force)
{
    bool hasSucceeded = true;

    if(m_updatePlotCheckBox->isChecked() || force)
    {
        if (graphIndex >= 0 && graphIndex < m_plotWidget->graphCount())
        {
            m_plotWidget->graph(graphIndex)->addData(x,y);

            if(x > m_xAxisMaxValues[graphIndex])
            {
                m_xAxisMaxValues[graphIndex] = x;
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
        SavedPlotOperation savedOperation;
        savedOperation.value1 = x;
        savedOperation.value2 = y;
        savedOperation.isAdded = true;
        m_savedOperationsDuringPlotFreeze[graphIndex]->push_back(savedOperation);
    }

    return hasSucceeded;
}

/**
 * This function returns several data points from a graph.
 * @param graphIndex
 *     The index off the target graph.
 * @param xStart
 *      The x coordinate of the first data point.
 * @param count
 *      Number of points to grab. If negative then this function grabs backwards.
 * @return
 *      Array of found x and y pairs.
 */
QJSValue ScriptPlotWidget::getDataFromGraph(int graphIndex, double xStart, int count)
{
    QJSValue ret = m_scriptThread->getScriptEngine()->newArray();

    if (graphIndex >= 0 && graphIndex < m_plotWidget->graphCount())
    {
        QCPGraphDataContainer::const_iterator beg = m_plotWidget->graph(graphIndex)->data()->constBegin();
        QCPGraphDataContainer::const_iterator end = m_plotWidget->graph(graphIndex)->data()->constEnd();
        QCPGraphDataContainer::const_iterator it = m_plotWidget->graph(graphIndex)->data()->findBegin(xStart, false);

        int idx = 0;

        // get all items until count items has been shifted out or no more items left
        while ((it != end) && (count != 0))
        {
            QJSValue value = m_scriptThread->getScriptEngine()->newObject();
            value.setProperty("x", it->key);
            value.setProperty("y", it->value);
            ret.setProperty(idx, value);

            // if reverse traveling is selected abort after first item has been processed
            if (it == beg)
                break;

            idx++;

            if (count  > 0)
            {
                count--;
                it++;
            }
            else
            {
                count++;
                it--;
            }
        }
    }
    else
    {//invalid index
        emit appendTextToConsole(Q_FUNC_INFO + QString("graph index is out of bounds:%1").arg(graphIndex));
    }
    return ret;
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
 * This slot function sets the line style of a graph.
 *
 * @param graphIndex
 *      The graph index.
 * @param width
 *      The line width.
 */
void ScriptPlotWidget::setLineWidthSlot(int graphIndex, int width)
{
    if (graphIndex >= 0 && graphIndex < m_plotWidget->graphCount())
    {
        QPen pen = m_plotWidget->graph(graphIndex)->pen();
        pen.setWidth(width);
        m_plotWidget->graph(graphIndex)->setPen(pen);
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
 * @param useYAxis2
 *     True if y axis 2 shall be used for this graph
 */
void ScriptPlotWidget::addGraphSlot(QString color, QString penStyle, QString name, int *graphIndex, bool useYAxis2)
{
    if(useYAxis2)
    {
        m_plotWidget->yAxis2->setVisible(true);
        m_plotWidget->yAxis2->setTickLabels(true);
        m_yAxis2IsVisible = true;

        m_y2MinRangeLabel->setVisible(m_yRangeHelperVisible);
        m_y2MaxRangeLabel->setVisible(m_yRangeHelperVisible);
        m_y2MinRangeLineEdit->setVisible(m_yRangeHelperVisible);
        m_y2MaxRangeLineEdit->setVisible(m_yRangeHelperVisible);
    }

    m_plotWidget->addGraph( m_plotWidget->xAxis, useYAxis2 ? m_plotWidget->yAxis2 : m_plotWidget->yAxis);
    int index = m_plotWidget->graphCount()- 1;

    QPen pen;

    if (QColor::isValidColor(color))
    {
         pen.setColor(QColor(color));
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

    std::vector<SavedPlotOperation>* vec = new std::vector<SavedPlotOperation>();
    m_savedOperationsDuringPlotFreeze.push_back(vec);

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

    m_yRangeHelperVisible = showYRange;
    m_y2MinRangeLabel->setVisible(showYRange && m_yAxis2IsVisible);
    m_y2MaxRangeLabel->setVisible(showYRange && m_yAxis2IsVisible);
    m_y2MinRangeLineEdit->setVisible(showYRange && m_yAxis2IsVisible);
    m_y2MaxRangeLineEdit->setVisible(showYRange && m_yAxis2IsVisible);


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
void ScriptPlotWidget::adjustBordersAndReplot(void)
{   
    if (m_xAxisMaxValues.size())
    {
        auto max_x = *std::max_element(m_xAxisMaxValues.begin(), m_xAxisMaxValues.end());

        m_plotWidget->xAxis->setRange(max_x - m_xRangeLineEdit->text().toDouble(),
                                      m_addSpaceAfterBiggestValues ? max_x + (m_xRangeLineEdit->text().toDouble() / 10) : max_x);
    }

    m_plotWidget->yAxis->setRange(m_yMinRangeLineEdit->text().toDouble(), m_yMaxRangeLineEdit->text().toDouble());
    m_plotWidget->yAxis2->setRange(m_y2MinRangeLineEdit->text().toDouble(), m_y2MaxRangeLineEdit->text().toDouble());
    m_plotWidget->replot();
}

/**
 * This function is called periodically by m_plotTimer (adjusts the borders of diagram and replots all graphs).
 */
void ScriptPlotWidget::plotTimeoutSlot()
{

    if(m_updatePlotCheckBox->isChecked())
    {
        //Adjust the borders of diagram and replot all graphs.
        adjustBordersAndReplot();
    }

    static quint32 checkMaxNumberOfValuesCounter = 0;
    checkMaxNumberOfValuesCounter++;
    if(checkMaxNumberOfValuesCounter >= 10)
    {//The following code is called every tenth call of plotTimeoutSlot.


        //Limit the number of data points in all graphs.
        for (int graphIndex = 0; graphIndex < m_plotWidget->graphCount(); graphIndex++)
        {
            QSharedPointer<QCPGraphDataContainer>  data = m_plotWidget->graph(graphIndex)->data();
            if(data->size() > m_maxDataPointsPerGraph)
            {
                QCPGraphDataContainer::const_iterator it = data->begin();
                int numberOfElements = data->size() - m_maxDataPointsPerGraph;

                while(numberOfElements > 0)
                {
                    it++;
                    numberOfElements--;
                }
                m_plotWidget->graph(graphIndex)->data()->removeBefore(it->key);
            }
        }
    }
}


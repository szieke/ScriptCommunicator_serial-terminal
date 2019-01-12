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

#include "scriptwindow.h"
#include "ui_scriptwindow.h"
#include<QTimer>
#include <QMenu>
#include <QFileDialog>
#include <QBuffer>
#include <QDomDocument>
#include "plotwindow.h"
#include "scriptComboBox.h"
#include "scriptLineEdit.h"
#include "mainwindow.h"
#include "scriptTableWidget.h"
#include "scriptTextEdit.h"
#include "scriptCheckBox.h"
#include "scriptButton.h"
#include "scriptPlotWindow.h"
#include "scriptProgressBar.h"
#include "scriptSpinBox.h"
#include "scriptTimeEdit.h"
#include "scriptDateEdit.h"
#include "scriptTextEdit.h"
#include "scriptSlider.h"

///Deletes the current SCEZ folder.
extern void deleteCurrentScezFolder(void);




/**
 * Drag enter event.
 * @param event
 *      The drag enter event.
 */
void DragDropTableWidget::dragEnterEvent(QDragEnterEvent *event)
{
    if(event->mimeData()->hasUrls())
    {
        event->acceptProposedAction();
    }
}

/**
 * Handles the data supplied by a drag and drop operation that ended with the given action in the given row and column.
 *
 * @param row
 *      The row.
 * @param column
 *      The column.
 * @param data
 *      The mime data.
 * @param action
 *      The drop action.
 * @return
 *      Returns true if the data and action can be handled by the model; otherwise returns false.
 *
 */
bool DragDropTableWidget::dropMimeData(int row, int column, const QMimeData *data, Qt::DropAction action)
{
    if(data->hasUrls())
    {
#ifdef Q_OS_LINUX
        QString files = data->text().remove("file://");
#else
        QString files = data->text().remove("file:///");
#endif
        QStringList list = files.split("\n");
        if(!list.isEmpty())
        {
            emit dropEventSignal(row, column, list);
        }

        return false;
    }
    else
    {
        return QTableWidget::dropMimeData(row, column, data, action);
    }
}

/**
 * Returns a list of MIME types that can be used to describe a list of tablewidget items.
 */
QStringList DragDropTableWidget::mimeTypes () const
{
    QStringList qstrList;
    qstrList.append("text/uri-list");
    return qstrList;
}

/**
 * Returns the drop actions supported by this view.
 */
Qt::DropActions DragDropTableWidget::supportedDropActions () const
{
    return Qt::CopyAction | Qt::MoveAction | Qt::LinkAction;
}

/**
 * Constructor.
 *
 * @param mainWindow
 *      Pointer to the main window.
 * @param thread
 *      Pointer to the main interface thread.
 * @param scripts
 *      The command-line scripts.
 */
ScriptWindow::ScriptWindow(MainWindow* mainWindow, MainInterfaceThread *thread, QStringList scripts) : ScriptSlots(),
    m_userInterface(new Ui::ScriptWindow), m_mainWindow(mainWindow), m_sendIdCounter(MainInterfaceThread::SEND_ID_SCRIPTS_START), m_mainInterfaceThread(thread),
    m_commandLineScripts(scripts), m_createSceFileDialog(0), m_exitCode(0)
{
    m_userInterface->setupUi(this);

    m_createSceFileDialog = new CreateSceFile(0, m_userInterface->tableWidget);

    connect(m_userInterface->tableWidget->horizontalHeader(), SIGNAL(sectionResized(int, int, int)), this, SLOT(resizeTableColumnsSlot()));

    QShortcut *shortcut = new QShortcut(QKeySequence("Ctrl+1"), this);
    QObject::connect(shortcut, SIGNAL(activated()), this, SLOT(startButtonPressedSlot()));

    shortcut = new QShortcut(QKeySequence("Ctrl+2"), this);
    QObject::connect(shortcut, SIGNAL(activated()), this, SLOT(pauseButtonPressedSlot()));

    shortcut = new QShortcut(QKeySequence("Ctrl+3"), this);
    QObject::connect(shortcut, SIGNAL(activated()), this, SLOT(stopButtonPressedSlot()));

    shortcut = new QShortcut(QKeySequence("Ctrl+Shift+X"), this);
    QObject::connect(shortcut, SIGNAL(activated()), this, SLOT(close()));

    connect(this->m_userInterface->startPushButton, SIGNAL(clicked()), this, SLOT(startButtonPressedSlot()));
    connect(this->m_userInterface->pausePushButton, SIGNAL(clicked()), this, SLOT(pauseButtonPressedSlot()));
    connect(this->m_userInterface->stopPushButton, SIGNAL(clicked()), this, SLOT(stopButtonPressedSlot()));
    connect(this->m_userInterface->closePushButton, SIGNAL(clicked()), this, SLOT(close()));

    connect(m_userInterface->clearConsolePushButton, SIGNAL(clicked()), m_userInterface->scriptConsole, SLOT(clear()));
    connect(m_userInterface->actionAddScript, SIGNAL(triggered()), this, SLOT(addScriptSlot()));
    connect(m_userInterface->actionRemoveScripts, SIGNAL(triggered()), this, SLOT(removeScriptSlot()));
    connect(m_userInterface->actionLoadConfig, SIGNAL(triggered()), this, SLOT(loadConfigSlot()));
    connect(m_userInterface->actionUnloadConfig, SIGNAL(triggered()), this, SLOT(unloadConfigSlot()));
    connect(m_userInterface->actionSaveConfig, SIGNAL(triggered()), this, SLOT(saveConfigSlot()));
    connect(m_userInterface->actionEditAllWorkerScripts, SIGNAL(triggered()), this, SLOT(editAllWorkerScriptsSlot()));
    connect(m_userInterface->actionSaveConfigAs, SIGNAL(triggered()), this, SLOT(saveConfigAsSlot()));
    connect(m_userInterface->actionCreateSceFile, SIGNAL(triggered()), this, SLOT(createSceFileSlot()));

    connect(m_userInterface->actionNewScript, SIGNAL(triggered()), this, SLOT(newScriptSlot()));
    connect(m_userInterface->actionEditScript, SIGNAL(triggered()), this, SLOT(editScriptButtonPressedSlot()));
    connect(m_userInterface->actionEditUi, SIGNAL(triggered()), this, SLOT(editUiSlot()));
    connect(m_userInterface->actionRemoveUi, SIGNAL(triggered()), this, SLOT(removeUi()));
    connect(m_userInterface->actionMoveDown, SIGNAL(triggered()), this, SLOT(moveTableEntryDownSlot()));
    connect(m_userInterface->actionMoveUp, SIGNAL(triggered()), this, SLOT(moveTableEntryUpSlot()));


    connect(m_userInterface->tableWidget, SIGNAL(cellEntered(int, int)), this, SLOT(cellEnteredSlot(int, int)));
    connect(m_userInterface->tableWidget, SIGNAL(cellChanged(int, int)), this, SLOT(cellChangedSlot(int, int)));
    connect(m_userInterface->tableWidget, SIGNAL(dropEventSignal(int,int,QStringList)), this, SLOT(tableDropEventSlot(int,int,QStringList)));
    connect(m_userInterface->tableWidget, SIGNAL(itemSelectionChanged()), this, SLOT(itemSelectionChangedSlot()));

    m_userInterface->tableWidget->setHorizontalHeaderLabels({"name", "status", "worker script path", "user interface"});
    m_currentScriptConfigFileString = tableToString();

    if(!m_commandLineScripts.isEmpty())
    {
        m_userInterface->actionAddScript->setEnabled(false);
        m_userInterface->actionRemoveScripts->setEnabled(false);
        m_userInterface->actionLoadConfig->setEnabled(false);
        m_userInterface->actionUnloadConfig->setEnabled(false);
        m_userInterface->actionSaveConfig->setEnabled(false);
        m_userInterface->actionSaveConfigAs->setEnabled(false);

        m_userInterface->actionNewScript->setEnabled(false);
        m_userInterface->actionEditScript->setEnabled(false);
        m_userInterface->actionEditUi->setEnabled(false);
        m_userInterface->actionRemoveUi->setEnabled(false);
        m_userInterface->actionCreateSceFile->setEnabled(false);



    }
    else
    {
        connect(m_userInterface->tableWidget, SIGNAL(cellDoubleClicked(int, int)), this, SLOT(cellDoubleClickedSlot(int, int)));


    }


}

/**
 * Destructor
 */
ScriptWindow::~ScriptWindow()
{
    m_createSceFileDialog->close();
    delete m_createSceFileDialog;
    delete m_userInterface;
}


/**
 * Returns the splitter sizes.
 * @return
 *      The splitter sizes.
 */
QList<int> ScriptWindow::getSplitterSizes(void)
{
    return m_userInterface->splitter->sizes();
}
/**
 * Sets the splitter sizes.
 * @param sizes
 *  The new sizes.
 */
void ScriptWindow::setSplitterSizes(QList<int> sizes)
{
    m_userInterface->splitter->setSizes(sizes);
}

/**
 * This function is called if the window has to be resized.
 * @param event
 *      The resize event.
 */
void ScriptWindow::resizeEvent(QResizeEvent * event)
{
    (void)event;
    resizeTableColumnsSlot();
}

/**
 * Starts the command line scripts.
 */
void ScriptWindow::startCommandLineScripts(void)
{
    m_userInterface->tableWidget->setRowCount(0);

    for(auto script : m_commandLineScripts)
    {
        QString scriptFile = script;
        if(QFile::exists(scriptFile))
        {

            createNewTableRow();

            QFileInfo fileInfo (script);
            m_userInterface->tableWidget->item(0, COLUMN_NAME)->setText(fileInfo.baseName());
            m_userInterface->tableWidget->item(0, COLUMN_SCRIPT_PATH)->setText(script);

            QString uiFile = fileInfo.absolutePath() + "/" + fileInfo.completeBaseName()+ ".ui";
            if(QFile::exists(uiFile))
            {
                m_userInterface->tableWidget->item(0, COLUMN_UI_PATH)->setText(uiFile);
            }
            startScriptThread(0);
        }
        else
        {
            QMessageBox::critical(this, "error starting script", "could not open " + script);
        }
    }

    m_userInterface->tableWidget->resizeColumnsToContents();
    resizeTableColumnsSlot();

    setTitle("command-line mode");

    resizeTableColumnsSlot();

    checkIfScriptCommunicatorMustExit();
}

/**
 * Shows the script window.
 */
void ScriptWindow::show(void)
{
    QWidget::show();

    resizeTableColumnsSlot();
}


/**
 * Swaps the position of 2 table rows.
 * @param row1
 *      Row 1.
 * @param row2
 *      Row 2.
 */
void ScriptWindow::swapTableRowPositions(int row1, int row2)
{
    QList<QTableWidgetItem*> rowItems1,rowItems2;
    int colCount = m_userInterface->tableWidget->columnCount();

    m_userInterface->tableWidget->blockSignals(true);

    //Remove all cells from the two rows which position have to be swapped.
    for (int col = 0; col < colCount; ++col)
    {
        rowItems1 << m_userInterface->tableWidget->takeItem(row1, col);
        rowItems2 << m_userInterface->tableWidget->takeItem(row2, col);

    }

    //Insert all cells from the two rows which positions have to be swapped
    //at their new positions.
    for (int cola = 0; cola < colCount; ++cola)
    {
        m_userInterface->tableWidget->setItem(row2, cola, rowItems1.at(cola));
        m_userInterface->tableWidget->setItem(row1, cola, rowItems2.at(cola));

    }

    m_userInterface->tableWidget->blockSignals(false);
    emit scriptTableHasChangedSignal();
}

/**
 * This slot function is called if the user enters a cell in the script table.
 * With this function the user can move the selected row up or down (while holding the mouse at the row)
 * @param row
 *      The row of the cell.
 * @param column
 *      The column of the cell.
 */
void ScriptWindow::cellEnteredSlot(int row, int column)
{
    (void) column;
    int rowsel;

    if(m_userInterface->tableWidget->currentIndex().row()<row)
    {
        //The position of the selected row has to be decremented.
        rowsel=row-1;
    }
    else if(m_userInterface->tableWidget->currentIndex().row()>row)
    {
        //The position of the selected row has to be decremented.
        rowsel=row+1;
    }
    else
    {   //The position of the selected row is not changed.
        rowsel = row;
    }

    if(rowsel != row)
    {//The selected row has to change his position with an other row.

        bool verticalScrollBarPressed = false;
        if(m_userInterface->tableWidget->verticalScrollBar())
        {
            verticalScrollBarPressed = m_userInterface->tableWidget->verticalScrollBar()->underMouse();
        }

        bool horizontalScrollBarPressed = false;
        if(m_userInterface->tableWidget->horizontalScrollBar())
        {
            horizontalScrollBarPressed = m_userInterface->tableWidget->horizontalScrollBar()->underMouse();
        }

        if((QApplication::mouseButtons() != Qt::NoButton) &&
                !verticalScrollBarPressed && !horizontalScrollBarPressed)
        {
            swapTableRowPositions(row, rowsel);
        }
    }
}

/**
 * Resizes the table columns.
 */
void ScriptWindow::resizeTableColumnsSlot(void)
{
    m_userInterface->tableWidget->resizeColumnToContents(COLUMN_SCRIPT_THREAD_STATUS);


    m_userInterface->tableWidget->setColumnWidth(COLUMN_UI_PATH, m_userInterface->tableWidget->width() -
                                                 (m_userInterface->tableWidget->columnWidth(COLUMN_NAME) + m_userInterface->tableWidget->columnWidth(COLUMN_SCRIPT_THREAD_STATUS)
                                                  + m_userInterface->tableWidget->columnWidth(COLUMN_SCRIPT_PATH)
                                                  + 2 *m_userInterface->tableWidget->frameWidth()
                                                  + m_userInterface->tableWidget->verticalHeader()->width()
                                                  + (m_userInterface->tableWidget->verticalScrollBar()->isVisible() ?
                                                         m_userInterface->tableWidget->verticalScrollBar()->width() : 0)));
}

/**
 * Creates a new script table row.
 */
void ScriptWindow::createNewTableRow()
{
    m_userInterface->tableWidget->blockSignals(true);
    m_userInterface->tableWidget->insertRow(0);

    m_userInterface->tableWidget->setItem(0, 0, new QTableWidgetItem(QString("%1").arg(m_userInterface->tableWidget->rowCount())));

    QTableWidgetItem* item = new QTableWidgetItem();
    item->setFlags(item->flags() ^ Qt::ItemIsEditable);
    m_userInterface->tableWidget->setItem(0, 1, item);

    item = new QTableWidgetItem();
    item->setFlags(item->flags() ^ Qt::ItemIsEditable);
    m_userInterface->tableWidget->setItem(0, 2, item);

    item = new QTableWidgetItem();
    item->setFlags(item->flags() ^ Qt::ItemIsEditable);
    m_userInterface->tableWidget->setItem(0, 3, item);

    QStringList list;

    for(int i = 0; i <  m_userInterface->tableWidget->rowCount(); i++)
    {
        list << "    ";
    }
    m_userInterface->tableWidget->setVerticalHeaderLabels(list);
    m_userInterface->tableWidget->clearSelection();
    m_userInterface->tableWidget->blockSignals(false);
    m_userInterface->tableWidget->selectRow(0);

}

/**
 * Sets the window title.
 * @param extraString
 *      The string which is appended at the title.
 */
void ScriptWindow::setTitle(QString extraString)
{
    setWindowTitle("ScriptCommunicator " + MainWindow::VERSION + " - Scripts " + extraString);
}

/**
 * Loads the saved script table content from file.
 */
void ScriptWindow::loadTableData(void)
{

    setTitle(m_currentScriptConfigFileName);

    m_userInterface->tableWidget->setRowCount(0);

    if(!m_currentScriptConfigFileName.isEmpty())
    {
        QFile file(m_currentScriptConfigFileName);
        QDomDocument doc("scripts");

        if (file.open(QFile::ReadOnly))
        {
            file.close();

            if (!doc.setContent(&file))
            {
                if(!file.readAll().isEmpty())
                {
                    QMessageBox::critical(this, "parse error", "could not parse " + m_currentScriptConfigFileName);

                    m_currentScriptConfigFileName = "";
                    setTitle(m_currentScriptConfigFileName);
                    emit configHasToBeSavedSignal();
                }
            }
            else
            {
                QDomElement docElem = doc.documentElement();

                QDomNodeList itemList = docElem.elementsByTagName("fileInfo");
                QDomNode nodeItem = itemList.at(0);
                bool hasBeenSaved = nodeItem.attributes().namedItem("hasBeenSaved").nodeValue().toUInt();

                itemList = docElem.elementsByTagName("columnWidth");
                nodeItem = itemList.at(0);


                quint32 width = nodeItem.attributes().namedItem("width0").nodeValue().toUInt();
                if(width != 0) m_userInterface->tableWidget->setColumnWidth(0, width);

                width = nodeItem.attributes().namedItem("width1").nodeValue().toUInt();
                if(width != 0) m_userInterface->tableWidget->setColumnWidth(1, width);

                width = nodeItem.attributes().namedItem("width2").nodeValue().toUInt();
                if(width != 0) m_userInterface->tableWidget->setColumnWidth(2, width);

                QDomNodeList nodeList = docElem.elementsByTagName("script");
                blockSignals(true);
                for (int x = nodeList.size() - 1; x >= 0; x--)
                {
                    QDomNode nodeScriptItem = nodeList.at(x);

                    createNewTableRow();

                    m_userInterface->tableWidget->item(0, COLUMN_NAME)->setText(nodeScriptItem.attributes().namedItem("name").nodeValue());

                    QString path = nodeScriptItem.attributes().namedItem("path").nodeValue();
                    if(m_mainWindow->isFirstProgramStart() || !hasBeenSaved)
                    {
                        if(path.startsWith("./")){path.replace("./", MainWindow::getScriptCommunicatorFilesFolder() + "/");}
                    }
                    else
                    {
                        path = MainWindow::convertToAbsolutePath(m_currentScriptConfigFileName, path);
                    }
                    m_userInterface->tableWidget->item(0, COLUMN_SCRIPT_PATH)->setText(path);

                    QString ui = nodeScriptItem.attributes().namedItem("ui").nodeValue();
                    if(m_mainWindow->isFirstProgramStart() || !hasBeenSaved)
                    {
                        if(ui.startsWith("./")){ui.replace("./", MainWindow::getScriptCommunicatorFilesFolder() + "/");}
                    }
                    else
                    {
                        ui = MainWindow::convertToAbsolutePath(m_currentScriptConfigFileName, ui);
                    }
                    m_userInterface->tableWidget->item(0 ,COLUMN_UI_PATH)->setText(ui);

                    quint32 rowHeight = nodeScriptItem.attributes().namedItem("height").nodeValue().toUInt();

                    if(rowHeight != 0)
                    {
                        m_userInterface->tableWidget->setRowHeight(0, rowHeight);

                    }

                    QString status = nodeScriptItem.attributes().namedItem("status").nodeValue();

                    if(status == "suspended")
                    {
                        status = "not running";
                    }
                    m_userInterface->tableWidget->item(0,COLUMN_SCRIPT_THREAD_STATUS)->setText(status);


                    if(status == "running")
                    {
                        startScriptThread(0);
                    }
                    else if(status == "paused")
                    {
                        startScriptThread(0);
                        pauseScriptThread(0);
                        m_userInterface->tableWidget->item(0, COLUMN_SCRIPT_THREAD_STATUS)->setText("paused");
                    }

                }
                blockSignals(false);

                resizeTableColumnsSlot();
                emit scriptTableHasChangedSignal();

                QStringList showStrList = m_currentScriptConfigFileName.split("/");
                statusBar()->showMessage(showStrList[showStrList.size() - 1] + " loaded", 3000);


                m_currentScriptConfigFileString = tableToString();
                itemSelectionChangedSlot();

                if(!hasBeenSaved)
                {
                    saveConfigSlot();
                }
            }
        }
        else
        {
            QMessageBox::critical(this, "could not open file", m_currentScriptConfigFileName);

            m_currentScriptConfigFileName = "";
            setTitle(m_currentScriptConfigFileName);
            emit configHasToBeSavedSignal();
        }

        resizeTableColumnsSlot();
        emit scriptTableHasChangedSignal();

    }
}

/**
 * Saves the script table content to file.
 */
void ScriptWindow::saveTable(void)
{
    QFile file(m_currentScriptConfigFileName);
    file.remove();
    if(file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QTextStream data( &file );
        data.setCodec("UTF-8");
        m_currentScriptConfigFileString = tableToString();
        data << m_currentScriptConfigFileString;
        file.close();

        QStringList showStrList = m_currentScriptConfigFileName.split("/");
        statusBar()->showMessage(showStrList[showStrList.size() - 1] + " saved", 3000);
    }
    else
    {
        QMessageBox::critical(this, "save failed", m_currentScriptConfigFileName);

        m_currentScriptConfigFileName = "";
        setTitle(m_currentScriptConfigFileName);
        emit configHasToBeSavedSignal();
    }
}


/**
 * This slot function is called if the user double clicks a cell in the script table.
 * @param row
 *      The row of the cell.
 * @param column
 *      The column of the cell.
 */
void ScriptWindow::cellDoubleClickedSlot(int row, int column)
{
    if(column == COLUMN_SCRIPT_PATH)
    {//script file path column
        QString tmpFileName = QFileDialog::getOpenFileName(this, tr("Open worker script file"),
                                                           "", tr("scipt files (*.js);;Files (*)"));
        if(!tmpFileName.isEmpty())
        {
            m_userInterface->tableWidget->item(row,column)->setText(tmpFileName);
        }
    }
    else if(column == COLUMN_UI_PATH)
    {//ui file path column

        QString tmpFileName = QFileDialog::getOpenFileName(this, tr("Open user interface file"),
                                                           "",tr("ui files (*.ui);;Files (*)"));

        if(!tmpFileName.isEmpty())
        {
            m_userInterface->tableWidget->item(row,column)->setText(tmpFileName);
        }
    }

    itemSelectionChangedSlot();
}

/**
 * This slot function is called if the cell in the script table has changed.
 * @param row
 *      The row of the cell.
 * @param column
 *      The column of the cell.
 */
void ScriptWindow::cellChangedSlot(int row, int column)
{
    (void) row;
    (void) column;
    resizeTableColumnsSlot();
    itemSelectionChangedSlot();
    emit scriptTableHasChangedSignal();
}


/**
 * Adds a script to the script table.
 * @param fileName
 *      The script name.
 */
void ScriptWindow::addScript(QString fileName)
{
    createNewTableRow();

    QFileInfo fileInfo (fileName);
    m_userInterface->tableWidget->item(0, COLUMN_NAME)->setText(fileInfo.baseName());
    m_userInterface->tableWidget->item(0, COLUMN_SCRIPT_PATH)->setText(fileName);
    m_userInterface->tableWidget->item(0, COLUMN_SCRIPT_THREAD_STATUS)->setText("not running");


    QString uiFile = fileInfo.absolutePath() + "/" + fileInfo.completeBaseName()+ ".ui";
    if(QFile::exists(uiFile))
    {
        m_userInterface->tableWidget->item(0, COLUMN_UI_PATH)->setText(uiFile);
    }
}

/**
 * Script table drop event function.
 * @param row
 *      The row of the drop event.
 * @param column
 *      The column of the drop event.
 * @param files
 *      The file from the drop event.
 */
void ScriptWindow::tableDropEventSlot(int row, int column, QStringList files)
{
    (void)row;
    (void)column;

    for(auto el : files)
    {
        if(!el.isEmpty())
        {
#ifdef Q_OS_MAC
            addScript("/" + el);
#else
            addScript(el);
#endif
        }
    }
}

/**
 * Slot function for the add script menu.
 */
void ScriptWindow::addScriptSlot()
{
    QString tmpFileName = QFileDialog::getOpenFileName(this, tr("Open worker script file"),
                                                       "", tr("script files (*.js);;Files (*)"));
    if(!tmpFileName.isEmpty())
    {
        addScript(tmpFileName);
    }
}

/**
 * Slot function for the remove script menu.
 */
void ScriptWindow::removeScriptSlot()
{
    for(auto it : m_userInterface->tableWidget->selectedItems())
    {
        stopScriptThread(it->row());
        m_userInterface->tableWidget->selectRow(it->row() - 1);
        m_userInterface->tableWidget->removeRow(it->row());
        break;
    }
    emit scriptTableHasChangedSignal();
}

/**
 * Slot function for the load config menu.
 */
void ScriptWindow::loadConfigSlot()
{
    QString tmpFileName = QFileDialog::getOpenFileName(this, tr("Open script config file"),
                                                       m_mainWindow->getAndCreateProgramUserFolder(), tr("script config files (*.scripts);;Files (*)"));
    if(!tmpFileName.isEmpty())
    {
        stopAllScripts();
        while(!allScriptsHaveBeenStopped())
        {
            QThread::msleep(10);
            QCoreApplication::processEvents();
        }

        m_currentScriptConfigFileName = tmpFileName;
        emit configHasToBeSavedSignal();
        setTitle(m_currentScriptConfigFileName);
        m_userInterface->tableWidget->setRowCount(0);
        loadTableData();
    }
}

/**
 * Checks if the script table has been changed and saves the table if necessary.
 */
void ScriptWindow::checkTableChanged()
{

    if(tableToString() != m_currentScriptConfigFileString)
    {
        if(m_currentScriptConfigFileName.isEmpty())
        {
            QMessageBox messageBox(QMessageBox::Question, "scripts config changed",
                                   "Save scripts config file?", QMessageBox::Yes | QMessageBox::No);
            messageBox.exec();
            if(messageBox.result() == QMessageBox::Yes)
            {
                saveConfigAsSlot();
            }
            else
            {
                m_currentScriptConfigFileString = "";
                setTitle(m_currentScriptConfigFileName);
            }
        }
        else
        {
            saveTable();
        }
    }
}

/**
 * Slot function for the unload config menu.
 */
void ScriptWindow::unloadConfigSlot()
{
    stopAllScripts();
    while(!allScriptsHaveBeenStopped())
    {
        QThread::msleep(10);
        QCoreApplication::processEvents();
    }

    m_userInterface->tableWidget->setRowCount(0);
    m_currentScriptConfigFileString = "";
    m_currentScriptConfigFileName = "";
    setTitle(m_currentScriptConfigFileName);
    emit configHasToBeSavedSignal();
    emit scriptTableHasChangedSignal();
}

/**
 * Slot function for the save config menu.
 */
void ScriptWindow::saveConfigSlot()
{
    if(!m_currentScriptConfigFileName.isEmpty())
    {
        saveTable();
    }
    else
    {
        saveConfigAsSlot();
    }
}
/**
 * Slot function for the edit all worker scripts menu.
 */
void ScriptWindow::editAllWorkerScriptsSlot()
{
    if( m_userInterface->tableWidget->rowCount() != 0)
    {
        QStringList arguments;
        for( int r = 0; r < m_userInterface->tableWidget->rowCount(); ++r )
        {
            QString script = m_userInterface->tableWidget->item(r, COLUMN_SCRIPT_PATH)->text();
            if(!script.isEmpty())
            {
                arguments << script;
            }
        }
        MainWindow::openScriptEditor(arguments, m_mainWindow->getSettingsDialog()->settings(), this);
    }
}

/**
 * Slot function for the remove ui menu.
 */
void ScriptWindow::removeUi()
{
    int selectedRow = (m_userInterface->tableWidget->selectedItems().isEmpty())? -1 : m_userInterface->tableWidget->selectedItems()[0]->row();

    if(selectedRow != -1)
    {
        m_userInterface->tableWidget->item(selectedRow, COLUMN_UI_PATH)->setText("");
    }

    itemSelectionChangedSlot();
}

/**
 * Slot function for the edit ui menu (opens the qt designer with the ui file which
 * belongs the the current selected row).
 */
void ScriptWindow::editUiSlot()
{
    int selectedRow = (m_userInterface->tableWidget->selectedItems().isEmpty())? -1 : m_userInterface->tableWidget->selectedItems()[0]->row();

    if(selectedRow != -1)
    {
        QString program;
#ifdef Q_OS_LINUX
        program = QCoreApplication::applicationDirPath() + "/designer";
#elif defined Q_OS_MAC

        QFileInfo fi("/Applications/Qt Creator.app");
        if(fi.exists())
        {
            QStringList arguments;
            arguments << m_userInterface->tableWidget->item(selectedRow, COLUMN_UI_PATH)->text() ;

            QProcess *myProcess = new QProcess(this);
            myProcess->start("/Applications/Qt Creator.app/Contents/MacOS/Qt Creator", arguments);

            myProcess->waitForFinished(10000);
            if(myProcess->exitCode() != 0)
            {
                QMessageBox::critical(this, "error starting QtCreator", "could not start QtCreator");
            }

        }
        else
        {
            QString text = "To edit a user interface you need QtCreator.";
            text.append("<br>Download QtCreator from here: <a href=\"http://sourceforge.net/projects/scriptcommunicator/files/Mac%20OS%20X/qt-creator-opensource-mac-x86_64-3.6.0.dmg/download\">");
            text.append("http://sourceforge.net/projects/scriptcommunicator/files/Mac%20OS%20X/qt-creator-opensource-mac-x86_64-3.6.0.dmg/download</a>");
            text.append("<br>Open the dmg file and copy the app to the Applications folder.");
            QMessageBox::critical(this, "could not find QtCreator", text);
        }

        return;
#else
        program = QCoreApplication::applicationDirPath() + "/designer.exe";
#endif
        QStringList arguments;
        arguments << m_userInterface->tableWidget->item(selectedRow, COLUMN_UI_PATH)->text() ;

        QProcess *myProcess = new QProcess(this);
        bool processCreated = false;

        if(m_userInterface->tableWidget->item(selectedRow, COLUMN_UI_PATH)->text().size() != 0)
        {
            processCreated = myProcess->startDetached(program, arguments);
        }
        else
        {
            processCreated = myProcess->startDetached(program);
        }

        if(!processCreated)
        {
            QMessageBox::critical(this, "error starting QtDesigner", "could not start QtDesigner ");
        }
    }
}

/**
 * Slot function for the save as config menu.
 */
void ScriptWindow::saveConfigAsSlot()
{
    QString tmpFileName = QFileDialog::getSaveFileName(this, tr("Save script config file"),
                                                       m_mainWindow->getAndCreateProgramUserFolder(), tr("script config files (*.scripts);;Files (*)"));
    if(!tmpFileName.isEmpty())
    {
        m_currentScriptConfigFileName = tmpFileName;
        setTitle(m_currentScriptConfigFileName);
        emit configHasToBeSavedSignal();
        saveTable();
    }
}


/**
 * Slot function for the create sce file menu.
 */
void ScriptWindow::createSceFileSlot()
{
    m_createSceFileDialog->show();
}

/**
 * Converts the script table to a string (XML).
 * @return
 *      The created string.
 */
QString ScriptWindow::tableToString(void)
{
    QString result;

    if(m_userInterface->tableWidget->rowCount() > 0)
    {
        QBuffer xmlBuffer;

        QXmlStreamWriter xmlWriter;
        xmlWriter.setAutoFormatting(true);
        xmlWriter.setAutoFormattingIndent(2);
        xmlBuffer.open(QIODevice::WriteOnly);
        xmlWriter.setDevice(&xmlBuffer);

        xmlWriter.writeStartElement("ScriptConfig");

        xmlWriter.writeStartElement("fileInfo");
        xmlWriter.writeAttribute("version", MainWindow::VERSION);
        xmlWriter.writeAttribute("hasBeenSaved", "1");
        xmlWriter.writeEndElement();


        xmlWriter.writeStartElement("columnWidth");
        xmlWriter.writeAttribute("width0", QString("%1").arg(m_userInterface->tableWidget->columnWidth(0)));
        xmlWriter.writeAttribute("width1", QString("%1").arg(m_userInterface->tableWidget->columnWidth(1)));
        xmlWriter.writeAttribute("width2", QString("%1").arg(m_userInterface->tableWidget->columnWidth(2)));
        xmlWriter.writeEndElement();//"columnWidth"

        for( int r = 0; r < m_userInterface->tableWidget->rowCount(); ++r )
        {

            QTableWidgetItem* item1 = m_userInterface->tableWidget->item(r, COLUMN_NAME);
            QTableWidgetItem* item2 = m_userInterface->tableWidget->item(r, COLUMN_SCRIPT_THREAD_STATUS);
            QTableWidgetItem* item3 = m_userInterface->tableWidget->item(r, COLUMN_SCRIPT_PATH);
            QTableWidgetItem* item4 = m_userInterface->tableWidget->item(r, COLUMN_UI_PATH);

            if(item1 && item2)
            {
                xmlWriter.writeStartElement("script");

                xmlWriter.writeAttribute("name", item1->text());
                xmlWriter.writeAttribute("status", item2->text());
                xmlWriter.writeAttribute("path", MainWindow::convertToRelativePath(m_currentScriptConfigFileName, item3->text()));
                xmlWriter.writeAttribute("ui", MainWindow::convertToRelativePath(m_currentScriptConfigFileName, item4->text()));
                xmlWriter.writeAttribute("height", QString("%1").arg(m_userInterface->tableWidget->rowHeight(r)));

                xmlWriter.writeEndElement();//"script"
            }
        }

        xmlWriter.writeEndElement();//"scripts"

        result =  xmlBuffer.data();
    }

    return result;
}

/**
 * This slot function can be used to stop a script.
 * @param threadToStop
 *      Pointer to the thread which has to be stopped.
 */
void ScriptWindow::stopScriptThreadSlot(ScriptThread* threadToStop)
{
    quint32 row = 0;
    if(findRow(threadToStop, row))
    {
        stopScriptThread(row);
        itemSelectionChangedSlot();
    }
    else
    {
        appendTextToConsoleSlot(QString("invalid thread in ") + Q_FUNC_INFO);
    }

}

/**
 * Returns the row to which the given script thread is connected.
 * @param threadToFind
 *      The script thread.
 * @param row
 *      The row to which the given script thread is connected.
 * @return
 *      True for success.
 */
bool ScriptWindow::findRow(ScriptThread* threadToFind, quint32& row)
{
    bool rowFound = false;

    for (int32_t i = 0; i < m_userInterface->tableWidget->rowCount(); i++)
    {
        ScriptThread* thread = (ScriptThread*)m_userInterface->tableWidget->item(i, COLUMN_SCRIPT_THREAD_POINTER)
                ->data(USER_ROLE_SCRIPT_THREAD_IN_TABLE).toULongLong();

        if (thread == 0)
        {
            rowFound = false;
        }
        else
        {
            if (thread == threadToFind)
            {
                rowFound = true;
                row = i;
                break;
            }
        }
    }

    return rowFound;
}


/**
 * Returns the script-table (script window) name of the calling script.
 * @param scriptName
 *      The name. In case of an error an empty string is returned.
 */
void ScriptWindow::getScriptTableNameSlot(QString* scriptName)
{
    *scriptName= "";
    ScriptThread* senderThread = dynamic_cast<ScriptThread*>(sender());

    for( int r = 0; r < m_userInterface->tableWidget->rowCount(); r++ )
    {
        ScriptThread* currentThread = (ScriptThread*)m_userInterface->tableWidget->item(r, COLUMN_SCRIPT_THREAD_POINTER
                                                                                        )->data(USER_ROLE_SCRIPT_THREAD_IN_TABLE).toULongLong();
        if(senderThread == currentThread)
        {
            *scriptName = m_userInterface->tableWidget->item(r,COLUMN_NAME)->text();
        }

    }
}

/**
 * Sets the state of a script (running, paused or stopped).
 * Note: The script must be in the script table (script window) and a script can not
 *       set it's own state.
 * @param state
 *      The state in which the script shall be switched:
 *      - 0: running
 *      - 1: paused
 *      - 2: stopped
 * @param scriptTableEntryName
 *      The name of the script in the script-table (script window).
 * @param result
 *      True if scriptTableEntryName has been found in the script-table and the state has a valid value.
 *
 */
void ScriptWindow::setScriptStateSlot(quint8 state, QString scriptTableEntryName, bool* result)
{
    *result = false;

    for( int r = 0; r < m_userInterface->tableWidget->rowCount(); r++ )
    {
        if(m_userInterface->tableWidget->item(r,COLUMN_NAME)->text() == scriptTableEntryName)
        {//Script found.

            ScriptThread* currentThread = (ScriptThread*)m_userInterface->tableWidget->item(r, COLUMN_SCRIPT_THREAD_POINTER
                                                                                            )->data(USER_ROLE_SCRIPT_THREAD_IN_TABLE).toULongLong();

            ScriptThread* senderThread = dynamic_cast<ScriptThread*>(sender());

            if(senderThread != currentThread)
            {
                *result = true;

                if(state == 0)
                {//running
                    startScriptThread(r);
                }
                else if(state == 1)
                {//paused

                    if(currentThread)
                    {//The script is running or is paused.

                        pauseScriptThread(r);
                    }
                    else
                    {//The script is stopped.
                        startScriptThread(r);
                        QThread::msleep(100);
                        pauseScriptThread(r);
                    }
                }
                else if(state == 2)
                {//stopped
                    stopScriptThread(r);
                }
                else
                {
                    *result = false;
                }
            }//if(senderThread != currentThread)

            break;
        }//if(m_userInterface->tableWidget->item(r,COLUMN_NAME)->text() == scriptTableEntryName)
    }
}

/**
 * Returns the names and the states of all scripts.
 */
QStringList ScriptWindow::getAllScriptNamesAndStates(void)
{
    QStringList scripts;

    for( int r = 0; r < m_userInterface->tableWidget->rowCount(); ++r )
    {
        scripts << m_userInterface->tableWidget->item(r,COLUMN_NAME)->text();
        scripts << m_userInterface->tableWidget->item(r,COLUMN_SCRIPT_THREAD_STATUS)->text();
    }

    return scripts;
}

/**
 * Checks if ScriptCommunicatormust exit.
 */
void ScriptWindow::checkIfScriptCommunicatorMustExit()
{
    if(!m_commandLineScripts.isEmpty())
    {
        bool allScriptsStopped = true;
        for (int32_t i = 0; i < m_userInterface->tableWidget->rowCount(); i++)
        {
            ScriptThread* thread = (ScriptThread*)m_userInterface->
                    tableWidget->item(i, COLUMN_SCRIPT_THREAD_POINTER)->data(USER_ROLE_SCRIPT_THREAD_IN_TABLE).toULongLong();

            if(thread != 0)
            {
                allScriptsStopped = false;
                break;
            }
        }
        if(allScriptsStopped && !this->isVisible())
        {
            deleteCurrentScezFolder();

            QCoreApplication::exit(m_exitCode);
            exit(m_exitCode);
        }
    }
}

/**
 * This slot function has to be called the state of one script thread has changed.
 * @param state
 *      The thread state.
 * @param thread
 *      The script thread.
 */
void ScriptWindow::threadStateChangedSlot(ThreadSate state, ScriptThread* thread)
{
    quint32 row = 0;
    bool threadIsInTable = findRow(thread, row);

    if(threadIsInTable || (state == EXITED))
    {
        QString text;

        if(state == INVALID)
        {
            text = "not started";
        }
        else if(state == CREATED)
        {
            text = "running";
        }
        else if(state == RUNNING)
        {
            text = "running";
        }
        else if(state == PAUSED)
        {
            text = "paused";
        }
        else if(state == SUSPENDED_BY_DEBUGGER)
        {
            text = "suspended";
        }
        else if(state == EXITED)
        {
            text = "not running";
            if(thread)
            {
                if(threadIsInTable)
                {
                    m_userInterface->tableWidget->item(row, COLUMN_SCRIPT_THREAD_POINTER)->setData(USER_ROLE_SCRIPT_THREAD_IN_TABLE, (quint64)0);
                }

                delete thread;
            }

            m_mainWindow->removeAllTabsAndToolBoxPages(thread);
        }

        if(m_userInterface->tableWidget->rowCount() > 0)
        {

            if(threadIsInTable)
            {
                m_userInterface->tableWidget->item(row, COLUMN_SCRIPT_THREAD_STATUS)->setText(text);
                itemSelectionChangedSlot();
            }

            checkIfScriptCommunicatorMustExit();
        }
    }


}

/**
 * This function pauses a script thread.
 * @param selectedRow
 *      The row to which the script thread belongs.
 */
void ScriptWindow::pauseScriptThread(int selectedRow)
{
    ScriptThread* thread = (ScriptThread*)m_userInterface->tableWidget->item(selectedRow, COLUMN_SCRIPT_THREAD_POINTER)
            ->data(USER_ROLE_SCRIPT_THREAD_IN_TABLE).toULongLong();

    if(thread)
    {
        thread->m_shallPause = true;
    }
    else
    {
        appendTextToConsoleSlot(QString("invalid thread pointer in ") + Q_FUNC_INFO);
    }

}

/**
 * Slot function for the pause button.
 */
void ScriptWindow::pauseButtonPressedSlot(void)
{
    int selectedRow = (m_userInterface->tableWidget->selectedItems().isEmpty())? -1 : m_userInterface->tableWidget->selectedItems()[0]->row();

    if(selectedRow != -1)
    {
        if(m_userInterface->pausePushButton->text() == "pause")
        {
            pauseScriptThread(selectedRow);
        }
        else
        {
            startScriptThread(selectedRow, true);
        }
    }
    else
    {
        QMessageBox::critical(this, "error starting script", "no script selected");
    }
}


/**
 * Returns the state of a script thread.
 * @param selectedRow
 *      The index of the script in the script table.
 * @return
 *      The script state.
 */
ThreadSate ScriptWindow::getScriptState(int selectedRow)
{
    ThreadSate state = EXITED;
    ScriptThread* currentThread = (ScriptThread*)m_userInterface->tableWidget->item(selectedRow, COLUMN_SCRIPT_THREAD_POINTER
                                                                                    )->data(USER_ROLE_SCRIPT_THREAD_IN_TABLE).toULongLong();
    if(currentThread)
    {
        state = currentThread->getThreadState();
    }

    return state;
}

/**
 * Starts a script thread
 * @param selectedRow
 *      The row to which the script thread belongs.
 */
void ScriptWindow::startScriptThread(int selectedRow, bool withDebugger)
{
    ScriptThread* currentThread = (ScriptThread*)m_userInterface->tableWidget->item(selectedRow, COLUMN_SCRIPT_THREAD_POINTER
                                                                                    )->data(USER_ROLE_SCRIPT_THREAD_IN_TABLE).toULongLong();
    if(!currentThread)
    {//This row has no thread.

        ScriptThread* thread = 0;
        QString scriptFileName = m_userInterface->tableWidget->item(selectedRow, COLUMN_SCRIPT_PATH)->text();

        if(m_userInterface->tableWidget->item(selectedRow, COLUMN_UI_PATH)->text() != 0)
        {//This script has a user interface.

            //Load the user interface and start the thread.
            QWidget* scriptUi = 0;
            QUiLoader loader;
            loader.clearPluginPaths();
            loader.addPluginPath(MainWindow::getPluginsFolder() + "/designer");

            //Add the extra paths.
            for(auto el : m_mainWindow->getExtraPluginPaths())
            {
                loader.addPluginPath(el);
            }

            QFile uiFile(m_userInterface->tableWidget->item(selectedRow, COLUMN_UI_PATH)->text());

            if(uiFile.open(QIODevice::ReadOnly))
            {
                scriptUi = loader.load(&uiFile);
                uiFile.close();

                thread = new ScriptThread(this, m_sendIdCounter,
                                          m_userInterface->tableWidget->item(selectedRow, COLUMN_SCRIPT_PATH)->text(),
                                          scriptUi, m_mainWindow->getSettingsDialog(), withDebugger);
            }
            else
            {
                m_userInterface->tableWidget->item(selectedRow,COLUMN_SCRIPT_THREAD_STATUS)->setText("not running");

                QMessageBox::critical(this, "error starting script", "could not open " +
                                      m_userInterface->tableWidget->item(selectedRow, COLUMN_UI_PATH)->text());

                delete thread;
            }
        }
        else
        {
            thread = new ScriptThread(this, m_sendIdCounter,scriptFileName,0,
                                      m_mainWindow->getSettingsDialog(), withDebugger);
        }

        if(thread != 0)
        {
            m_sendIdCounter++;
            m_userInterface->tableWidget->item(selectedRow, COLUMN_SCRIPT_THREAD_POINTER)
                    ->setData(USER_ROLE_SCRIPT_THREAD_IN_TABLE, (quint64)thread);


            m_userInterface->startPushButton->setEnabled(false);
            m_userInterface->stopPushButton->setEnabled(true);
            m_userInterface->pausePushButton->setEnabled(true);
            m_userInterface->pausePushButton->setText("pause");

            if(withDebugger)
            {
                thread->startDebugging();
            }
            else
            {
                thread->moveToThread(thread);
                thread->start();
            }

        }
    }//if(!currentThread)
    else
    {
        currentThread->m_shallPause = false;
    }
}

/**
 * This slot function creates a new script.
 */
void ScriptWindow::newScriptSlot(void)
{
    QString templateScriptFileName = QFileDialog::getOpenFileName(this, tr("Select worker script template"),
                                                                  MainWindow::getScriptCommunicatorFilesFolder() + "/templates/workerScriptTemplates",
                                                                  tr("worker script files (*.js);;Files (*)"));
    if(!templateScriptFileName.isEmpty())
    {
        QFileInfo fileInfo (templateScriptFileName);
        QString templateUiFile = fileInfo.absolutePath() + "/" + fileInfo.completeBaseName()+ ".ui";
        if(!templateScriptFileName.isEmpty())
        {

            QString scriptFileName = QFileDialog::getSaveFileName(this, tr("Select worker script name"),
                                                                  MainWindow::getScriptCommunicatorFilesFolder(), tr("worker script files (*.js);;Files (*)"));
            if(!scriptFileName.isEmpty())
            {
                QFile(scriptFileName).remove();

                if(QFile::copy(templateScriptFileName, scriptFileName))
                {

                    createNewTableRow();
                    QFileInfo fileInfo (scriptFileName);
                    m_userInterface->tableWidget->item(0, COLUMN_NAME)->setText(fileInfo.baseName());
                    m_userInterface->tableWidget->item(0, COLUMN_SCRIPT_PATH)->setText(scriptFileName);
                    m_userInterface->tableWidget->item(0, COLUMN_SCRIPT_THREAD_STATUS)->setText("not running");

                    if(QFile::exists(templateUiFile))
                    {

                        QString uiFile = fileInfo.absolutePath() + "/" + fileInfo.completeBaseName()+ ".ui";
                        QFile(uiFile).remove();

                        if(QFile::copy(templateUiFile, uiFile))
                        {
                            m_userInterface->tableWidget->item(0, COLUMN_UI_PATH)->setText(uiFile);
                        }
                        else
                        {
                            QMessageBox::critical(this, "error", QString("could not copy ui file %1 to %2").arg(templateUiFile, uiFile));
                        }
                    }
                }
                else
                {
                    QMessageBox::critical(this, "error", QString("could not copy script %1 to %2").arg(templateScriptFileName, scriptFileName));
                }
            }

        }//if(!templateScriptFileName.isEmpty())

    }//if(!templateScriptFileName.isEmpty())

}

/**
 * Slot function for the start button.
 */
void ScriptWindow::startButtonPressedSlot(void)
{

    int selectedRow = (m_userInterface->tableWidget->selectedItems().isEmpty())? -1 : m_userInterface->tableWidget->selectedItems()[0]->row();

    if(selectedRow != -1)
    {
        startScriptThread(selectedRow);
    }
    else
    {
        QMessageBox::critical(this, "error starting script", "no script selected");
    }
}

/**
 * Slot function for the move up menu.
 */
void ScriptWindow::moveTableEntryUpSlot(void)
{
    QList<QTableWidgetItem*> selectedItems = m_userInterface->tableWidget->selectedItems();
    if(!selectedItems.isEmpty())
    {
        int selectedRow = selectedItems[0]->row();
        if((selectedRow != -1) && (selectedRow != 0) )
        {
            swapTableRowPositions(selectedRow, selectedRow - 1);
            m_userInterface->tableWidget->clearSelection();
            m_userInterface->tableWidget->selectRow(selectedRow - 1);
        }
    }
}

/**
 * Slot function for the move up menu.
 */
void ScriptWindow::moveTableEntryDownSlot(void)
{
    QList<QTableWidgetItem*> selectedItems = m_userInterface->tableWidget->selectedItems();
    if(!selectedItems.isEmpty())
    {
        int selectedRow = selectedItems[0]->row();
        if((selectedRow != -1) && (selectedRow != (m_userInterface->tableWidget->rowCount() - 1)))
        {
            swapTableRowPositions(selectedRow, selectedRow + 1);
            m_userInterface->tableWidget->clearSelection();
            m_userInterface->tableWidget->selectRow(selectedRow + 1);
        }
    }
}

/**
 * This slot function is called if the selection in the script table has been changed.
 */
void ScriptWindow::itemSelectionChangedSlot(void)
{
    int selectedRow = (m_userInterface->tableWidget->selectedItems().isEmpty())? -1 : m_userInterface->tableWidget->selectedItems()[0]->row();

    m_userInterface->actionEditAllWorkerScripts->setEnabled(false);
    for( int r = 0; r < m_userInterface->tableWidget->rowCount(); ++r )
    {
        if(!m_userInterface->tableWidget->item(r, COLUMN_SCRIPT_PATH)->text().isEmpty())
        {
            m_userInterface->actionEditAllWorkerScripts->setEnabled(true);
            break;
        }
    }


    if(selectedRow != -1)
    {
        ScriptThread* thread = (ScriptThread*)m_userInterface->tableWidget->item(selectedRow, COLUMN_SCRIPT_THREAD_POINTER)
                ->data(USER_ROLE_SCRIPT_THREAD_IN_TABLE).toULongLong();

        m_userInterface->actionMoveUp->setEnabled(true);
        m_userInterface->actionMoveDown->setEnabled(true);

        if(selectedRow == 0)
        {
            m_userInterface->actionMoveUp->setEnabled(false);
        }

        if(selectedRow == (m_userInterface->tableWidget->rowCount() - 1))
        {
            m_userInterface->actionMoveDown->setEnabled(false);
        }

        if(thread)
        {
            if(thread->getThreadState() == CREATED)
            {
                m_userInterface->startPushButton->setEnabled(false);
                m_userInterface->startPushButton->setText("start");

                m_userInterface->stopPushButton->setEnabled(true);
                m_userInterface->pausePushButton->setEnabled(false);
                m_userInterface->pausePushButton->setText("pause");
            }
            else if(thread->getThreadState() == RUNNING)
            {
                m_userInterface->startPushButton->setEnabled(false);
                m_userInterface->startPushButton->setText("start");

                m_userInterface->stopPushButton->setEnabled(true);
                m_userInterface->pausePushButton->setEnabled(true);
                m_userInterface->pausePushButton->setText("pause");
            }
            else if(thread->getThreadState() == PAUSED)
            {
                m_userInterface->startPushButton->setEnabled(true);
                m_userInterface->startPushButton->setText("run");

                m_userInterface->stopPushButton->setEnabled(true);
                m_userInterface->pausePushButton->setEnabled(false);
                m_userInterface->pausePushButton->setText("pause");
            }
            else if(thread->getThreadState() == SUSPENDED_BY_DEBUGGER)
            {
                m_userInterface->startPushButton->setEnabled(false);
                m_userInterface->stopPushButton->setEnabled(false);
                m_userInterface->pausePushButton->setEnabled(false);
            }
            else
            {//INVALID, EXITED
                m_userInterface->startPushButton->setEnabled(true);
                m_userInterface->startPushButton->setText("start");

                m_userInterface->stopPushButton->setEnabled(false);
                m_userInterface->pausePushButton->setEnabled(true);
                m_userInterface->pausePushButton->setText("debug");
            }
        }// if(thread)
        else
        {
            m_userInterface->startPushButton->setEnabled(true);
            m_userInterface->startPushButton->setText("start");

            m_userInterface->stopPushButton->setEnabled(false);
            m_userInterface->pausePushButton->setEnabled(true);
            m_userInterface->pausePushButton->setText("debug");
        }

        if(m_userInterface->tableWidget->item(selectedRow, COLUMN_UI_PATH)->text().size() == 0)
        {
            m_userInterface->actionEditUi->setText("New ui");
            m_userInterface->actionEditUi->setShortcut(QKeySequence("Ctrl+Shift+N"));
            m_userInterface->actionRemoveUi->setEnabled(false);
        }
        else
        {
            m_userInterface->actionEditUi->setText("Edit ui");
            m_userInterface->actionEditUi->setShortcut(QKeySequence("Ctrl+Shift+E"));
            m_userInterface->actionRemoveUi->setEnabled(true);

        }

        m_userInterface->actionEditUi->setEnabled(true);

        if(m_userInterface->tableWidget->item(selectedRow, COLUMN_SCRIPT_PATH)->text().size() != 0)
        {
            m_userInterface->actionEditScript->setEnabled(true);
            m_userInterface->actionRemoveScripts->setEnabled(true);
        }
        else
        {
            m_userInterface->actionEditScript->setEnabled(false);
            m_userInterface->actionRemoveScripts->setEnabled(false);
        }
    }
    else
    {
        m_userInterface->actionRemoveUi->setEnabled(false);
        m_userInterface->actionEditUi->setEnabled(false);
        m_userInterface->startPushButton->setEnabled(false);
        m_userInterface->stopPushButton->setEnabled(false);
        m_userInterface->pausePushButton->setEnabled(false);
        m_userInterface->pausePushButton->setText("debug");
        m_userInterface->actionEditScript->setEnabled(false);
        m_userInterface->actionRemoveScripts->setEnabled(false);
        m_userInterface->actionMoveUp->setEnabled(false);
        m_userInterface->actionMoveDown->setEnabled(false);
    }
}

/**
 * Slot function for the edit script button.
 */
void ScriptWindow::editScriptButtonPressedSlot(void)
{
    int selectedRow = (m_userInterface->tableWidget->selectedItems().isEmpty())? -1 : m_userInterface->tableWidget->selectedItems()[0]->row();

    if(selectedRow != -1 )
    {
        QStringList arguments;
        arguments << m_userInterface->tableWidget->item(selectedRow, COLUMN_SCRIPT_PATH)->text();
        MainWindow::openScriptEditor(arguments, m_mainWindow->getSettingsDialog()->settings(), this);
    }
}

/**
 * This function exits ScriptCommunicator.
 * @param exitCode
 *      The exit code of ScriptCommunicator.
 */
void ScriptWindow::exitScriptCommunicatorSlot(qint32 exitCode)
{
    m_exitCode = exitCode;
    m_mainWindow->setExitCodeFromScript(exitCode);

    if(!m_commandLineScripts.isEmpty())
    {
        if(!this->isHidden())
        {
            close();
        }

        stopAllScripts();
    }
    else
    {
        m_mainWindow->exitScriptCommunicator();
    }
}

/**
 * This slot function append text to the console.
 * @param text
 *      The text which has to be appended to the console.
 * @param newLine
 *      True if a new line should be appended.
 * @param bringToForeground
 *      True if the script window shall be on top of all windows.
 */
void ScriptWindow::appendTextToConsoleSlot(QString text, bool newLine, bool bringToForeground)
{
    if(newLine)
    {
        m_userInterface->scriptConsole->append(text);
    }
    else
    {
        m_userInterface->scriptConsole->insertPlainText(text);
    }
    limtCharsInTextEditSlot(m_userInterface->scriptConsole, MAX_CHARS_IN_CONSOLE);

    if(bringToForeground)
    {
        if(isVisible())
        {
            setWindowState( (windowState() & ~Qt::WindowMinimized) | Qt::WindowActive);
        }
        else
        {
            show();
        }
        raise();  // for MacOS
        activateWindow(); // for Windows
    }
}


/**
 * The function stops the script thread which is connected the a given row.
 * @param selectedRow
 *      The row to which the given script thread is connected.
 */
void ScriptWindow::stopScriptThread(int selectedRow)
{
    if(selectedRow != -1)
    {
        ScriptThread* thread = (ScriptThread*)m_userInterface->
                tableWidget->item(selectedRow, COLUMN_SCRIPT_THREAD_POINTER)->data(USER_ROLE_SCRIPT_THREAD_IN_TABLE).toULongLong();

        if(thread)
        {
            try
            {

                thread->m_shallExit = true;
                thread->exit(m_exitCode);

                //Wait until the script thread has exited.
                uint counter = 0;
                bool threadIsBlocked = false;

                do
                {
                    counter++;
                    if(counter > thread->m_blockTime)
                    {
                        threadIsBlocked = true;
                        break;
                    }
                    QThread::msleep(1);
                    QCoreApplication::processEvents();


                    thread = (ScriptThread*)m_userInterface->
                            tableWidget->item(selectedRow, COLUMN_SCRIPT_THREAD_POINTER)->data(USER_ROLE_SCRIPT_THREAD_IN_TABLE).toULongLong();
                    if(thread == 0)
                    {//The thread has already been deleted in threadStateChangedSlot.
                        break;
                    }
                }while(thread->isRunning());

                m_userInterface->tableWidget->item(selectedRow, COLUMN_SCRIPT_THREAD_POINTER)->setData(USER_ROLE_SCRIPT_THREAD_IN_TABLE, (quint32)0);

                if(threadIsBlocked)
                {

                    QMessageBox::critical(this, "error while stopping a script",
                                          "following script is blocked and will be terminated:" +
                                          m_userInterface->tableWidget->item(selectedRow, COLUMN_SCRIPT_PATH)->text() );

                    if(thread)
                    {
                        thread->terminateScriptThread();
                        m_mainWindow->removeAllTabsAndToolBoxPages(thread);
                    }

                    checkIfScriptCommunicatorMustExit();
                }

                m_userInterface->tableWidget->item(selectedRow, COLUMN_SCRIPT_THREAD_STATUS)->setText("not running");


            }
            catch(...)
            {
                QMessageBox::critical(this, "error while stopping script",
                                      "could not stop the current script" );
            }
        }//
    }//if(selectedRow != -1)
}

/**
 * Slot function for the stop button.
 */
void ScriptWindow::stopButtonPressedSlot(void)
{
    int selectedRow = (m_userInterface->tableWidget->selectedItems().isEmpty())? -1 : m_userInterface->tableWidget->selectedItems()[0]->row();
    if(selectedRow != -1)
    {
        stopScriptThread(selectedRow);

        m_userInterface->startPushButton->setEnabled(true);
        m_userInterface->stopPushButton->setEnabled(false);
        m_userInterface->pausePushButton->setEnabled(true);
        m_userInterface->pausePushButton->setText("debug");
    }
}

/**
 * Returns true if all scripts have been stopped.
 */
bool ScriptWindow::allScriptsHaveBeenStopped()
{
    bool stopped = true;

    for (int32_t i = 0; i < m_userInterface->tableWidget->rowCount(); i++)
    {
        ScriptThread* thread = (ScriptThread*)m_userInterface->tableWidget->item(i, COLUMN_SCRIPT_THREAD_POINTER)
                ->data(USER_ROLE_SCRIPT_THREAD_IN_TABLE).toULongLong();

        if(thread != 0)
        {
            stopped = false;
            break;
        }
    }


    return stopped;

}

/**
 * Stops all scripts.
 */
void ScriptWindow::stopAllScripts(void)
{
    if(m_commandLineScripts.isEmpty())
    {//No command line mode is active.
        checkTableChanged();
    }

    for (int32_t i = 0; i < m_userInterface->tableWidget->rowCount(); i++)
    {
        stopScriptThread(i);
    }
}

/**
 * This function is called if the window is closed.
 * @param event
 *      The close event.
 */
void ScriptWindow::closeEvent(QCloseEvent * event)
{
    if(!m_commandLineScripts.isEmpty())
    {//Command line mode is active.
        stopAllScripts();
    }

    event->accept();
}

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

#ifndef SCRIPTWINDOW_H
#define SCRIPTWINDOW_H

#include <QMainWindow>
#include "mainwindow.h"
#include <QCloseEvent>
#include "plotwindow.h"
#include <QUiLoader>
#include "mainInterfaceThread.h"
#include "scriptThread.h"
#include "scriptSlots.h"
#include "createSceFile.h"


namespace Ui {
class ScriptWindow;

}

class DragDropTableWidget : public QTableWidget
{
    Q_OBJECT

public:

    explicit DragDropTableWidget(QWidget* parent) : QTableWidget(parent)
    {

    }
signals:

    ///Drop event signal.
    void dropEventSignal(int row, int column, QStringList files);

protected:
    ///Drag enter event.
    void dragEnterEvent(QDragEnterEvent *event);

    ///Handles the data supplied by a drag and drop operation that ended with the given action in the given row and column.
    bool dropMimeData(int row, int column, const QMimeData *data, Qt::DropAction action);

    ///Returns a list of MIME types that can be used to describe a list of tablewidget items.
    QStringList mimeTypes() const;

    ///Returns the drop actions supported by this view.
    Qt::DropActions supportedDropActions () const;
};

///Class which represents the script window.
class ScriptWindow : public ScriptSlots
{
    Q_OBJECT
    friend class ScriptThread;

public:
    explicit ScriptWindow(MainWindow* mainWindow, MainInterfaceThread* thread, QStringList scripts);
    ~ScriptWindow();

    ///The max. number of chars in the console.
    static const quint32 MAX_CHARS_IN_CONSOLE = 1000000;


    ///Shows the script window.
    void show(void);

    ///Starts the command line scripts.
    void startCommandLineScripts(void);

    ///Returns the script config file name.
    QString getCurrentScriptConfigFileName(){return m_currentScriptConfigFileName;}

    ///Sets the script config file name.
    void setCurrentScriptConfigFileName(QString name){m_currentScriptConfigFileName = name;}

    ///This function is called if the window has to be resized.
    void resizeEvent(QResizeEvent * event);

    ///This function is called if the window is closed.
    void closeEvent(QCloseEvent * event);

    ///Stops all scripts.
    void stopAllScripts(void);

    ///Returns true if all scripts have been stopped.
    bool allScriptsHaveBeenStopped();

    ///Loads the saved script table content from file.
    void loadTableData(void);

    ///Sets the window title.
    void setTitle(QString extraString);

    ///Returns the splitter sizes.
    QList<int> getSplitterSizes(void);

    ///Sets the splitter sizes.
    void setSplitterSizes(QList<int> sizes);

    ///Returns the names and the states of all scripts.
    QStringList getAllScriptNamesAndStates(void);

    ///The function stops the script thread which is connected the a given row.
    void stopScriptThread(int selectedRow);

    ///Starts a script thread.
    void startScriptThread(int selectedRow, bool withDebugger=false);

    ///Returns the state of a script thread.
    ThreadSate getScriptState(int selectedRow);

    ///This function pauses a script thread.
    void pauseScriptThread(int selectedRow);

    ///Returns m_mainWindow.
    MainWindow* getMainWindow(){return m_mainWindow;}

    ///Returns m_mainInterfaceThread.
    MainInterfaceThread* getMainInterfaceThread(){return m_mainInterfaceThread;}

    ///Adds a script to the script table.
    void addScript(QString fileName);

    ///Closes the create sce file dialog.
    void closeCreateSceFileDialog(void){m_createSceFileDialog->close();}

    ///Returns m_createSceFileDialog;
    CreateSceFile* getCreateSceFileDialog(void){return m_createSceFileDialog;}

    ///User role value for the script thread in the script table.
    static const int  USER_ROLE_SCRIPT_THREAD_IN_TABLE = Qt::UserRole + 1;

    ///The name column (in the script table)
    static const int COLUMN_NAME = 0;

    ///The script thread status column (in the script table)
    static const int COLUMN_SCRIPT_THREAD_STATUS = 1;

    ///The script path column (in the script table)
    static const int COLUMN_SCRIPT_PATH = 2;

    ///The ui file path column (in the script table)
    static const int COLUMN_UI_PATH = 3;

    ///The column (in the script table) in which the script thread pointer a inserted.
    static const int COLUMN_SCRIPT_THREAD_POINTER = 1;

    ///Returns the tmp directory for a unsaved info files (created by ScriptEditor for script file which
    ///have unsaved changes).
    static QString getTmpDirectory(QString fileName){return QFileInfo(fileName).path() +"/.tmp";}

    ///Returns the unsaved info file name for a script file (created by ScriptEditor for script file which
    ///have unsaved changes)..
    static QString getUnsavedInfoFileName(QString fileName){return getTmpDirectory(fileName) + "/" + QFileInfo(fileName).fileName() + ".unsaved";}

signals:

    ///This signals is emitted if the global config (of the program) has to be saved.
    void configHasToBeSavedSignal(void);

    ///This signal is emitted if the script table has been changed.
    void scriptTableHasChangedSignal(void);

    ///Is emitted if a string in the global string map has been changed.
    void globalStringChangedSignal(QString* name, QString* string);

    ///Is emitted if a data vector in the global string data vector has been changed.
    void globalDataArrayChangedSignal(QString* name, QVector<unsigned char>* data);

    ///Is emitted if an unsigned number in the global unsigned number map has been changed
    void globalUnsignedChangedSignal(QString* name, quint32 number);

    ///Is emitted if a signed number in the global signed number map has been changed
    void globalSignedChangedSignal(QString* name, qint32 number);

    ///Is emitted if a real number in the global real number map has been changed
    void globalRealChangedSignal(QString* name, double number);

public slots:

    ///Returns the script-table (script window) name of the calling script.
    void getScriptTableNameSlot(QString* scriptName);

    ///Sets the state of a script (running, paused or stopped).
    void setScriptStateSlot(quint8 state, QString scriptTableEntryName, bool* result);

    ///This function exits ScriptCommunicator.
    void exitScriptCommunicatorSlot(qint32 exitCode);

    ///This slot function append text to the console.
    void appendTextToConsoleSlot(QString text, bool newLine=true, bool bringToForeground=false);

    ///This slot function has to be called the state of one script thread has changed.
    void threadStateChangedSlot(ThreadSate state, ScriptThread* thread);

    ///Slot function for the unload config menu.
    void unloadConfigSlot();

    ///Slot function for the save config menu.
    void saveConfigSlot();

    ///Slot function for the edit all worker scripts menu.
    void editAllWorkerScriptsSlot();

    ///Script table drop event function.
    void tableDropEventSlot(int row, int column, QStringList files);

private slots:

    ///This slot function creates a new script.
    void newScriptSlot(void);

    ///Slot function for the start button.
    void startButtonPressedSlot(void);

    ///Slot function for the edit script button.
    void editScriptButtonPressedSlot(void);

    ///Slot function for the pause button.
    void pauseButtonPressedSlot(void);

    ///Slot function for the stop button.
    void stopButtonPressedSlot(void);

    ///Slot function for the add script menu.
    void addScriptSlot();

    ///Slot function for the remove script menu.
    void removeScriptSlot();

    ///Slot function for the load config menu.
    void loadConfigSlot();

    ///Slot function for the save as config menu.
    void saveConfigAsSlot();

    ///Slot function for the create sce file menu.
    void createSceFileSlot();

    ///Slot function for the remove ui menu.
    void removeUi();

    ///Slot function for the edit ui menu (opens qt designer with the ui file which
    ///belongs the the current selected row).
    void editUiSlot();

    ///This slot function is called if the cell in the script table has changed.
    void cellChangedSlot(int row, int column);

    ///This slot function is called if the user double clicks a cell in the script table.
    void cellDoubleClickedSlot(int row, int column);

    ///This slot function is called if the user enters a cell in the script table.
    ///With this function the user can move the selected row up or down (while holding the mouse at the row)
    void cellEnteredSlot(int row, int column);

    ///This slot function is called if the selection in the script table has been changed.
    void itemSelectionChangedSlot();

    ///This slot function can be used to stop a script.
    void stopScriptThreadSlot(ScriptThread* threadToStop);

    ///Resizes the table columns.
    void resizeTableColumnsSlot(void);

    ///Slot function for the move up menu.
    void moveTableEntryUpSlot(void);

    ///Slot function for the move down menu.
    void moveTableEntryDownSlot(void);


private:

    ///Swaps the position of 2 table rows.
    void swapTableRowPositions(int row1, int row2);

    ///Checks if ScriptCommunicatormust exit.
    void checkIfScriptCommunicatorMustExit();

    ///Converts the script table to a string (XML).
    QString tableToString(void);


    ///Returns the row to which the given script thread is connected.
    bool findRow(ScriptThread* threadToFind, quint32& row);

    ///Saves the script table content to file.
    void saveTable(void);

    ///Creates a new script table row.
    void createNewTableRow();

    ///Checks if the script table has been changed and saves the table if necessary.
    void checkTableChanged();

    ///The script window user interface.
    Ui::ScriptWindow *m_userInterface;

    ///Pointer to the main window.
    MainWindow* m_mainWindow;

    ///Path to the file in which the script table content has to be saved.
    QString m_currentScriptConfigFileName;

    ///The current content of the script table converted to a string.
    QString m_currentScriptConfigFileString;

    ///Start value for the send ids (every script thread has a unique send id).
    quint32 m_sendIdCounter;

    ///Pointer to the main interface.
    MainInterfaceThread* m_mainInterfaceThread;

    ///The command line argument.
    QStringList m_commandLineScripts;

    ///The create sce file dialog.
    CreateSceFile* m_createSceFileDialog;

    ///The exit code which was passed in exitScriptCommunicator.
    qint32 m_exitCode;


};


#endif // SCRIPTWINDOW_H

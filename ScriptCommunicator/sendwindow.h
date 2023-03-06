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

#ifndef SENDWINDOW_H
#define SENDWINDOW_H

#include "sequencetableview.h"
#include <QMainWindow>
#include <QWidget>
#include <QThread>
#include <QMutex>
#include <QTableWidget>
#include<QFile>
#include "QJSEngine"
#include "QPlainTextEdit"
#include "QComboBox"
#include <QTimer>
#include <QSplitter>
#include <mainwindow.h>

#include "settingsdialog.h"

namespace Ui {
class SendWindow;
}

class SendThread;
class SendWindow;

class SendWindowTextEdit : public QPlainTextEdit
{
    Q_OBJECT

public:
    SendWindowTextEdit(QWidget * parent = 0) : QPlainTextEdit(parent), m_mainWindow(0), m_sendWindow(0)
    {
    }

    ///This function is called if the text looses the focus
    void focusOutEvent(QFocusEvent *e)
    {
        QPlainTextEdit::focusOutEvent(e);
        emit focusOutSignal();
    }

    ///Sets m_sendWindow.
    void setMainWindowPointer(MainWindow* mainWindow)
    {
        m_mainWindow = mainWindow;
    }

    ///Sets m_sendWindow.
    void setSendWindowPointer(SendWindow* sendWindow)
    {
        m_sendWindow = sendWindow;
    }
protected:
    ///Drag enter event.
    void dragEnterEvent(QDragEnterEvent *event);

    ///Drop event.
    void dropEvent(QDropEvent *event);

    ///Drag move event.
    void dragMoveEvent(QDragMoveEvent *event);

    ///The user has pressed a key.
    void keyPressEvent(QKeyEvent *event);

    ///This function is called on double click events.
    void mouseDoubleClickEvent(QMouseEvent *e)
    {
        QPlainTextEdit::mouseDoubleClickEvent(e);
        emit doubleClickSignal();
    }


signals:
    void focusOutSignal(void);
    void doubleClickSignal(void);

private:
    ///Pointer to the main window.
    MainWindow* m_mainWindow;

    ///Pointer to the send window.
    SendWindow* m_sendWindow;

};

///Wrapper class for a QPlainTextEdit in the sequence table.
class SequenceTablePlainTextEdit : public QPlainTextEdit
{
    Q_OBJECT

public:
    SequenceTablePlainTextEdit(SequenceTableView* tableView, SendWindow* sendWindow) :
        QPlainTextEdit(tableView), m_tableView(tableView), m_sendWindow(sendWindow)
    {
    }


    ///This function is called if the user release a mouse button at the PlainTextEdit.
    ///If the mouse button is the right button then the corresponding sequence from the row is send.
    void mouseReleaseEvent(QMouseEvent* event)
    {
        QPlainTextEdit::mouseReleaseEvent(event);

        if ( event->button() == Qt::RightButton )
        {
            m_tableView->sendSequence(m_row, false, this);
        }

    }

    ///Returns m_row.
    int row(void)
    {
        return m_row;
    }
    ///Sets m_row.
    void setRow(int row)
    {
        m_row = row;
    }
protected:
    ///Drag enter event.
    void dragEnterEvent(QDragEnterEvent *event);

    ///Drop event.
    void dropEvent(QDropEvent *event);

    ///Drag move event.
    void dragMoveEvent(QDragMoveEvent *event);

    ///This function is called if the text looses the focus
    void focusOutEvent(QFocusEvent *e);

    ///The user has pressed a key.
    void keyPressEvent(QKeyEvent *event);


private:
    ///Pointer to the main window.
    SequenceTableView* m_tableView;

    ///Pointer to the main window.
    SendWindow* m_sendWindow;

    ///The row in the sequence table to which this element belongs to.
    int m_row;

};

///Wrapper class for a HEX QPlainTextEdit in the sequence table.
class SequenceTableHexTextEdit : public SequenceTablePlainTextEdit
{
public:
    SequenceTableHexTextEdit(SequenceTableView* tableView, SendWindow* sendWindow) :  SequenceTablePlainTextEdit(tableView, sendWindow)
    {
        connect(this, &QPlainTextEdit::textChanged, this, &SequenceTableHexTextEdit::textChangedSlot);
    }

    /**
    * Configures the hex line edit.
    *
    * @param max Max. value.
    */
    void configure(quint32 max){m_max = max;textChangedSlot();}

    ///Returns the current value.
    quint32 getValue(void)
    {
        bool isOk;
        return toPlainText().toULong(&isOk, 16);
    }

    ///Is called if the current text/value has changed.
    void textChangedSlot(void);



private:

    ///Max. value.
    quint32 m_max;
};


///Wrapper class for a QComboBox in the sequence table.
class SequenceTableComboBox : public QComboBox
{
    Q_OBJECT

public:
    SequenceTableComboBox(SequenceTableView* tableView) : QComboBox(tableView), m_tableView(tableView)
    {
    }

    ///Returns m_row.
    int row(void)
    {
        return m_row;
    }
    ///Sets m_row.
    void setRow(int row)
    {
        m_row = row;
    }

protected:

    ///This slot function is called if the user release a mouse button at the PlainTextEdit.
    ///If the mouse button is the right button then the corresponding sequence from the row is send.
    void mouseReleaseEvent(QMouseEvent* event)
    {
        QComboBox::mouseReleaseEvent(event);

        if ( event->button() == Qt::RightButton )
        {
            m_tableView->sendSequence(m_row, false, this);
        }

    }

    ///The user has pressed a key.
    void keyPressEvent(QKeyEvent *event)
    {
        if((event->modifiers() == Qt::AltModifier) && (event->text() == "\r"))
        {//alt+enter pressed

            m_tableView->sendSequence(m_row, false, this);
        }
        else
        {
            QComboBox::keyPressEvent(event);
        }
    }

private:
    ///Pointer to the main window.
    SequenceTableView* m_tableView;

    ///The row in the sequence table to which this element belongs to.
    int m_row;


};

///Class which represents the send window.
class SendWindow : public QMainWindow
{
    friend class SequenceTableView;

    Q_OBJECT
public:
    explicit SendWindow(SettingsDialog* settingsDialog, MainWindow* mainWindow);
    ~SendWindow();

    ///Is called by the main window if the programm is closing
    void programIsClosing();

    ///Sets the connection status of the main interface thread (true for connected).
    void setIsConnected(bool connected);

    ///Send data with the main interface.
    void sendDataWithTheMainInterface(const QByteArray &data, QWidget* callerWidget, int repetitionCount = 0, int pause = 0, bool isCyclicSend=false,
                                      QString scriptName="", bool debug=false);

    ///Shows the send window.
    void show(void);

    ///Is called if the send window has been resized.
    void resizeEvent(QResizeEvent * event);

    ///Saves the sequence table.
    void saveTable();

    ///Converts the sequence table to a string.
    QString tableToString(void);

    ///Loads a saved sequence table.
    void loadTableData(void);

    ///Returns the names of all sequences.
    QStringList getAllSequences(void);

    ///Returns the current name/path to the saved sequence table.
    QString getCurrentSequenceFileName(){return m_currentSequenceFileName;}

    ///Sets the current name/path to the saved sequence table.
    void setCurrentSequenceFileName(QString name){m_currentSequenceFileName = name;}

    ///Returns the current send string (from to gui).
    QString getCurrentSendString();

    ///Returns the current cyclic script (from to gui).
    QString getCurrentCyclicScript();

    ///Sets the current cyclic script.
    void setCurrentCyclicScript(QString script);

    ///Sets the current send string (in the gui).
    void setCurrentSendString(QString text);

    ///Returns the current send string format (from the gui).
    QString getCurrentSendStringFormat();

    ///Sets the current send string format (in the gui).
    void setCurrentSendStringFormat(QString text);

    ///Returns the current send CAN type (from the gui).
    QString getCurrentCanType();

    ///Sets the current send send CAN (in the gui).
    void setCurrentCanType(QString type);

    ///Returns the current send CAN ID (from the gui).
    QString getCurrentCanId();

    ///Sets the current send CAN ID(in the gui).
    void setCurrentCanId(QString text);

    ///Returns the current send repetition value (from the gui).
    QString getCurrentSendRepetition();

    ///Sets the current send repetition value(in the gui).
    void setCurrentSendRepetition(QString text);

    ///Returns the current send pause value (from the gui).
    QString getCurrentSendPause();

    ///Sets the current send pause value (in the gui).
    void setCurrentSendPause(QString text);

    ///True if the cyclic data should be added to the send history.
    void setAddToHistoryCheckBox(bool add);

    ///Returns true if the cyclic data should be added to the send history.
    bool getAddToHistoryCheckBox(void);

    ///Converts a string into a bytes array.
    static QByteArray textToByteArray(QString formatString, QString text, DecimalType decimalType, Endianess endianess);

    ///Checks if the text has the correct format.
    void checkTextEditCell(int row);

    ///The name column (in the sequence table)
    static const int COLUMN_NAME = 0;

    ///The format column (in the sequence table)
    static const int COLUMN_FORMAT = 1;

    ///The CAN type column (in the sequence table)
    static const int COLUMN_CAN_TYPE = 2;

    ///The CAN ID column (in the sequence table)
    static const int COLUMN_CAN_ID = 3;

    ///The value column (in the sequence table)
    static const int COLUMN_VALUE = 4;

    ///The script column (in the sequence table)
    static const int COLUMN_SCRIPT = 5;


    ///Sends a sequence.
    void sendSequence(quint32 sequenceIndex, bool debug, QWidget* callerWidget);

    ///Is called if the content of a text edit in the sequence table of if the content
    ///of the cyclic text edit has been changed.
    void textEditChanged(QPlainTextEdit* textEdit, QString currentFormat, DecimalType decimalType);

    ///Returns the decimal type.
    static DecimalType formatToDecimalType(QString format);

    ///Is called if the value of a combo box in the sequence table or of the cyclic combo box
    ///has been changed.
    QString formatComboBoxChanged(QPlainTextEdit* textEdit, QString format, QString oldFormat);

    ///Checks of the content of a text edit has the correct format.
    void checkTextEditContent(QPlainTextEdit* textEdit, QString currentFormat);

    ///Returns the window splitter.
    QSplitter* getWindowSplitter(void);

    ///Returns the cyclic area splitter.
    QSplitter* getCyclicAreaSplitter(void);

signals:

    ///This signal is emitted for sending data with the main interface.
    void sendDataWithTheMainInterfaceSignal(const QByteArray data, uint id);

    ///This signal is emitted to signalize that the main configuration has to be saved.
    void configHasToBeSavedSignal();

    ///This signal is emitted if the sequence table has been changed.
    void sequenceTableHasChangedSignal();

public slots:

    ///The slot function is called if the the current transmission (sending of data) has been finished.
    void dataHasBeenSendSlot(bool success, uint id);

    ///This slot function is called if a combobox (all comboboxes inside sequence table) value has been changed.
    void comboBoxCellChangedSlot(QString text);

    ///This slot function is called if a CAN type combobox (all comboboxes inside sequence table) value has been changed.
    void canTypeCellChangedSlot(QString type);

    ///This slot function is called if a line edit (all line edit inside the sequence table) value has been changed.
    void textEditCellChangedSlot();

    ///Checks if the text has the correct format.
    void checkCyclicSendInputSlot(void);

    ///Slot function for the unload button.
    void unloadFileSlot(void);

    ///Slot function for the edit cyclic send menu.
    void editCyclicSendScriptSlot(void);

    ///Sets the window title.
    void setTitle(QString extraString);

    ///Slot function for the add cyclic send menu.
    void addCyclicSendScriptSlot(void);

    ///Slot function for the save file button.
    void saveFileSlot(void);

    ///Slot function for the send button.
    void sendButtonPressedSlot();

private slots:

    ///Is called if the user double clicks the cyclic script text edit.
    void cyclicScriptTextEditDoubleClickedSlot(void);

    ///Is called if text of the cyclic script text edit has been changed.
    void cyclicScriptTextEditChangedSlot(void);

    ///Is called if the name of a sequence has been changed.
    void sequenceNameChangeSlot(QTableWidgetItem *item);

    ///Resizes all sequence table columns.
    void resizeTableColumnsSlot(void);

    ///Slot function for the delete button.
    void deleteButtonClickedSlot(void);

    ///Slot function for the remove script button.
    void removeScriptButtonClickedSlot(void);

    ///Slot function for the edit script button.
    void editScriptButtonClickedSlot(void);

    ///Slot function for the new button.
    void newButtonClickedSlot(void);

    ///This slot function for the create script menu item.
    void createScriptSlot(void);

    ///This slot function for the debug script menu item.
    void debugScriptSlot(void);

    ///This slot function for the debug cyclic script menu item.
    void debugCyclicScriptSlot(void);

    ///This slot function for the add menu item.
    void addScriptSlot(void);

    ///Slot function for the load file button.
    void loadFileSlot(void);

    ///This slot function is called if the user changes the text of the line edit.
    void cyclicSendInputTextChangedSlot();

    ///Slot function for the save file as button.
    void saveAsFileSlot(void);

    ///Slot function for the edit all sequence scripts menu.
    void editAllSequenceScriptsSlot(void);

    ///This slot is called if the value of the send string format combobox has been changed.
    void currentSendStringFormatChangedSlot(QString format);

    ///This slot is called if the value of the CAN type combobox has been changed.
    void currentCanTypeChangedSlot(QString type);

    ///This slot function is called if the user enters a cell in the script table.
    ///With this function the user can move the selected row up or down (while holding the mouse at the row)
    void cellEnteredSlot(int row, int column);

    ///This slot function is called by m_currentSendTimer.
    void sendTimerElapsedSlot(void);

    ///Copies the content of a sequence table entry to the cyclic send area.
    void copyFromSequenceTableSlot(void);

    ///This slot function is called if the selection in the sequence table has been changed.
    void itemSelectionChangedSlot(void);

    ///Slot function for the move up menu.
    void moveTableEntryUpSlot(void);

    ///Slot function for the move down menu.
    void moveTableEntryDownSlot(void);

private:

    ///Sets the CAN GUI elements in the sequence table.
    void setCanElementsInSequenceTable(void);

    ///Swaps the position of 2 table rows.
    void swapTableRowPositions(int row1, int row2);

    ///Enables or disable the send window for cyclic sending.
    void enableWindowForCyclicSend(bool enable);

    ///Maximum value for the repetition.
    static const int MAX_REPETITIONS = 10000000;

    ///Maximum value for the pause.
    static const int MAX_PAUSE = 100000;

    ///Sets the value of the progress bar.
    void setProgressbarValue(int value);

    ///This function is called if the main window is closed.
    void closeEvent(QCloseEvent * event);

    ///This function is called if an error during a cyclic sending has occured.
    void cyclicSendErrorReceived(void);

    ///Checks if the content of the sequence table has been changed. And saves the table it has been changed.
    void checkTableChanged();

    ///The function is called if the current cyclic send process has been finished.
    void currentCyclicSendFinished(void);

    ///The user interface.
    Ui::SendWindow *m_userInterface;

    ///Pointer to the settings dialog.
    SettingsDialog *m_settingsDialog;

    ///True if a cyclic sending is in progress.
    bool m_cyclicSendingIsInProgress;

    ///True if ScriptCommunicator is closing.
    bool m_programIsClosing;

    ///True if the main interface thread is connected.
    bool m_isConnected;

    ///The name/path of the save sequence table.
    QString m_currentSequenceFileName;

    ///The current sequence table converted into a string.
    QString m_currentSequenceFileString;

    ///The old value for the send string format combobox.
    QString m_oldSendStringFormat;

    ///The data which shall be sent during the current send process.
    QByteArray m_currentSendData;

    ///The repetition count which shall be used during the current send process.
    int m_currentSendRepetitionCount;

    ///The number of successfully sends during the current send process.
    int m_currentSendNumberOfSends;

    ///The pause which shall be used during the current send process.
    int m_currentSendPause;


    ///The timer which shall be used during the current send process.
    QTimer m_currentSendTimer;

    ///The cyclic send script.
    QString m_currentSendScript;

    ///The cyclic send script engine wrapper.
    SequenceScriptEngineWrapper* m_currentScriptEngineWrapper;

    ///Pointer to the main window.
    MainWindow* m_mainWindow;


};


#endif // SENDWINDOW_H

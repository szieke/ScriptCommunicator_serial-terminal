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

#include "sendwindow.h"
#include "ui_sendwindow.h"
#include <QFocusEvent>
#include <QPalette>
#include "mainwindow.h"
#include <QtSerialPort/QSerialPort>
#include <QMenu>
#include <QFileDialog>
#include <QBuffer>
#include<QDomDocument>
#include <QSignalMapper>
#include <QDebug>
#include "mainwindow.h"
#include "mainInterfaceThread.h"
#include "QScriptEngine"
#include "QRegularExpression"
#include <QPainter>
#include <QScrollBar>
#include "scriptThread.h"
#include <QProcess>
#include <QInputDialog>
#include <QStringList>
#include <QMimeData>



/**
 * Drag enter event.
 * @param event
 *      The drag enter event.
 */
void SendWindowTextEdit::dragEnterEvent(QDragEnterEvent *event)
{
    if(event->mimeData()->hasUrls())
    {
        event->acceptProposedAction();
    }
}

/**
 * Drop event.
 * @param event
 *      The drop event.
 */
void SendWindowTextEdit::dropEvent(QDropEvent *event)
{
    if(event->mimeData()->hasUrls())
    {
#ifdef Q_OS_LINUX
        QString files = event->mimeData()->text().remove("file://");
#else
        QString files = event->mimeData()->text().remove("file:///");
#endif
        QStringList list = files.split("\n");
        if(!list.isEmpty())
        {
            setPlainText(list[0]);
        }
        event->acceptProposedAction();
    }
}

/**
 * The user has pressed a key.
 *
 * @param event
 *      The key event.
 */
void SendWindowTextEdit::keyPressEvent(QKeyEvent *event)
{
    if((event->modifiers() == Qt::AltModifier) && (event->text() == "\r"))
    {//alt+enter pressed

        if(m_mainWindow)
        {
            m_mainWindow->sendButtonPressedSlot();
        }
        else if(m_sendWindow)
        {
            m_sendWindow->sendButtonPressedSlot();
        }

    }
    else
    {
        QPlainTextEdit::keyPressEvent(event);
    }
}


/**
 * Constructor.
 * @param settingsDialog
 *      Pointer to the settings dialog.
 * @param mainWindow
 *      Pointer to the main window.
 */
SendWindow::SendWindow(SettingsDialog *settingsDialog, MainWindow *mainWindow) :
    QMainWindow(0),
    m_userInterface(new Ui::SendWindow), m_settingsDialog(settingsDialog), m_cyclicSendingIsInProgress(false), m_programIsClosing(false), m_isConnected(false), m_currentSendData(),
    m_currentSendRepetitionCount(0), m_currentSendNumberOfSends(0), m_currentSendPause(0), m_currentSendTimer(0), m_currentScriptEngineWrapper(0),
    m_mainWindow(mainWindow)
{
    m_userInterface->setupUi(this);

    connect(m_userInterface->tableWidget->horizontalHeader(), SIGNAL(sectionResized(int, int, int)), this, SLOT(resizeTableColumnsSlot()));
    connect(m_userInterface->tableWidget, SIGNAL(itemChanged(QTableWidgetItem*)), this, SLOT(sequenceNameChangeSlot(QTableWidgetItem*)));

    connect(m_userInterface->SendPushButton, SIGNAL(clicked()), this, SLOT(sendButtonPressedSlot()));
    connect(m_userInterface->tableWidget, SIGNAL(cellEntered(int, int)), this, SLOT(cellEnteredSlot(int, int)));
    connect(m_userInterface->action_new, SIGNAL(triggered()), this, SLOT(newButtonClickedSlot()));
    connect(m_userInterface->action_delete, SIGNAL(triggered()), this, SLOT(deleteButtonClickedSlot()));
    connect(m_userInterface->actionRemoveScript, SIGNAL(triggered()), this, SLOT(removeScriptButtonClickedSlot()));
    connect(m_userInterface->actionEditScript, SIGNAL(triggered()), this, SLOT(editScriptButtonClickedSlot()));
    connect(m_userInterface->actionLoad, SIGNAL(triggered()), this, SLOT(loadFileSlot()));
    connect(m_userInterface->action_save, SIGNAL(triggered()), this, SLOT(saveFileSlot()));
    connect(m_userInterface->actionSave_as, SIGNAL(triggered()), this, SLOT(saveAsFileSlot()));
    connect(m_userInterface->actionEditAllSequenceScripts, SIGNAL(triggered()), this, SLOT(editAllSequenceScriptsSlot()));
    connect(m_userInterface->actionUnload, SIGNAL(triggered()), this, SLOT(unloadFileSlot()));
    connect(m_userInterface->actionMoveDown, SIGNAL(triggered()), this, SLOT(moveTableEntryDownSlot()));
    connect(m_userInterface->actionMoveUp, SIGNAL(triggered()), this, SLOT(moveTableEntryUpSlot()));
    connect(m_userInterface->actionAddCyclicScript, SIGNAL(triggered()), this, SLOT(addCyclicSendScriptSlot()));
    connect(m_userInterface->actionEditCyclicScript, SIGNAL(triggered()), this, SLOT(editCyclicSendScriptSlot()));

    connect(m_userInterface->actionAddSequenceScript, SIGNAL(triggered()), this, SLOT(addScriptSlot()));
    connect(m_userInterface->actionCreateSequenceScript, SIGNAL(triggered()), this, SLOT(createScriptSlot()));
    connect(m_userInterface->actionDebugSequenceScript, SIGNAL(triggered()), this, SLOT(debugScriptSlot()));
    connect(m_userInterface->actionDebugCyclicSequenceScript, SIGNAL(triggered()), this, SLOT(debugCyclicScriptSlot()));

    connect(m_userInterface->CyclicSendFormat, SIGNAL(currentTextChanged(QString)), this, SLOT(currentSendStringFormatChangedSlot(QString)));

    connect(m_userInterface->CyclicSendInput, SIGNAL(textChanged()), this, SLOT(cyclicSendInputTextChangedSlot()));
    connect(m_userInterface->CyclicSendInput, SIGNAL(focusOutSignal()), this, SLOT(checkCyclicSendInputSlot()));
    connect(m_userInterface->actionCopySequenceFromTable, SIGNAL(triggered()), this, SLOT(copyFromSequenceTableSlot()));
    connect(m_userInterface->CyclicSendScript, SIGNAL(doubleClickSignal()), this, SLOT(cyclicScriptTextEditDoubleClickedSlot()));
    connect(m_userInterface->CyclicSendScript, SIGNAL(textChanged()), this, SLOT(cyclicScriptTextEditChangedSlot()));
    connect(m_userInterface->tableWidget, SIGNAL(itemSelectionChanged()), this, SLOT(itemSelectionChangedSlot()));

    connect(&m_currentSendTimer, SIGNAL(timeout()), this, SLOT(sendTimerElapsedSlot()));

    m_userInterface->SendProgressBar->setValue(0);
    m_userInterface->CyclicSendRepetition->setText("0");
    m_userInterface->CyclicSendPause->setText("0");

    m_userInterface->CyclicSendRepetition->setValidator(new QIntValidator(0, MAX_REPETITIONS, m_userInterface->CyclicSendRepetition));
    m_userInterface->CyclicSendPause->setValidator(new QIntValidator(0, MAX_PAUSE, m_userInterface->CyclicSendPause));

    m_userInterface->tableWidget->setHorizontalHeaderLabels({"name", "format", "value", "optional sequence script"});

    m_userInterface->tableWidget->setSendWindow(this);
    m_userInterface->tableWidget->setMainWindow(m_mainWindow);

    m_currentSequenceFileString = tableToString();

    QStringList availTargets;
    availTargets << "ascii" << "hex" << "bin" << "uint8" << "uint16" << "uint32" << "int8" << "int16" << "int32";
    m_userInterface->CyclicSendFormat->addItems(availTargets);

    QShortcut* shortcut = new QShortcut(QKeySequence("Ctrl+Shift+X"), this);
    QObject::connect(shortcut, SIGNAL(activated()), this, SLOT(close()));

    m_userInterface->CyclicSendInput->setSendWindowPointer(this);
    m_userInterface->CyclicSendScript->setSendWindowPointer(this);

}
/**
 * Destructor.
 */
SendWindow::~SendWindow()
{
    delete m_userInterface;
}

/**
 * Is called by the main window if the programm is closing
 */
void SendWindow::programIsClosing()
{
    m_programIsClosing = true;
    while(m_cyclicSendingIsInProgress)
    {
        QCoreApplication::processEvents();
        QThread::msleep(1);
    }
}

/**
 * Returns the window splitter.
 * @return
 *      The splitter.
 */
QSplitter* SendWindow::getWindowSplitter(void)
{
    return m_userInterface->windowSplitter;
}

/**
 * Returns the cyclic area splitter.
 * @return
 *      The splitter.
 */
QSplitter* SendWindow::getCyclicAreaSplitter(void)
{
    return m_userInterface->cyclicAreaSplitter;
}


/**
 * Slot function for the move up menu.
 */
void SendWindow::moveTableEntryUpSlot(void)
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
void SendWindow::moveTableEntryDownSlot(void)
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
 * This slot function is called if the selection in the sequence table has been changed.
 */
void SendWindow::itemSelectionChangedSlot(void)
{
    int selectedRow = (m_userInterface->tableWidget->selectedItems().isEmpty())? -1 : m_userInterface->tableWidget->selectedItems()[0]->row();

    m_userInterface->actionEditScript->setEnabled(false);
    m_userInterface->actionRemoveScript->setEnabled(false);
    m_userInterface->actionAddSequenceScript->setEnabled(false);
    m_userInterface->actionCreateSequenceScript->setEnabled(false);
    m_userInterface->actionMoveDown->setEnabled(false);
    m_userInterface->actionMoveUp->setEnabled(false);
    m_userInterface->action_delete->setEnabled(false);
    m_userInterface->actionCopySequenceFromTable->setEnabled(false);
    m_userInterface->actionDebugSequenceScript->setEnabled(false);
    m_userInterface->actionEditAllSequenceScripts->setEnabled(false);

    for( int r = 0; r < m_userInterface->tableWidget->rowCount(); ++r )
    {
        SequenceTablePlainTextEdit* scriptTextEdit = static_cast<SequenceTablePlainTextEdit*>(m_userInterface->tableWidget->cellWidget(r, COLUMN_SCRIPT));
        if(scriptTextEdit && !scriptTextEdit->toPlainText().isEmpty())
        {
            m_userInterface->actionEditAllSequenceScripts->setEnabled(true);
            break;
        }
    }


    if(selectedRow != -1)
    {
        if(m_userInterface->SendPushButton->text() != "cancel")
        {//No cyclic send is in progress.
            m_userInterface->actionCopySequenceFromTable->setEnabled(true);
        }

        SequenceTablePlainTextEdit* textEdit = static_cast<SequenceTablePlainTextEdit*>(m_userInterface->tableWidget->cellWidget(selectedRow, COLUMN_VALUE));
        SequenceTablePlainTextEdit* scriptTextEdit = static_cast<SequenceTablePlainTextEdit*>(m_userInterface->tableWidget->cellWidget(selectedRow, COLUMN_SCRIPT));

        if(!scriptTextEdit->toPlainText().isEmpty() &&!textEdit->toPlainText().isEmpty())
        {
            m_userInterface->actionDebugSequenceScript->setEnabled(true);
        }

        if(scriptTextEdit->toPlainText().size() != 0)
        {
            m_userInterface->actionEditScript->setEnabled(true);
            m_userInterface->actionRemoveScript->setEnabled(true);
        }

        if(selectedRow != 0)
        {
            m_userInterface->actionMoveUp->setEnabled(true);
        }

        if(selectedRow != (m_userInterface->tableWidget->rowCount() - 1))
        {
            m_userInterface->actionMoveDown->setEnabled(true);
        }

        m_userInterface->actionAddSequenceScript->setEnabled(true);
        m_userInterface->actionCreateSequenceScript->setEnabled(true);
        m_userInterface->action_delete->setEnabled(true);
    }
}

/**
 * Copies the content of a sequence table entry to the cyclic send area.
 */
void SendWindow::copyFromSequenceTableSlot(void)
{
    if( m_userInterface->tableWidget->rowCount() > 0)
    {
        QModelIndexList indexes = m_userInterface->tableWidget->selectionModel()->selection().indexes();
        qint32 row = 0;

        if(indexes.size() > 0)
        {
            row = indexes[0].row();
        }

        SequenceTableComboBox* box = static_cast<SequenceTableComboBox*>(m_userInterface->tableWidget->cellWidget(row, COLUMN_FORMAT));
        SequenceTablePlainTextEdit* lineEdit = static_cast<SequenceTablePlainTextEdit*>(m_userInterface->tableWidget->cellWidget(row, COLUMN_VALUE));
        SequenceTablePlainTextEdit* scriptLineEdit = static_cast<SequenceTablePlainTextEdit*>(m_userInterface->tableWidget->cellWidget(row, COLUMN_SCRIPT));

        m_userInterface->CyclicSendInput->blockSignals(true);
        m_userInterface->CyclicSendFormat->blockSignals(true);

        m_userInterface->CyclicSendInput->setPlainText(lineEdit->toPlainText());
        m_userInterface->CyclicSendFormat->setCurrentText(box->currentText());

        m_userInterface->CyclicSendInput->blockSignals(false);
        m_userInterface->CyclicSendFormat->blockSignals(false);

        m_userInterface->CyclicSendScript->setPlainText(scriptLineEdit->toPlainText());
    }

}

/**
 * Is called if the content of a text edit in the sequence table of if the content
 * of the cyclic text edit has been changed.
 * @param textEdit
 *      The text edit.
 * @param currentFormat
 *      The current format.
 * @param decimalType
 *      The decimal type.
 */
void SendWindow::textEditChanged(QPlainTextEdit* textEdit, QString currentFormat, DecimalType decimalType)
{
    QString text = textEdit->toPlainText();
    int cursorPosition = textEdit->textCursor().position();
    QTextCursor cursor = textEdit->textCursor();


    if(!text.isEmpty())
    {
        if(currentFormat != "ascii")
        {
            text.replace("\n", "");

            if(currentFormat == "hex")
            {
                text.replace(QRegularExpression("[^a-fA-F\\d\\s]"), "");
            }
            else if(currentFormat == "bin")
            {
                text.replace(QRegularExpression("[^0-1\\s]"), "");
            }
            else
            {
                QStringList list = text.split(" ");
                QString resulText = "";
                for(int i = 0; i < list.length(); i++)
                {
                    if(list[i].length() > 1)
                    {
                        int index = list[i].indexOf("-", 1);
                        if((index > 0) && (index != -1))
                        {
                            list[i].remove(index, 1);
                        }
                    }
                    QString regEx = "[^";
                    if((decimalType == DECIMAL_TYPE_INT8) || (decimalType == DECIMAL_TYPE_INT16)
                            || (decimalType == DECIMAL_TYPE_INT32))
                    {
                        regEx +="\\-";
                    }
                    regEx += "\\d\\s]";
                    list[i].replace(QRegularExpression(regEx), "");
                    resulText += list[i];
                    if((i + 1) != list.length())
                    {
                        resulText += " ";
                    }
                }
                text = resulText;
            }

            textEdit->blockSignals(true);
            textEdit->setPlainText(text);
            textEdit->blockSignals(false);
        }
    }

    cursor.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor);
    cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, cursorPosition);
    cursor.clearSelection();
    textEdit->setTextCursor(cursor);
}

/**
 * This slot function is called if a line edit (all line edit inside the sequence table) value has been changed.
 */
void SendWindow::textEditCellChangedSlot()
{
    SequenceTablePlainTextEdit* textEdit = static_cast<SequenceTablePlainTextEdit*>(sender());
    SequenceTableComboBox* box = static_cast<SequenceTableComboBox*>(m_userInterface->tableWidget->cellWidget(textEdit->row(),COLUMN_FORMAT));

    textEditChanged(textEdit, box->currentText(), formatToDecimalType(box->currentText()));
}

/**
* This slot function is called if the user changes the text of the line edit.
* @param text
*         The new text.
*/
void SendWindow::cyclicSendInputTextChangedSlot(void)
{
    textEditChanged(m_userInterface->CyclicSendInput, m_userInterface->CyclicSendFormat->currentText(),
                    formatToDecimalType(m_userInterface->CyclicSendFormat->currentText()));

    m_userInterface->actionDebugCyclicSequenceScript->setEnabled(false);
    if(!m_userInterface->CyclicSendInput->toPlainText().isEmpty() &&
            !m_userInterface->CyclicSendScript->toPlainText().isEmpty())
    {
        m_userInterface->actionDebugCyclicSequenceScript->setEnabled(true);
    }
}

/**
 * Returns the decimal type.
 * @param format
 *      The current format.
 * @return
 *      The decimal type.
 */
DecimalType SendWindow::formatToDecimalType(QString format)
{
    DecimalType ret = DECIMAL_TYPE_UINT8;

    if(format == "uint8")
    {
        ret = DECIMAL_TYPE_UINT8;
    }
    else if(format == "int8")
    {
        ret = DECIMAL_TYPE_INT8;
    }
    else if(format == "uint16")
    {
        ret = DECIMAL_TYPE_UINT16;
    }
    else if(format == "int16")
    {
        ret = DECIMAL_TYPE_INT16;
    }
    else if(format == "uint32")
    {
        ret = DECIMAL_TYPE_UINT32;
    }
    else if(format == "int32")
    {
        ret = DECIMAL_TYPE_INT32;
    }
    else
    {
        ret = DECIMAL_TYPE_UINT8;
    }
    return ret;
}

/**
  * Checks of the content of a text edit has the correct format.
  * @param textEdit
  *      The text edit.
  * @param currentFormat
  *      The current format.
  */
void SendWindow::checkTextEditContent(QPlainTextEdit* textEdit, QString currentFormat)
{
    QString text = textEdit->toPlainText();

    if(!text.isEmpty())
    {
        if(currentFormat != "ascii")
        {
            const Settings* settings = m_settingsDialog->settings();
            textEditChanged(textEdit, currentFormat, formatToDecimalType(currentFormat));
            text = textEdit->toPlainText();

            QByteArray array = textToByteArray(currentFormat, text, formatToDecimalType(currentFormat), settings->targetEndianess);

            text = MainWindow::byteArrayToNumberString(array, (currentFormat == "bin") ? true : false, (currentFormat == "hex") ? true : false,
                                                       false, true, true, formatToDecimalType(currentFormat), settings->targetEndianess);

            textEdit->blockSignals(true);
            textEdit->setPlainText(text);
            textEdit->blockSignals(false);
        }
    }
}

/**
 * Checks if the text has the correct format.
 * @param row
 *      The row of the line edit.
 */
void SendWindow::checkTextEditCell(int row)
{
    SequenceTableComboBox* box = static_cast<SequenceTableComboBox*>(m_userInterface->tableWidget->cellWidget(row,COLUMN_FORMAT));
    SequenceTablePlainTextEdit* textEdit = static_cast<SequenceTablePlainTextEdit*>(m_userInterface->tableWidget->cellWidget(row,COLUMN_VALUE));
    checkTextEditContent(textEdit, box->currentText());
}


/**
 * Checks if the text has the correct format.
 */
void SendWindow::checkCyclicSendInputSlot(void)
{
    checkTextEditContent(m_userInterface->CyclicSendInput, m_userInterface->CyclicSendFormat->currentText());
}

/**
 * Is called if the value of a combo box in the sequence table or of the cyclic combo box
 * has been changed.
 * @param textEdit
 *      The text edit.
 * @param format
 *      The current format string.
 * @param oldFormat
 *      The old format string.
 */
QString SendWindow::formatComboBoxChanged(QPlainTextEdit* textEdit, QString format, QString oldFormat)
{
    QString newText;
    const Settings* settings = m_settingsDialog->settings();

    if(format != "ascii")
    {
        QByteArray array = textToByteArray(oldFormat,textEdit->toPlainText(), formatToDecimalType(oldFormat), settings->targetEndianess);
        bool isHex = (format == "hex") ? true : false;
        bool isBin = (format == "bin") ? true : false;
        newText = MainWindow::byteArrayToNumberString(array, isBin, isHex, false, true, true, formatToDecimalType(format), settings->targetEndianess);
    }
    else
    {
        QByteArray array = textToByteArray(oldFormat,textEdit->toPlainText(), formatToDecimalType(oldFormat), settings->targetEndianess);
        for(auto el : array)
        {
            if(el == '\r')
            {
                newText += "<|#CR#|>";
            }
            else
            {
                newText += el;
            }
        }
    }
    return newText;
}

/**
 * This slot function is called if a combobox (all comboboxes inside sequence table) value has been changed.
 * @param text
 *      The new text.
 */
void SendWindow::comboBoxCellChangedSlot(QString text)
{
    SequenceTableComboBox* box = static_cast<SequenceTableComboBox*>(sender());

    SequenceTablePlainTextEdit* textEdit = static_cast<SequenceTablePlainTextEdit*>(m_userInterface->tableWidget->cellWidget(box->row(),COLUMN_VALUE));

    QString format = text;
    QString oldFormat = m_userInterface->tableWidget->item(box->row(), COLUMN_VALUE)->data(Qt::UserRole + 1).toString();

    QString newText = formatComboBoxChanged(textEdit, format, oldFormat);

    textEdit->blockSignals(true);
    textEdit->setPlainText(newText);
    textEdit->blockSignals(false);
    m_userInterface->tableWidget->item(box->row(), COLUMN_VALUE)->setData(Qt::UserRole + 1, format);

}

/**
 * This slot is called if the value of the send string format combobox has been changed.
 * @param format
 *      The new format.
 */
void SendWindow::currentSendStringFormatChangedSlot(QString format)
{

    if(!m_userInterface->CyclicSendInput->toPlainText().isEmpty())
    {
        QString newText = formatComboBoxChanged(m_userInterface->CyclicSendInput, format, m_oldSendStringFormat);
        m_userInterface->CyclicSendInput->blockSignals(true);
        m_userInterface->CyclicSendInput->setPlainText(newText);
        m_userInterface->CyclicSendInput->blockSignals(false);
    }

    m_oldSendStringFormat = format;
}

/**
 * Slot function for the remove script button.
 */
void SendWindow::removeScriptButtonClickedSlot(void)
{

    if(m_userInterface->tableWidget->selectedItems().empty())
    {
        QMessageBox::information(this, "no script removed", "to remove a script an entry must be selected");
    }
    else
    {
        for(auto it : m_userInterface->tableWidget->selectedItems())
        {
            SequenceTablePlainTextEdit* textEdit = static_cast<SequenceTablePlainTextEdit*>(m_userInterface->tableWidget->cellWidget(it->row(), COLUMN_SCRIPT));
            textEdit->setPlainText("");
            break;
        }
    }
    itemSelectionChangedSlot();
}

/**
 * Slot function for the edit script button.
 */
void SendWindow::editScriptButtonClickedSlot(void)
{

    int selectedRow = (m_userInterface->tableWidget->selectedItems().isEmpty())? -1 : m_userInterface->tableWidget->selectedItems()[0]->row();
    if(selectedRow != -1 )
    {
        QStringList arguments;
        SequenceTablePlainTextEdit* textEdit = static_cast<SequenceTablePlainTextEdit*>(m_userInterface->tableWidget->cellWidget(selectedRow, COLUMN_SCRIPT));
        arguments << textEdit->toPlainText();
        MainWindow::openScriptEditor(arguments, m_settingsDialog->settings(), this);
    }
}

/**
 * Slot function for the delete button.
 */
void SendWindow::deleteButtonClickedSlot(void)
{
    for(auto it : m_userInterface->tableWidget->selectedItems())
    {
        m_userInterface->tableWidget->selectRow(it->row() - 1);
        m_userInterface->tableWidget->removeRow(it->row());
        break;
    }

    for(int i = 0; i < m_userInterface->tableWidget->rowCount(); i++)
    {
        SequenceTableComboBox * box = static_cast<SequenceTableComboBox*>(m_userInterface->tableWidget->cellWidget(i, COLUMN_FORMAT));
        box->setRow(i);
        SequenceTablePlainTextEdit * listEdit = static_cast<SequenceTablePlainTextEdit*>(m_userInterface->tableWidget->cellWidget(i, COLUMN_VALUE));
        listEdit->setRow(i);
    }
    itemSelectionChangedSlot();
    emit sequenceTableHasChangedSignal();
}

/**
 * Slot function for the new button.
 */
void SendWindow::newButtonClickedSlot(void)
{

    m_userInterface->tableWidget->blockSignals(true);

    m_userInterface->tableWidget->insertRow(0);

    m_userInterface->tableWidget->setItem(0, COLUMN_NAME,  new QTableWidgetItem(QString("%1").arg(m_userInterface->tableWidget->rowCount())));
    m_userInterface->tableWidget->setItem(0, COLUMN_FORMAT, new QTableWidgetItem());
    m_userInterface->tableWidget->setItem(0, COLUMN_SCRIPT, new QTableWidgetItem());
    m_userInterface->tableWidget->setItem(0, COLUMN_VALUE, new QTableWidgetItem());

    SequenceTableComboBox* comboBox = new SequenceTableComboBox(m_userInterface->tableWidget);
    QStringList availTargets;
    availTargets  << "ascii" << "hex" << "bin" << "uint8" << "uint16" << "uint32" << "int8" << "int16" << "int32";
    comboBox->addItems(availTargets);
    comboBox->setToolTip(toolTip());
    m_userInterface->tableWidget->setCellWidget(0, COLUMN_FORMAT, comboBox);
    comboBox->setRow(0);
    comboBox->setFrame(false);
    connect(comboBox, SIGNAL(currentTextChanged(QString)), this, SLOT(comboBoxCellChangedSlot(QString)));



    SequenceTablePlainTextEdit* lineEdit = new SequenceTablePlainTextEdit(m_userInterface->tableWidget, this);
    lineEdit->setContextMenuPolicy(Qt::NoContextMenu);
    lineEdit->setToolTip(toolTip());
    lineEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_userInterface->tableWidget->setCellWidget(0, COLUMN_VALUE, lineEdit);
    lineEdit->setRow(0);
    lineEdit->setFrameStyle(QFrame::NoFrame);
    connect(lineEdit, SIGNAL(textChanged()), this, SLOT(textEditCellChangedSlot()));

    lineEdit = new SequenceTablePlainTextEdit(m_userInterface->tableWidget, this);
    lineEdit->setContextMenuPolicy(Qt::NoContextMenu);
    lineEdit->setToolTip(toolTip());
    lineEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_userInterface->tableWidget->setCellWidget(0, COLUMN_SCRIPT, lineEdit);
    lineEdit->setRow(0);
    lineEdit->setFrameStyle(QFrame::NoFrame);



    m_userInterface->tableWidget->item(0, COLUMN_VALUE)->setData(Qt::UserRole + 1, "ascii");

    QStringList list;
    for(int i = 0; i <  m_userInterface->tableWidget->rowCount(); i++)
    {
        list << "    ";
    }
    m_userInterface->tableWidget->setVerticalHeaderLabels(list);

    for( int r = 0; r < m_userInterface->tableWidget->rowCount(); ++r )
    {
        SequenceTableComboBox* box = static_cast<SequenceTableComboBox*>(m_userInterface->tableWidget->cellWidget(r,COLUMN_FORMAT));
        box->setRow(r);
        SequenceTablePlainTextEdit* textEdit = static_cast<SequenceTablePlainTextEdit*>(m_userInterface->tableWidget->cellWidget(r,COLUMN_VALUE));
        textEdit->setRow(r);
        textEdit = static_cast<SequenceTablePlainTextEdit*>(m_userInterface->tableWidget->cellWidget(r,COLUMN_SCRIPT));
        textEdit->setRow(r);
    }

    m_userInterface->tableWidget->clearSelection();
    m_userInterface->tableWidget->selectRow(0);
    m_userInterface->tableWidget->blockSignals(false);
    itemSelectionChangedSlot();

    emit sequenceTableHasChangedSignal();
}


/**
 * This slot function for the debug cyclic script menu item.
 */
void SendWindow::debugCyclicScriptSlot(void)
{
    checkCyclicSendInputSlot();

    const Settings* settings = m_settingsDialog->settings();
    QByteArray sendData = textToByteArray(m_userInterface->CyclicSendFormat->currentText(), m_userInterface->CyclicSendInput->toPlainText(),
                                          formatToDecimalType(m_userInterface->CyclicSendFormat->currentText()), settings->targetEndianess);
    if(m_userInterface->CyclicSendFormat->currentText() == "ascii")
    {
        const Settings* settings = m_settingsDialog->settings();
        sendData.replace("\n", settings->consoleSendOnEnter.toLocal8Bit());
    }
    if(!sendData.isEmpty())
    {
        sendDataWithTheMainInterface(sendData, this,
                                     m_userInterface->CyclicSendRepetition->text().toInt(),
                                     m_userInterface->CyclicSendPause->text().toInt(), true, m_userInterface->CyclicSendScript->toPlainText(), true);
    }
}

/**
 *This slot function for the debug script menu item.
 */
void SendWindow::debugScriptSlot(void)
{
    if(m_userInterface->tableWidget->selectedItems().length() > 0)
    {
        sendSequence(m_userInterface->tableWidget->selectedItems().at(SendWindow::COLUMN_NAME)->row(), true, this);
    }
}

/**
 * This slot function for the create script menu item.
 */
void SendWindow::createScriptSlot(void)
{
    int selectedRow = (m_userInterface->tableWidget->selectedItems().isEmpty())? -1 : m_userInterface->tableWidget->selectedItems()[0]->row();
    if(selectedRow != -1)
    {
        QString templateScriptFileName = QFileDialog::getOpenFileName(this, tr("Select sequence script template"),
                                                                      MainWindow::getScriptCommunicatorFilesFolder() + "/templates/sequenceScriptTemplates",
                                                                      tr("sequence script files (*.js);;Files (*)"));
        if(!templateScriptFileName.isEmpty())
        {
            QString scriptFileName = QFileDialog::getSaveFileName(this, tr("Select sequence script name"),
                                                                  MainWindow::getScriptCommunicatorFilesFolder(), tr("sequence script files (*.js);;Files (*)"));
            if(!scriptFileName.isEmpty())
            {
                QFile(scriptFileName).remove();

                if(QFile::copy(templateScriptFileName, scriptFileName))
                {
                    SequenceTablePlainTextEdit* textEdit = static_cast<SequenceTablePlainTextEdit*>(m_userInterface->tableWidget->cellWidget(selectedRow, COLUMN_SCRIPT));
                    textEdit->setPlainText(scriptFileName);
                    itemSelectionChangedSlot();

                }
            }
        }
    }
}

/**
 * This slot function for the add menu item.
 */
void SendWindow::addScriptSlot(void)
{

    int selectedRow = (m_userInterface->tableWidget->selectedItems().isEmpty())? -1 : m_userInterface->tableWidget->selectedItems()[0]->row();
    if(selectedRow != -1)
    {
        QString tmpFileName = QFileDialog::getOpenFileName(this, tr("Open sequence script file"),
                                                           "", tr("sequence script files (*.js);;Files (*)"));
        if(!tmpFileName.isEmpty())
        {
            SequenceTablePlainTextEdit* textEdit = static_cast<SequenceTablePlainTextEdit*>(m_userInterface->tableWidget->cellWidget(selectedRow, COLUMN_SCRIPT));
            textEdit->setPlainText(tmpFileName);
            itemSelectionChangedSlot();
        }
    }
}
/**
 * Is called if the name of a sequence has been changed.
 *
 * @param item
 *      The changed item.
 */
void SendWindow::sequenceNameChangeSlot(QTableWidgetItem* item)
{
    (void)item;
    itemSelectionChangedSlot();
    emit sequenceTableHasChangedSignal();
}

/**
 * Resizes all sequence table columns.
 */
void SendWindow::resizeTableColumnsSlot(void)
{

    m_userInterface->tableWidget->setColumnWidth(COLUMN_SCRIPT, m_userInterface->tableWidget->width() -
                                                 (m_userInterface->tableWidget->columnWidth(COLUMN_NAME)
                                                  + m_userInterface->tableWidget->columnWidth(COLUMN_FORMAT)
                                                  + m_userInterface->tableWidget->columnWidth(COLUMN_VALUE)
                                                  + 2 *m_userInterface->tableWidget->frameWidth()
                                                  + m_userInterface->tableWidget->verticalHeader()->width()
                                                  + (m_userInterface->tableWidget->verticalScrollBar()->isVisible() ?
                                                         m_userInterface->tableWidget->verticalScrollBar()->width() : 0)));
}

/**
 * Is called if the send window has been resized.
 * @param event
 *      The resize event.
 */
void SendWindow::resizeEvent(QResizeEvent * event)
{
    (void)event;
    resizeTableColumnsSlot();
}

/**
 * Shows the send window.
 */
void SendWindow::show(void)
{
    QWidget::show();

    m_userInterface->tableWidget->setRowCount(0);
    if(!m_currentSequenceFileName.isEmpty())
    {
        loadTableData();

    }
    else
    {
        resizeTableColumnsSlot();
    }
}

/**
 * Returns the names of all sequences.
 */
QStringList SendWindow::getAllSequences(void)
{
    QStringList sequences;

    for( int r = 0; r < m_userInterface->tableWidget->rowCount(); ++r )
    {
        sequences << m_userInterface->tableWidget->item(r,COLUMN_NAME)->text();
    }
    return sequences;
}

/**
 * Loads a saved sequence table.
 */
void SendWindow::loadTableData(void)
{

    setTitle(m_currentSequenceFileName);

    m_userInterface->tableWidget->setRowCount(0);

    if(!m_currentSequenceFileName.isEmpty())
    {
        QFile file(m_currentSequenceFileName);
        QDomDocument doc("sequences");

        if (file.open(QFile::ReadOnly))
        {
            file.close();


            if (!doc.setContent(&file))
            {
                if(!file.readAll().isEmpty())
                {
                    QMessageBox::critical(this, "parse error", "could not parse " + m_currentSequenceFileName);

                    m_currentSequenceFileName = "";
                    setTitle(m_currentSequenceFileName);
                    emit configHasToBeSavedSignal();
                }
            }
            else
            {
                QDomElement docElem = doc.documentElement();

                m_userInterface->tableWidget->blockSignals(true);

                QDomNodeList itemList = docElem.elementsByTagName("fileInfo");
                QDomNode nodeItem = itemList.at(0);
                bool hasBeenSaved = nodeItem.attributes().namedItem("hasBeenSaved").nodeValue().toUInt();

                itemList = docElem.elementsByTagName("ItemList");
                nodeItem = itemList.at(0);



                quint32 width = nodeItem.attributes().namedItem("width0").nodeValue().toUInt();
                if(width != 0) m_userInterface->tableWidget->setColumnWidth(COLUMN_NAME, width);

                width = nodeItem.attributes().namedItem("width1").nodeValue().toUInt();
                if(width != 0) m_userInterface->tableWidget->setColumnWidth(COLUMN_FORMAT, width);

                width = nodeItem.attributes().namedItem("width2").nodeValue().toUInt();
                if(width != 0) m_userInterface->tableWidget->setColumnWidth(COLUMN_VALUE, width);

                QDomNodeList nodeList = docElem.elementsByTagName("SequenceItem");
                for (int x = nodeList.size() - 1; x >= 0; x--)
                {
                    QDomNode nodeSequenceItem = nodeList.at(x);
                    QDomNode nodeSequence = nodeList.at(x).namedItem("sequence");
                    blockSignals(true);
                    newButtonClickedSlot();
                    blockSignals(false);

                    m_userInterface->tableWidget->item(0,COLUMN_NAME)->setText(nodeSequenceItem.attributes().namedItem("name").nodeValue());

                    QString path = nodeSequence.attributes().namedItem("script").nodeValue();
                    if(m_mainWindow->isFirstProgramStart() || !hasBeenSaved)
                    {
                        if(path.startsWith("./")){path.replace("./", MainWindow::getScriptCommunicatorFilesFolder() + "/");}
                    }
                    else
                    {
                        path = MainWindow::convertToAbsolutePath(m_currentSequenceFileName, path);
                    }
                    SequenceTablePlainTextEdit* textEdit = static_cast<SequenceTablePlainTextEdit*>(m_userInterface->tableWidget->cellWidget(0, COLUMN_SCRIPT));
                    textEdit->setPlainText(path);

                    quint32 rowHeight = nodeSequence.attributes().namedItem("height").nodeValue().toUInt();

                    if(rowHeight != 0)
                    {
                        m_userInterface->tableWidget->setRowHeight(0, rowHeight);

                    }

                    QString format = nodeSequence.attributes().namedItem("format").nodeValue();
                    SequenceTableComboBox* box = static_cast<SequenceTableComboBox*>(m_userInterface->tableWidget->cellWidget(0, COLUMN_FORMAT));
                    box->blockSignals(true);
                    box->setCurrentText(format);
                    box->blockSignals(false);

                    textEdit = static_cast<SequenceTablePlainTextEdit*>(m_userInterface->tableWidget->cellWidget(0,COLUMN_VALUE));
                    textEdit->blockSignals(true);
                    m_userInterface->tableWidget->item(0, COLUMN_VALUE)->setData(Qt::UserRole + 1, format);
                    textEdit->setPlainText(nodeSequence.attributes().namedItem("value").nodeValue());
                    textEdit->blockSignals(false);

                    checkTextEditCell(0);

                }//for (int x = 0; x < nodeList.size(); x++)

                emit sequenceTableHasChangedSignal();

                m_userInterface->tableWidget->blockSignals(false);
                QStringList showStrList = m_currentSequenceFileName.split("/");
                statusBar()->showMessage(showStrList[showStrList.size() - 1] + " loaded", 3000);

                if(!hasBeenSaved)
                {
                    saveFileSlot();
                }

                m_currentSequenceFileString = tableToString();

            }//else: if(!doc.setContent(&file))
        }
        else
        {
            QMessageBox::critical(this, "could not open file", m_currentSequenceFileName);

            m_currentSequenceFileName = "";
            setTitle(m_currentSequenceFileName);
            emit configHasToBeSavedSignal();
        }

        resizeTableColumnsSlot();
        emit sequenceTableHasChangedSignal();
    }

    itemSelectionChangedSlot();

}

/**
 * Converts the sequence table to a string.
 * @return
 *      The created string.
 */
QString SendWindow::tableToString(void)
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

        xmlWriter.writeStartElement("SequenceConfig");

        xmlWriter.writeStartElement("fileInfo");
        xmlWriter.writeAttribute("version", MainWindow::VERSION);
        xmlWriter.writeAttribute("hasBeenSaved", "1");
        xmlWriter.writeEndElement();

        xmlWriter.writeStartElement("ItemList");
        xmlWriter.writeAttribute("width0", QString("%1").arg(m_userInterface->tableWidget->columnWidth(COLUMN_NAME)));
        xmlWriter.writeAttribute("width1", QString("%1").arg(m_userInterface->tableWidget->columnWidth(COLUMN_FORMAT)));
        xmlWriter.writeAttribute("width2", QString("%1").arg(m_userInterface->tableWidget->columnWidth(COLUMN_VALUE)));

        for( int r = 0; r < m_userInterface->tableWidget->rowCount(); ++r )
        {

            QTableWidgetItem* item1 = m_userInterface->tableWidget->item(r,COLUMN_NAME);
            SequenceTablePlainTextEdit* lineEdit = static_cast<SequenceTablePlainTextEdit*>(m_userInterface->tableWidget->cellWidget(r, COLUMN_VALUE));
            SequenceTablePlainTextEdit* scriptTextEdit = static_cast<SequenceTablePlainTextEdit*>(m_userInterface->tableWidget->cellWidget(r, COLUMN_SCRIPT));

            if(item1 && lineEdit)
            {

                xmlWriter.writeStartElement("SequenceItem");
                xmlWriter.writeAttribute("name", item1->text());

                xmlWriter.writeStartElement("sequence");

                SequenceTableComboBox* box = static_cast<SequenceTableComboBox*>(m_userInterface->tableWidget->cellWidget(r, COLUMN_FORMAT));

                xmlWriter.writeAttribute("format", box->currentText());
                xmlWriter.writeAttribute("value", lineEdit->toPlainText());
                xmlWriter.writeAttribute("script", MainWindow::convertToRelativePath(m_currentSequenceFileName, scriptTextEdit->toPlainText()));
                xmlWriter.writeAttribute("height", QString("%1").arg(m_userInterface->tableWidget->rowHeight(r)));
                xmlWriter.writeEndElement();//"sequence "

                xmlWriter.writeEndElement();//"SequenceItem"
            }

        }

        xmlWriter.writeEndElement();//"ItemList"
        xmlWriter.writeEndElement();//"SequenceFile"

        result = xmlBuffer.data();
    }

    return result;
}


/**
 * Saves the sequence table.
 */
void SendWindow::saveTable(void)
{
    QFile file(m_currentSequenceFileName);
    file.remove();
    if(file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QTextStream data( &file );
        data.setCodec("UTF-8");
        m_currentSequenceFileString = tableToString();
        data << m_currentSequenceFileString;
        file.close();

        QStringList showStrList = m_currentSequenceFileName.split("/");
        statusBar()->showMessage(showStrList[showStrList.size() - 1] + " saved", 3000);
    }
    else
    {
        QMessageBox::critical(this, "save failed", m_currentSequenceFileName);

        m_currentSequenceFileName = "";
        setTitle(m_currentSequenceFileName);
        emit configHasToBeSavedSignal();
    }
}

/**
 * Slot function for the load file button.
 */
void SendWindow::loadFileSlot(void)
{
    checkTableChanged();

    QString tmpFileName = QFileDialog::getOpenFileName(this, tr("Open sequence config"),
                                                       m_mainWindow->getAndCreateProgramUserFolder(), tr("sequence config files (*.seq);;Files (*)"));
    if(!tmpFileName.isEmpty())
    {
        m_currentSequenceFileName = tmpFileName;
        setTitle(m_currentSequenceFileName);
        emit configHasToBeSavedSignal();
        m_userInterface->tableWidget->setRowCount(0);
        loadTableData();

    }
    itemSelectionChangedSlot();
    emit sequenceTableHasChangedSignal();
}



/**
 * Slot function for the save file button.
 */
void SendWindow::saveFileSlot(void)
{
    if(!m_currentSequenceFileName.isEmpty())
    {
        saveTable();
    }
    else
    {
        saveAsFileSlot();
    }
}

/**
 * Checks if the content of the sequence table has been changed. And saves the table it has been changed.
 */
void SendWindow::checkTableChanged()
{

    if(tableToString() != m_currentSequenceFileString)
    {

        if(m_currentSequenceFileName.isEmpty())
        {
            QMessageBox messageBox(QMessageBox::Question, "sequence file changed",
                                   "Save sequence file?", QMessageBox::Yes | QMessageBox::No);
            messageBox.exec();
            if(messageBox.result() == QMessageBox::Yes)
            {
                saveFileSlot();
            }
            else
            {
                m_currentSequenceFileString = "";
            }
        }
        else
        {
            saveFileSlot();
        }
    }
}

/**
 * Sets the window title.
 * @param extraString
 *      The string which is appended at the title.
 */
void SendWindow::setTitle(QString extraString)
{
    setWindowTitle("ScriptCommunicator " + MainWindow::VERSION + " - Send " + extraString);
}

/**
 * Slot function for the edit cyclic send menu.
 */
void SendWindow::editCyclicSendScriptSlot(void)
{
    QStringList arguments;
    arguments << m_userInterface->CyclicSendScript->toPlainText() ;
    MainWindow::openScriptEditor(arguments, m_settingsDialog->settings(), this);
}

/**
 * Slot function for the add cyclic send menu.
 */
void SendWindow::addCyclicSendScriptSlot(void)
{
    QString tmpFileName = QFileDialog::getOpenFileName(this, tr("Open script file"),
                                                       "", tr("script files (*.js);;Files (*)"));
    if(!tmpFileName.isEmpty())
    {
        m_userInterface->CyclicSendScript->setPlainText(tmpFileName);
    }
}

/**
 * Slot function for the unload button.
 */
void SendWindow::unloadFileSlot(void)
{
    checkTableChanged();

    m_userInterface->tableWidget->setRowCount(0);

    m_currentSequenceFileName = "";
    setTitle(m_currentSequenceFileName);
    m_currentSequenceFileString = "";
    itemSelectionChangedSlot();
    emit configHasToBeSavedSignal();
    emit sequenceTableHasChangedSignal();

}

/**
 * Slot function for the save file as button.
 */
void SendWindow::saveAsFileSlot(void)
{
    QString tmpFileName = QFileDialog::getSaveFileName(this, tr("Save sequence config file"),
                                                       m_mainWindow->getAndCreateProgramUserFolder(), tr("sequence config files (*.seq);;Files (*)"));
    if(!tmpFileName.isEmpty())
    {
        m_currentSequenceFileName = tmpFileName;
        setTitle(m_currentSequenceFileName);
        emit configHasToBeSavedSignal();
        saveTable();
    }
}

/**
 * Slot function for the edit all sequence scripts menu.
 */
void SendWindow::editAllSequenceScriptsSlot(void)
{
    if( m_userInterface->tableWidget->rowCount() != 0)
    {
        QStringList arguments;
        for( int r = 0; r < m_userInterface->tableWidget->rowCount(); ++r )
        {
            SequenceTablePlainTextEdit* scriptTextEdit = static_cast<SequenceTablePlainTextEdit*>(m_userInterface->tableWidget->cellWidget(r, COLUMN_SCRIPT));
            QString script = scriptTextEdit->toPlainText();
            if(!script.isEmpty())
            {
                arguments << script;
            }
        }
        MainWindow::openScriptEditor(arguments, m_mainWindow->getSettingsDialog()->settings(), this);
    }
}

/**
 * This function is called if the main window is e.
 * @param event
 *      The close event.
 */
void SendWindow::closeEvent(QCloseEvent * event)
{
    checkTableChanged();
    event->accept();
}


/**
 * Swaps the position of 2 table rows.
 * @param row1
 *      Row 1.
 * @param row2
 *      Row 2.
 */
void SendWindow::swapTableRowPositions(int row1, int row2)
{
    int colCount = m_userInterface->tableWidget->columnCount();

    m_userInterface->tableWidget->blockSignals(true);
    SequenceTableComboBox * box1 = static_cast<SequenceTableComboBox*>(m_userInterface->tableWidget->cellWidget(row1, COLUMN_FORMAT));
    SequenceTableComboBox * box2 = static_cast<SequenceTableComboBox*>(m_userInterface->tableWidget->cellWidget(row2, COLUMN_FORMAT));

    SequenceTablePlainTextEdit * listEdit1 = static_cast<SequenceTablePlainTextEdit*>(m_userInterface->tableWidget->cellWidget(row1, COLUMN_VALUE));
    SequenceTablePlainTextEdit * listEdit2 = static_cast<SequenceTablePlainTextEdit*>(m_userInterface->tableWidget->cellWidget(row2, COLUMN_VALUE));

    SequenceTablePlainTextEdit * scriptEdit1 = static_cast<SequenceTablePlainTextEdit*>(m_userInterface->tableWidget->cellWidget(row1, COLUMN_SCRIPT));
    SequenceTablePlainTextEdit * scriptEdit2 = static_cast<SequenceTablePlainTextEdit*>(m_userInterface->tableWidget->cellWidget(row2, COLUMN_SCRIPT));


    box1->blockSignals(true);
    box2->blockSignals(true);
    listEdit1->blockSignals(true);
    listEdit2->blockSignals(true);
    scriptEdit1->blockSignals(true);
    scriptEdit2->blockSignals(true);

    QList<QTableWidgetItem*> rowItems1,rowItems2;

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

    QString tmpFormat = box1->currentText();
    box1->setCurrentText(box2->currentText());
    box2->setCurrentText(tmpFormat);

    box1->setRow(row1);
    box2->setRow(row2);


    QString tmpValue = listEdit1->toPlainText();
    listEdit1->setPlainText(listEdit2->toPlainText());
    listEdit2->setPlainText(tmpValue);

    listEdit1->setRow(row1);
    listEdit2->setRow(row2);

    tmpValue = scriptEdit1->toPlainText();
    scriptEdit1->setPlainText(scriptEdit2->toPlainText());
    scriptEdit2->setPlainText(tmpValue);

    scriptEdit1->setRow(row1);
    scriptEdit2->setRow(row2);

    m_userInterface->tableWidget->blockSignals(false);
    box1->blockSignals(false);
    box2->blockSignals(false);
    listEdit1->blockSignals(false);
    listEdit2->blockSignals(false);
    scriptEdit1->blockSignals(false);
    scriptEdit2->blockSignals(false);

    emit sequenceTableHasChangedSignal();
}

/**
 * This slot function is called if the user enters a cell in the script table.
 * With this function the user can move the selected row up or down (while holding the mouse at the row)
 * @param row
 *      The row of the cell.
 * @param column
 *      The column of the cell.
 */
void SendWindow::cellEnteredSlot(int row, int column)
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
 * Sets the connection status of the main interface thread (true for connected)
 * @param connected
 *      True for connected.
 */
void SendWindow::setIsConnected(bool connected)
{
    m_isConnected = connected;
}

/**
 * Sets the value of the progress bar.
 * @param value
 *      The new value.
 */
void SendWindow::setProgressbarValue(int value)
{
    m_userInterface->SendProgressBar->setValue(value);
}

/**
 * Converts a string into a bytes array.
 * @param formatString
 *      The format of the string. Possible value are:
 *      - decimal
 *      - hex
 *      - ascii
 * @param text
 *      The string.
 * @param decimalType
 *      The decimal type.
 * @param endianess
 *      The endianess of the data.
 * @return
 *      The created byte array.
 */
QByteArray SendWindow::textToByteArray(QString formatString, QString text, DecimalType decimalType, Endianess endianess)
{
    QByteArray dataArray;
    QStringList strList = text.split(" ");

    if(formatString != "ascii")
    {
        uint format = 10;
        qint32 bytesPerNumber = 1;

        if(formatString == "hex")
        {
            format = 16;
            bytesPerNumber = 2;
        }
        else if(formatString == "bin")
        {
            format = 2;
            bytesPerNumber = 8;
        }

        for(auto var : strList)
        {
            bool isOk = false;

            if((formatString == "hex") || (formatString == "bin"))
            {
                while(!var.isEmpty())
                {
                    QString tmpStr = var.left(bytesPerNumber);
                    var.remove(0, bytesPerNumber);

                    dataArray.append(tmpStr.toUInt(&isOk,format));
                }
            }
            else
            {
                quint32 value = 0;

                if(decimalType == DECIMAL_TYPE_UINT16)
                {
                    bytesPerNumber = 2;
                    value = (quint32)var.toULongLong(&isOk,format);
                }
                else if(decimalType == DECIMAL_TYPE_INT16)
                {
                    bytesPerNumber = 2;
                    value = (quint32)var.toLongLong(&isOk,format);
                }
                else if(decimalType == DECIMAL_TYPE_UINT32)
                {
                    bytesPerNumber = 4;
                    value = (quint32)var.toULongLong(&isOk,format);
                }
                else if(decimalType == DECIMAL_TYPE_INT32)
                {
                    bytesPerNumber = 4;
                    value = (quint32)var.toLongLong(&isOk,format);
                }
                else if(decimalType == DECIMAL_TYPE_UINT8)
                {
                    bytesPerNumber = 1;
                    value = (quint32)var.toULongLong(&isOk,format);
                }
                else
                {
                    value = (quint32)var.toLongLong(&isOk,format);
                }

                for(int k = 0; k < bytesPerNumber; k++)
                {
                    if(endianess == LITTLE_ENDIAN_TARGET)
                    {
                        dataArray.append((quint8)(value >> (8 * k)));
                    }
                    else
                    {
                        dataArray.append((quint8)(value >> (8 * (bytesPerNumber - (k + 1)))));
                    }
                }

            }

        }
    }
    else
    {
        text.replace("<|#CR#|>", "\r");
        dataArray = text.toLocal8Bit();
    }

    return dataArray;

}

/**
 * Returns the current send string (from to gui).
 * @return
 *      The send string.
 */
QString SendWindow::getCurrentSendString()
{
    return m_userInterface->CyclicSendInput->toPlainText();
}

/**
 * Returns Returns the current cyclic script (from to gui).
 * @return
 *      The current cyclic script.
 */
QString SendWindow::getCurrentCyclicScript()
{
    return m_userInterface->CyclicSendScript->toPlainText();
}

/**
 * Sets the current cyclic script.
 * @param script
 *      The new script.
 */
void SendWindow::setCurrentCyclicScript(QString script)
{
    if(script.startsWith("./")){script.replace("./", MainWindow::getScriptCommunicatorFilesFolder() + "/");}
    m_userInterface->CyclicSendScript->setPlainText(script);
}


/**
 * Sets the current send string (in the gui).
 * @param text
 *      The new send string.
 */
void SendWindow::setCurrentSendString(QString text)
{
    m_userInterface->CyclicSendInput->setPlainText(text);
    checkCyclicSendInputSlot();
}

/**
 * Returns the current send string format (from the gui).
 * @return
 *      The format string.
 */
QString SendWindow::getCurrentSendStringFormat()
{
    return m_userInterface->CyclicSendFormat->currentText();
}

/**
 * Sets the current send string format (in the gui).
 * @param text
 *      The new format string.
 */
void SendWindow::setCurrentSendStringFormat(QString text)
{
    m_userInterface->CyclicSendFormat->blockSignals(true);
    m_userInterface->CyclicSendFormat->setCurrentText(text);
    m_userInterface->CyclicSendFormat->blockSignals(false);
    m_oldSendStringFormat = text;
}

/**
 * Returns the current send repetition value (from the gui).
 * @return
 *      The current send repetition.
 */
QString SendWindow::getCurrentSendRepetition()
{
    return m_userInterface->CyclicSendRepetition->text();
}

/**
 * Sets the current send repetition value(in the gui).
 * @param text
 *      The new send repetition.
 */
void SendWindow::setCurrentSendRepetition(QString text)
{
    m_userInterface->CyclicSendRepetition->setText(text);
}

/**
 * Returns the current send pause value (from the gui).
 * @return
 *      The current send pause.
 */
QString SendWindow::getCurrentSendPause()
{
    return m_userInterface->CyclicSendPause->text();
}

/**
 * Sets the current send pause value (in the gui).
 * @param text
 *      The send pause.
 */
void SendWindow::setCurrentSendPause(QString text)
{
    m_userInterface->CyclicSendPause->setText(text);
}

/**
 * Returns true if the cyclic data should be added to the send history.
 */
bool SendWindow::getAddToHistoryCheckBox(void)
{
    return m_userInterface->addToHistoryCheckBox->isChecked();
}

/**
 * True if the cyclic data should be added to the send history.
 * @param add
 *      True for add.
 */
void SendWindow::setAddToHistoryCheckBox(bool add)
{
    m_userInterface->addToHistoryCheckBox->setChecked(add);
}

/**
 * Slot function for the send button.
 */
void SendWindow::sendButtonPressedSlot()
{
    if(m_cyclicSendingIsInProgress)
    {
        currentCyclicSendFinished();

        m_userInterface->tableWidget->closeDebugger(false);
    }
    else
    {
        checkCyclicSendInputSlot();

        const Settings* settings = m_settingsDialog->settings();
        QByteArray sendData = textToByteArray(m_userInterface->CyclicSendFormat->currentText(), m_userInterface->CyclicSendInput->toPlainText(),
                                              formatToDecimalType(m_userInterface->CyclicSendFormat->currentText()), settings->targetEndianess);
        if(m_userInterface->CyclicSendFormat->currentText() == "ascii")
        {
            const Settings* settings = m_settingsDialog->settings();
            sendData.replace("\n", settings->consoleSendOnEnter.toLocal8Bit());
        }
        if(!sendData.isEmpty())
        {
            sendDataWithTheMainInterface(sendData, this,
                                         m_userInterface->CyclicSendRepetition->text().toInt(),
                                         m_userInterface->CyclicSendPause->text().toInt(), true, m_userInterface->CyclicSendScript->toPlainText());
        }
    }
}

///Is called if text of the cyclic script text edit has been changed.
void SendWindow::cyclicScriptTextEditChangedSlot(void)
{
    if(m_userInterface->CyclicSendScript->toPlainText().isEmpty())
    {
        m_userInterface->actionEditCyclicScript->setEnabled(false);
    }
    else
    {
        m_userInterface->actionEditCyclicScript->setEnabled(true);
    }

    m_userInterface->actionDebugCyclicSequenceScript->setEnabled(false);
    if(!m_userInterface->CyclicSendInput->toPlainText().isEmpty() &&
            !m_userInterface->CyclicSendScript->toPlainText().isEmpty())
    {
        m_userInterface->actionDebugCyclicSequenceScript->setEnabled(true);
    }
}

///Is called when the user double clicks the cyclic script text edit.
void SendWindow::cyclicScriptTextEditDoubleClickedSlot(void)
{
    addCyclicSendScriptSlot();
}

/**
 * Sends a sequence.
 * @param sequenceIndex
 *      The sequence index.
 * @param debug
 *      True if the script shall be executed in the script debugger.
 * @param callerWidget
 *      The caller widget.
 */
void SendWindow::sendSequence(quint32 sequenceIndex, bool debug, QWidget* callerWidget)
{
    m_userInterface->tableWidget->sendSequence(sequenceIndex, debug, callerWidget);
}

/**
 * This function is called if an error during a cyclic sending has occured.
 */
void SendWindow::cyclicSendErrorReceived(void)
{
    if(m_cyclicSendingIsInProgress)
    {
        m_currentSendTimer.stop();

        m_cyclicSendingIsInProgress = false;

        if(!m_isConnected)
        {
            QMessageBox::critical(this->isVisible() ? this : NULL, "error", "error while sending: no connection");
        }
        else
        {
            const Settings* settings = m_settingsDialog->settings();

            if((settings->connectionType == CONNECTION_TYPE_AARDVARK) &&
               ((settings->aardvarkI2cSpi.deviceMode == AARDVARK_I2C_SPI_DEVICE_MODE_I2C_SLAVE) ||
                (settings->aardvarkI2cSpi.deviceMode == AARDVARK_I2C_SPI_DEVICE_MODE_SPI_SLAVE)))
            {//I2C/SPI slave.

                QMessageBox::information(this->isVisible() ? this : NULL, "Aardvark I2C/SPI", "set slave response failed");
            }
            else
            {
                QMessageBox::critical(this->isVisible() ? this : NULL, "error", "error while sending");
            }
        }

        enableWindowForCyclicSend(true);
    }
}

/**
 * The function is called when the current cyclic send process has been finished.
 */
void SendWindow::currentCyclicSendFinished(void)
{
    m_currentSendTimer.stop();

    m_cyclicSendingIsInProgress = false;
    enableWindowForCyclicSend(true);
    setProgressbarValue(100);

    const Settings* settings = m_settingsDialog->settings();
    if((settings->connectionType == CONNECTION_TYPE_AARDVARK) &&
       ((settings->aardvarkI2cSpi.deviceMode == AARDVARK_I2C_SPI_DEVICE_MODE_I2C_SLAVE) ||
        (settings->aardvarkI2cSpi.deviceMode == AARDVARK_I2C_SPI_DEVICE_MODE_SPI_SLAVE)))
    {//I2C/SPI slave.

        QMessageBox::information(this->isVisible() ? this : NULL, "Aardvark I2C/SPI", "set slave response succeeded");
    }
}

/**
 * Is called if data has been sent with the main interface.
 * @param success
 *      True on success.
 * @param id
 *      The send id.
 */
void SendWindow::dataHasBeenSendSlot(bool success, uint id)
{
    if(id == MainInterfaceThread::SEND_ID_SEND_WINDOW_CYCLIC)
    {
        if(success)
        {
            if(m_cyclicSendingIsInProgress)
            {

                if(!m_programIsClosing)
                {
                    m_currentSendNumberOfSends++;

                    int currentProgress = 0;
                    int progressPerRepitition = 0;

                    if(m_currentSendRepetitionCount != 0)
                    {
                        progressPerRepitition = (100 * MAX_REPETITIONS) / (m_currentSendRepetitionCount + 1);
                    }
                    else
                    {
                        progressPerRepitition = 100;
                    }

                    currentProgress = (m_currentSendNumberOfSends * progressPerRepitition) / MAX_REPETITIONS;
                    setProgressbarValue(currentProgress);

                    if(m_currentSendNumberOfSends > m_currentSendRepetitionCount)
                    {
                        m_userInterface->tableWidget->closeDebugger(false);
                        currentCyclicSendFinished();
                    }
                    else
                    {
                        if(m_currentSendPause == 0)
                        {
                            sendTimerElapsedSlot();
                        }
                        else
                        {
                            m_currentSendTimer.start(m_currentSendPause);
                        }
                    }
                }
                else
                {
                    m_userInterface->tableWidget->closeDebugger(false);
                    m_currentSendTimer.stop();
                    m_cyclicSendingIsInProgress = false;
                }
            }

        }
        else
        {
            m_userInterface->tableWidget->closeDebugger(false);
            cyclicSendErrorReceived();
        }


    }
    else if(id == MainInterfaceThread::SEND_ID_SEND_WINDOW_SINGLE)
    {
        const Settings* settings = m_settingsDialog->settings();

        if(!success)
        {
            if(!m_isConnected)
            {
                QMessageBox::critical(this->isVisible() ? this : NULL, "error", "error while sending: no connection");
            }
            else
            {
                if((settings->connectionType == CONNECTION_TYPE_AARDVARK) &&
                   ((settings->aardvarkI2cSpi.deviceMode == AARDVARK_I2C_SPI_DEVICE_MODE_I2C_SLAVE) ||
                    (settings->aardvarkI2cSpi.deviceMode == AARDVARK_I2C_SPI_DEVICE_MODE_SPI_SLAVE)))
                {//I2C/SPI slave.

                    QMessageBox::information(this->isVisible() ? this : NULL, "Aardvark I2C/SPI", "set slave response failed");
                }
                else
                {
                    QMessageBox::critical(this->isVisible() ? this : NULL, "error", "error while sending");
                }
            }
        }
        else
        {
            if((settings->connectionType == CONNECTION_TYPE_AARDVARK) &&
               ((settings->aardvarkI2cSpi.deviceMode == AARDVARK_I2C_SPI_DEVICE_MODE_I2C_SLAVE) ||
                (settings->aardvarkI2cSpi.deviceMode == AARDVARK_I2C_SPI_DEVICE_MODE_SPI_SLAVE)))
            {//I2C/SPI slave.

                QMessageBox::information(this->isVisible() ? this : NULL, "Aardvark I2C/SPI", "set slave response succeeded");
            }

        }

        m_userInterface->tableWidget->closeDebugger(true);
    }
}

/**
 * The slot function is called if the the current transmission (sending of data) has been finished.
 * @param data
 *      The data.
 * @param callerWidget
 *      The caller widget.
 * @param repetitionCount
 *      The number of repetitions.
 * @param pause
 *      The pause between two repetitions.
 * @param scriptName
 *      The name of the script.
 * @param debug
 *      True if the script shall be executed in the script debugger.
 */
void SendWindow::sendDataWithTheMainInterface(const QByteArray &data, QWidget* callerWidget, int repetitionCount, int pause, bool isCyclicSend, QString scriptName, bool debug)
{
    if(!m_isConnected)
    {
        QMessageBox::critical(callerWidget, "error", "sending failed: no connection");
    }


    if(m_isConnected && (data.size() > 0) &&  !m_programIsClosing)
    {

        SequenceScriptEngineWrapper* scriptEngineWrapper = 0;

        QByteArray sendData;

        if(!scriptName.isEmpty())
        {
            sendData = m_userInterface->tableWidget->executeScript(scriptName, data, &scriptEngineWrapper, !isCyclicSend, debug, true);
        }
        else
        {
            sendData = data;
        }

        if(!sendData.isEmpty())
        {
            if(isCyclicSend && !m_cyclicSendingIsInProgress)
            {
                m_cyclicSendingIsInProgress = true;

                enableWindowForCyclicSend(false);

                m_currentSendData = data;
                m_currentSendRepetitionCount = repetitionCount;
                m_currentSendPause = pause;
                m_currentSendNumberOfSends = 0;
                m_currentSendScript = scriptName;

                if(m_currentScriptEngineWrapper != 0)
                {
                    m_currentScriptEngineWrapper->deleteLater();
                }
                m_currentScriptEngineWrapper = scriptEngineWrapper;

                emit sendDataWithTheMainInterfaceSignal(sendData, MainInterfaceThread::SEND_ID_SEND_WINDOW_CYCLIC);
                if(m_userInterface->addToHistoryCheckBox->isChecked())
                {
                    m_mainWindow->getHandleDataObject()->addDataToSendHistory(&sendData);
                }
            }
            else if(!isCyclicSend)
            {
                emit sendDataWithTheMainInterfaceSignal(sendData, MainInterfaceThread::SEND_ID_SEND_WINDOW_SINGLE);
                m_mainWindow->getHandleDataObject()->addDataToSendHistory(&sendData);
                if(scriptEngineWrapper != 0)
                {
                    scriptEngineWrapper->deleteLater();
                }
            }
            else
            {
                if(scriptEngineWrapper != 0)
                {
                    scriptEngineWrapper->deleteLater();
                }
            }
        }// if(!sendData.isEmpty())
        else
        {
            m_userInterface->tableWidget->closeDebugger(!isCyclicSend);

            if(scriptEngineWrapper != 0)
            {
                scriptEngineWrapper->deleteLater();
            }
        }

    }
}

/**
 * Enables or disable the send window for cyclic sending.
 * @param enable
 *      True of enable.
 */
void SendWindow::enableWindowForCyclicSend(bool enable)
{

    if(enable)
    {

        m_userInterface->SendPushButton->setText("send");

        QPalette tmpPalette = m_userInterface->CyclicSendInput->palette();
        tmpPalette.setColor(QPalette::Base, QColor(255,255,255));
        m_userInterface->CyclicSendInput->setEnabled(true);
        m_userInterface->CyclicSendInput->setPalette(tmpPalette);

        m_userInterface->CyclicSendScript->setEnabled(true);
        m_userInterface->CyclicSendScript->setPalette(tmpPalette);

        m_userInterface->CyclicSendRepetition->setEnabled(true);
        m_userInterface->CyclicSendRepetition->setPalette(tmpPalette);

        m_userInterface->CyclicSendPause->setEnabled(true);
        m_userInterface->CyclicSendPause->setPalette(tmpPalette);

        m_userInterface->CyclicSendFormat->setEnabled(true);
        m_userInterface->actionAddCyclicScript->setEnabled(true);

        cyclicScriptTextEditChangedSlot();

    }
    else
    {

        m_userInterface->SendPushButton->setText("cancel");

        QPalette tmpPalette = m_userInterface->CyclicSendInput->palette();
        tmpPalette.setColor(QPalette::Base, QColor(190,190,190));
        m_userInterface->CyclicSendInput->setEnabled(false);
        m_userInterface->CyclicSendInput->setPalette(tmpPalette);

        m_userInterface->CyclicSendScript->setEnabled(false);
        m_userInterface->CyclicSendScript->setPalette(tmpPalette);

        m_userInterface->CyclicSendRepetition->setEnabled(false);
        m_userInterface->CyclicSendRepetition->setPalette(tmpPalette);

        m_userInterface->CyclicSendPause->setEnabled(false);
        m_userInterface->CyclicSendPause->setPalette(tmpPalette);

        m_userInterface->CyclicSendFormat->setEnabled(false);
        m_userInterface->actionEditCyclicScript->setEnabled(false);
        m_userInterface->actionAddCyclicScript->setEnabled(false);

        m_userInterface->actionDebugCyclicSequenceScript->setEnabled(false);


    }

    itemSelectionChangedSlot();
}

/**
 * This slot function is called by m_currentSendTimer.
 */
void SendWindow::sendTimerElapsedSlot(void)
{
    if(m_currentSendPause != 0)
    {
        m_currentSendTimer.stop();
    }

    if(!m_currentSendScript.isEmpty())
    {//The sequence has a script.

        QByteArray sendData = m_userInterface->tableWidget->executeScript(m_currentSendScript, m_currentSendData,
                                                                          &m_currentScriptEngineWrapper, false, m_currentScriptEngineWrapper->runsInDebugger);
        if(!sendData.isEmpty())
        {
            emit sendDataWithTheMainInterfaceSignal(sendData, MainInterfaceThread::SEND_ID_SEND_WINDOW_CYCLIC);
            if(m_userInterface->addToHistoryCheckBox->isChecked())
            {
                m_mainWindow->getHandleDataObject()->addDataToSendHistory(&sendData);
            }
        }
        else
        {
            m_currentSendTimer.stop();
            m_cyclicSendingIsInProgress = false;
            enableWindowForCyclicSend(true);
            m_userInterface->tableWidget->closeDebugger(false);
        }
    }
    else
    {
        emit sendDataWithTheMainInterfaceSignal(m_currentSendData, MainInterfaceThread::SEND_ID_SEND_WINDOW_CYCLIC);
        if(m_userInterface->addToHistoryCheckBox->isChecked())
        {
            m_mainWindow->getHandleDataObject()->addDataToSendHistory(&m_currentSendData);
        }
    }


}

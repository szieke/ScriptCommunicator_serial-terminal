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

#include "sequencetableview.h"
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
#include "scriptwindow.h"
#include <QScriptEngineDebugger>


//Global sequence data maps (sequences can store data here).
static QMap<QString, QString> g_stringMap;
static QMap<QString, QVector<unsigned char>> g_dataMap;
static QMap<QString, quint32> g_unsignedNumberMap;
static QMap<QString, qint32> g_signedNumberMap;

//Mutexes for the global data maps access.
static QMutex g_stringMapMutex;
static QMutex g_dataMapMutex;
static QMutex g_unsignedNumberMapMutex;
static QMutex g_signedNumberMapMutex;

///Is set to true if a thread has been terminated.
///This variabke is used un the main function.
extern bool g_aThreadHasBeenTerminated;

///Sets a string in the global sequence string map.
void SequenceScriptThread::setGlobalString(QString name, QString string)
{
    g_stringMapMutex.lock();
    g_stringMap[name] = string;
    g_stringMapMutex.unlock();
}

///Returns a string from the global sequence string map.
///Note: Returns an empty string if name is not in the map.
QString SequenceScriptThread::getGlobalString(QString name, bool removeValue)
{
    QString result;
    g_stringMapMutex.lock();

    if(g_stringMap.contains(name))
    {
        result = g_stringMap[name];

        if(removeValue)
        {
            g_stringMap.remove(name);
        }
    }
    g_stringMapMutex.unlock();
    return result;
}

///Sets a data vector in the global sequence data vector map.
void SequenceScriptThread::setGlobalDataArray(QString name, QVector<unsigned char> data)
{
    g_dataMapMutex.lock();
    g_dataMap[name] = data;
    g_dataMapMutex.unlock();
}

///Returns a data vector from the global sequence data vector map.
///Note: Returns an empty data vector if name is not in the map.
QVector<unsigned char> SequenceScriptThread::getGlobalDataArray(QString name, bool removeValue)
{
    QVector<unsigned char> result;
    g_dataMapMutex.lock();

    if(g_dataMap.contains(name))
    {
      result = g_dataMap[name];
      if(removeValue)
      {
          g_dataMap.remove(name);
      }
    }
    g_dataMapMutex.unlock();
    return result;
}

///Sets a unsigned number in the global sequence unsigned number map.
void SequenceScriptThread::setGlobalUnsignedNumber(QString name, quint32 number)
{
    g_unsignedNumberMapMutex.lock();
    g_unsignedNumberMap[name] = number;
    g_unsignedNumberMapMutex.unlock();
}

///Returns a unsigned number from the global sequence unsigned number map.
///Returns an quint32 array.
///The first element is the result status (1=name found, 0=name not found)
///The second element is the read value.
QList<quint32> SequenceScriptThread::getGlobalUnsignedNumber(QString name, bool removeValue)
{
    QList<quint32> result;
    g_unsignedNumberMapMutex.lock();

    if(g_unsignedNumberMap.contains(name))
    {
        result.append(1);
        result.append(g_unsignedNumberMap[name]);

        if(removeValue)
        {
            g_unsignedNumberMap.remove(name);
        }
    }
    else
    {
        result.append(0);
        result.append(0);
    }
    g_unsignedNumberMapMutex.unlock();
    return result;
}

///Sets a signed number in the global sequence signed number map.
void SequenceScriptThread::setGlobalSignedNumber(QString name, qint32 number)
{
    g_signedNumberMapMutex.lock();
    g_signedNumberMap[name] = number;
    g_signedNumberMapMutex.unlock();
}

///Returns a signed number from the global sequence signed number map.
///The first element is the result status (1=name found, 0=name not found)
///The second element is the read value.
QList<qint32> SequenceScriptThread::getGlobalSignedNumber(QString name, bool removeValue)
{
    QList<qint32> result;
    g_signedNumberMapMutex.lock();

    if(g_signedNumberMap.contains(name))
    {
        result.append(1);
        result.append(g_signedNumberMap[name]);

        if(removeValue)
        {
            g_signedNumberMap.remove(name);
        }
    }
    else
    {
        result.append(0);
        result.append(0);
    }
    g_signedNumberMapMutex.unlock();
    return result;
}


/**
 * Drag enter event.
 * @param event
 *      The drag enter event.
 */
void SequenceTablePlainTextEdit::dragEnterEvent(QDragEnterEvent *event)
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
void SequenceTablePlainTextEdit::dropEvent(QDropEvent *event)
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
void SequenceTablePlainTextEdit::keyPressEvent(QKeyEvent *event)
{
    if((event->modifiers() == Qt::AltModifier) && (event->text() == "\r"))
    {//alt+enter pressed

        m_tableView->sendSequence(m_row, false, this);
    }
    else
    {
        QPlainTextEdit::keyPressEvent(event);
    }
}


/**
 * This function is called if the text looses the focus
 * @param e
 *      The event.
 */
void SequenceTablePlainTextEdit::focusOutEvent(QFocusEvent *e)
{
    QPlainTextEdit::focusOutEvent(e);
    m_sendWindow->checkTextEditCell(m_row);
}

/**
 * This slot function is called if the user release a mouse button at the table view.
 * If the mouse button is the right button then the corresponding sequence from the row is send.
 * @param event
 *      The mouse event.
 */
void SequenceTableView::mouseReleaseEvent(QMouseEvent* event)
{
    QTableWidget::mouseReleaseEvent(event);

    if ( event->button() == Qt::RightButton )
    {
        if(m_sendWindow->m_userInterface->tableWidget->selectedItems().length() > 0)
        {
            sendSequence(m_sendWindow->m_userInterface->tableWidget->selectedItems().at(SendWindow::COLUMN_NAME)->row(), false, this);
        }
    }
}

/**
 * The user has pressed a key.
 *
 * @param event
 *      The key event.
 */
void SequenceTableView::keyPressEvent(QKeyEvent *event)
{
    if((event->modifiers() == Qt::AltModifier) && (event->text() == "\r"))
    {//alt+enter pressed

        sendSequence(m_sendWindow->m_userInterface->tableWidget->selectedItems().at(SendWindow::COLUMN_NAME)->row(), false, this);
    }
    else
    {
        QTableWidget::keyPressEvent(event);
    }
}

/**
 * Closes the debugger.
 */
void SequenceScriptThread::closeDebugger(void)
{

    if(m_runsInDebugger && m_debugger)
    {
        m_debugger->detach();
        m_debugWindow->close();
    }
}

/**
 * Executes a script before sending the data.
 *
 * @param sendScript
 *  The script name.
 * @param sendData
 *      The send data. (contains the modified data)
 * @param scriptEngineWrapper
 *      The script wrapper. If 0 then the script wrapper will be create and written to this argument.
 */
void SequenceScriptThread::executeScriptSlot(QString* sendScript, QByteArray* sendData, SequenceScriptEngineWrapper **scriptEngineWrapper)
{
    bool debugWindowHasBeenClosed = false;

    if(m_runsInDebugger && m_debugWindow)
    {
        if(m_debugWindow->isHidden())
        {//The debug window hahs been closed.
            debugWindowHasBeenClosed = true;
        }
    }
    if(!sendScript->isEmpty() && !debugWindowHasBeenClosed)
    {
        if(*scriptEngineWrapper == 0)
        {//The script engine has not been created yet.
            *scriptEngineWrapper = loadScript(*sendScript);
        }

        //Script has been loaded.
        if (*scriptEngineWrapper)
        {

            if((*scriptEngineWrapper)->sendDataFunction == 0)
            {
                (*scriptEngineWrapper)->sendDataFunction = new QScriptValue((*scriptEngineWrapper)->scriptEngine->evaluate("sendData"));
            }

            if (!(*scriptEngineWrapper)->sendDataFunction->isError())
            {
                QScriptValue scriptArray = (*scriptEngineWrapper)->scriptEngine->newArray(sendData->size());
                for(int i = 0; i < sendData->size(); i++)
                {
                    scriptArray.setProperty(i, QScriptValue((*scriptEngineWrapper)->scriptEngine, (unsigned char)(sendData->at(i))));
                }

                //call the sendData function
                QScriptValue val = (*scriptEngineWrapper)->sendDataFunction->call(QScriptValue(), QScriptValueList() << scriptArray);
                QList<QVariant> resultVariant = val.toVariant().toList();


                sendData->clear();
                for(auto el : resultVariant)
                {
                    sendData->append((char)el.toUInt());

                }
            }
            else
            {
                sendData->clear();
            }

            if((*scriptEngineWrapper)->scriptEngine->hasUncaughtException())
            {
                QScriptValue exception = (*scriptEngineWrapper)->scriptEngine->uncaughtException();
                m_dialogIsShown = true;
                QWidget *parent = (m_sendWindow->isVisible()) ? static_cast<QWidget *>(m_sendWindow) : static_cast<QWidget *>(m_mainWindow);
                m_scriptFileObject->showExceptionInMessageBox(exception, *sendScript, (*scriptEngineWrapper)->scriptEngine, parent, m_mainWindow->getScriptWindow());
                m_dialogIsShown = false;
            }

        }
        else
        {
            sendData->clear();
        }

    }
    else
    {
        sendData->clear();
    }

    m_scriptFunctionIsFinished = true;
}


/**
 * Creates the script thread.
 * @param isSingle
 *      True if this is a single sequence.
 * @param debug
 *      True if the script shall be executed in the script debugger.
 */
void SequenceTableView::createThread(bool isSingle, bool debug)
{
    SequenceScriptThread** thread = isSingle ? &m_scriptSingle : &m_scriptCyclic;

    *thread = new SequenceScriptThread(m_sendWindow, m_mainWindow, this, debug);


    qRegisterMetaType<QMessageBox::Icon>("QMessageBox::Icon");
    qRegisterMetaType<QMessageBox::StandardButtons>("QMessageBox::StandardButtons");
    qRegisterMetaType<QList<QVariant>*>("QList<QVariant>*");




    if(debug)
    {
        (*thread)->run();
    }
    else
    {
        (*thread)->moveToThread((*thread));


        if(isSingle)
        {
            connect(this, SIGNAL(executeScriptSingle(QString*,QByteArray*,SequenceScriptEngineWrapper**)),
                    (*thread), SLOT(executeScriptSlot(QString*,QByteArray*,SequenceScriptEngineWrapper**)), Qt::QueuedConnection);
        }
        else
        {
            connect(this, SIGNAL(executeScriptCyclic(QString*,QByteArray*,SequenceScriptEngineWrapper**)),
                    (*thread), SLOT(executeScriptSlot(QString*,QByteArray*,SequenceScriptEngineWrapper**)), Qt::QueuedConnection);
        }

        (*thread)->start(QThread::HighPriority);
    }

}

/**
 * Terminates the script thread.
 * @param isSingle
 *      True if this is a single sequence.
 */
void SequenceTableView::terminateThread(bool isSingle)
{
     SequenceScriptThread** thread = isSingle ? &m_scriptSingle : &m_scriptCyclic;

    //Disconnect all external signals.
     if(isSingle)
     {
        disconnect(this, SIGNAL(executeScriptSingle(QString*,QByteArray*,SequenceScriptEngineWrapper**)),
                (*thread), SLOT(executeScriptSlot(QString*,QByteArray*,SequenceScriptEngineWrapper**)));
     }
     else
     {
         disconnect(this, SIGNAL(executeScriptCyclic(QString*,QByteArray*,SequenceScriptEngineWrapper**)),
                 (*thread), SLOT(executeScriptSlot(QString*,QByteArray*,SequenceScriptEngineWrapper**)));
     }

    QApplication::removePostedEvents((*thread));
    (*thread)->terminate();
    (*thread) = 0;
    g_aThreadHasBeenTerminated = true;

}
/**
 * Executes a script before sending the data.
 *
 * @param sendScript
 *  The script name.
 * @param sendData
 *      The send data.
 * @param scriptEngineWrapper
 *      The script wrapper. If 0 then the script wrapper will be create and written to this argument.
 * @param isSingle
 *      True if this is a single sequence.
 * @param debug
 *      True if the script shall be executed in the script debugger.
 * @param firstCyclicSend
 *      True if this call is the first call of a cyclic sequence.
 * @return
 *      The resulting data from the script.
 */
QByteArray SequenceTableView::executeScript(QString sendScript, QByteArray sendData, SequenceScriptEngineWrapper **scriptEngineWrapper, bool isSingle,
                                            bool debug, bool firstCyclicSend)
{
    SequenceScriptThread** thread = isSingle ? &m_scriptSingle : &m_scriptCyclic;

    if(!debug)
    {
        if((*thread) == 0)
        {
            createThread(isSingle);
        }

        (*thread)->m_scriptFunctionIsFinished = false;
        QDateTime callTime = QDateTime::currentDateTime();

        if(isSingle)
        {
            emit executeScriptSingle(&sendScript, &sendData, scriptEngineWrapper);
        }
        else
        {
            emit executeScriptCyclic(&sendScript, &sendData, scriptEngineWrapper);
        }
        while(!(*thread)->m_scriptFunctionIsFinished)
        {
            QCoreApplication::processEvents();
            if((*thread)->m_dialogIsShown)
            {
                callTime = QDateTime::currentDateTime();
            }

            if(callTime.msecsTo(QDateTime::currentDateTime()) > (*thread)->m_blockTime)
            {//Thread is blocked.

                terminateThread(isSingle);
                createThread(isSingle);
                sendData.clear();
                QMessageBox::critical(this, "error", sendScript + " is blocked");
                break;
            }
        }
    }
    else
    {
        if((*thread) != 0 && firstCyclicSend)
        {
            (*thread)->exit();
        }

        if(firstCyclicSend)
        {
            createThread(isSingle, debug);
        }

        (*thread)->executeScriptSlot(&sendScript, &sendData, scriptEngineWrapper);

    }

    return (sendData.isEmpty()) ? QByteArray() : sendData;
}

/**
 * Closes the debugger.
 * @param isSingle
 *      True if this is a single sequence.
 */
void SequenceTableView::closeDebugger(bool isSingle)
{
    SequenceScriptThread** thread = isSingle ? &m_scriptSingle : &m_scriptCyclic;
    if((*thread) != 0)
    {
        if((*thread) ->getRunsInDebugger())
        {
            (*thread)->closeDebugger();

            delete (*thread);
            (*thread) = 0;
        }
    }

}

/**
 * This function sends the selected sequence.
 * @param row
 *      The row of the sequence in the sequence table.
 * @param debug
 *      True if the script shall be executed in the script debugger.
 * @param callerWidget
 *      The caller widget.
 */
void SequenceTableView::sendSequence(int row, bool debug, QWidget* callerWidget)
{

    m_sendWindow->checkTextEditCell(row);
    const Settings* settings = m_mainWindow->getSettingsDialog()->settings();

    SequenceTableComboBox* box = static_cast<SequenceTableComboBox*>(m_sendWindow->m_userInterface->tableWidget->cellWidget(row, SendWindow::COLUMN_FORMAT));
    SequenceTablePlainTextEdit* lineEdit = static_cast<SequenceTablePlainTextEdit*>(m_sendWindow->m_userInterface->tableWidget->cellWidget(row, SendWindow::COLUMN_VALUE));
    SequenceTablePlainTextEdit* scriptLineEdit = static_cast<SequenceTablePlainTextEdit*>(m_sendWindow->m_userInterface->tableWidget->cellWidget(row, SendWindow::COLUMN_SCRIPT));

    if(!lineEdit->toPlainText().isEmpty())
    {
        QByteArray sendData = m_sendWindow->textToByteArray(box->currentText(), lineEdit->toPlainText(),
                                                            m_sendWindow->formatToDecimalType(box->currentText()), settings->targetEndianess);

        if(box->currentText() == "ascii")
        {
            const Settings* settings = m_sendWindow->m_settingsDialog->settings();
            sendData.replace("\n", settings->consoleSendOnEnter.toLocal8Bit());
        }

        if(!sendData.isEmpty())
        {
            m_sendWindow->sendDataWithTheMainInterface(sendData, callerWidget, 0, 0, false, scriptLineEdit->toPlainText(), debug);
        }

    }
}

/**
 * Wrapper for showing a QMessageBox dialog
 * @param icon
 *      The icon if the message box.
 * @param title
 *      The title of the message box.
 * @param text
 *      The text of the message box.
 */
void SequenceScriptThread::messageBox(QString icon, QString title, QString text)
{
    m_dialogIsShown = true;
    QWidget *parent = (m_sendWindow->isVisible()) ? static_cast<QWidget *>(m_sendWindow) : static_cast<QWidget *>(m_mainWindow);
    m_standardDialogs->messageBox(icon, title, text, parent);
    m_dialogIsShown = false;


}

/**
 * Convenience function to get a string from the user.
 * Shows a QInputDialog::getText dialog (line edit).
 * @param title
 *      The title of the dialog.
 * @param label
 *      The label over the input section.
 * @param displayedText
 *      The display text in the input section
 * @return
 *      The text in the input section after closing the dialog (empty if the ok button was not pressed).
 */
QString SequenceScriptThread::showTextInputDialog(QString title, QString label, QString displayedText)
{
    m_dialogIsShown = true;
    QWidget *parent = (m_sendWindow->isVisible()) ? static_cast<QWidget *>(m_sendWindow) : static_cast<QWidget *>(m_mainWindow);
    QString result = m_standardDialogs->showTextInputDialog(title, label, displayedText, parent);
    m_dialogIsShown = false;
    return result;
}

/**
 * Convenience function to get a multiline string from the user.
 * Shows a QInputDialog::getMultiLineText dialog (plain text edit).
 * @param title
 *      The title of the dialog.
 * @param label
 *      The label over the input section.
 * @param displayedText
 *      The display text in the input section
 * @return
 *      The text in the input section after closing the dialog (empty if the ok button was not pressed).
 */
QString SequenceScriptThread::showMultiLineTextInputDialog(QString title, QString label, QString displayedText)
{

    QWidget *parent = (m_sendWindow->isVisible()) ? static_cast<QWidget *>(m_sendWindow) : static_cast<QWidget *>(m_mainWindow);
    m_dialogIsShown = true;
    QString result = m_standardDialogs->showMultiLineTextInputDialog(title, label, displayedText, parent);
    m_dialogIsShown = false;
    return result;
}

/**
 * Convenience function to let the user select an item from a string list.
 * Shows a QInputDialog::getItem dialog (combobox).
 * @param title
 *      The title of the dialog.
 * @param label
 *      The label over the combobox.
 * @param displayedItems
 *      The displayed items.
 * @param currentItemIndex
 *      The current combobox index.
 * @param editable
 *      True if the combobox shall be editable.
 * @return
 *      The text of the selected item after closing the dialog (empty if the ok button was not pressed).
 */
QString SequenceScriptThread::showGetItemDialog(QString title, QString label, QStringList displayedItems,
                                         int currentItemIndex, bool editable)
{
    QWidget *parent = (m_sendWindow->isVisible()) ? static_cast<QWidget *>(m_sendWindow) : static_cast<QWidget *>(m_mainWindow);
    m_dialogIsShown = true;
    QString result = m_standardDialogs->showGetItemDialog(title,label,displayedItems,currentItemIndex,editable, parent);
    m_dialogIsShown = false;
    return result;
}


/**
 * Convenience function to get an integer input from the user.
 * Shows a QInputDialog::getInt dialog (spinbox).
 * @param title
 *      The title of the dialog.
 * @param label
 *      The label over the input section.
 * @param initialValue
 *      The initial value.
 * @param min
 *      The minimum value.
 * @param max
 *      The maximum value.
 * @param step
 *      The amount by which the values change as the user presses the arrow buttons to
 *      increment or decrement the value.
 * @result
 *      item 0: 1 if the ok button has been pressed, 0 otherwise
 *      item 1: The value of the spinbox after closing the dialog.
 */
QList<int> SequenceScriptThread::showGetIntDialog(QString title, QString label, int initialValue, int min, int max, int step)
{

    m_dialogIsShown = true;
    QWidget *parent = (m_sendWindow->isVisible()) ? static_cast<QWidget *>(m_sendWindow) : static_cast<QWidget *>(m_mainWindow);
    QList<int> result = m_standardDialogs->showGetIntDialog(title,label,initialValue, min, max, step, parent);
    m_dialogIsShown = false;
    return result;
}

/**
 * Convenience function to get a floating point number from the user.
 * Shows a QInputDialog::getDouble dialog (spinbox).
 *
 * @param title
 *      The title of the dialog.
 * @param label
 *      The label over the input section.
 * @param initialValue
 *      The initial value.
 * @param min
 *      The minimum value.
 * @param max
 *      The maximum value.
 * @param decimals
 *      The maximum number of decimal places the number may have.
 * @result
 *      item 0: 1.0 if the ok button has been pressed, 0 otherwise
 *      item 1: The value of the spinbox after closing the dialog.
 */
QList<double> SequenceScriptThread::showGetDoubleDialog(QString title, QString label, double initialValue, double min, double max,
                                           int decimals)
{
    m_dialogIsShown = true;
    QWidget *parent = (m_sendWindow->isVisible()) ? static_cast<QWidget *>(m_sendWindow) : static_cast<QWidget *>(m_mainWindow);
    QList<double> result = m_standardDialogs->showGetDoubleDialog(title,label,initialValue, min, max, decimals, parent);
    m_dialogIsShown = false;
    return result;
}

/**
 * Convenience function to get color settings from the user.
 * Shows a color_widgets::ColorDialog dialog.
 * @param initInitalRed
 *      The inital value for red.
 * @param initInitalGreen
 *      The inital value for green.
 * @param initInitalBlue
 *      The inital value for blue.
 * @param initInitalAlpha
 *      The inital value for the alpha value.
 * @param alphaIsEnabled
 *      True if the color alpha channel should be editedable.
 *      If alpha is disabled, the selected color's alpha will always be 255.
 * @return
 *      The list contains following:
 *      - 1 if the user has pressed the OK button, otherwise 0
 *      - the selected red value
 *      - the selected green value
 *      - the selected blue value
 *      - the selected alpha value
 */
QList<int> SequenceScriptThread::showColorDialog(quint8 initInitalRed, quint8 initInitalGreen, quint8 initInitalBlue, quint8 initInitalAlpha, bool alphaIsEnabled)
{
    m_dialogIsShown = true;
    QWidget *parent = (m_sendWindow->isVisible()) ? static_cast<QWidget *>(m_sendWindow) : static_cast<QWidget *>(m_mainWindow);
    QList<int> result = m_standardDialogs->showColorDialog(initInitalRed, initInitalGreen, initInitalBlue, initInitalAlpha, alphaIsEnabled, parent);
    m_dialogIsShown = false;
    return result;
}


/**
 * Returns all functions and properties of an object.
 * @param object
 *      The object.
 * @return
 *      All functions and properties of the object.
 */
QStringList SequenceScriptThread::getAllObjectPropertiesAndFunctions(QScriptValue object)
{
    QStringList resultList;
    QScriptValueIterator it(object);
    while (it.hasNext())
    {
        it.next();
        resultList.append(it.name());
    }
    return resultList;
}

/**
 * This function shows a yes/no dialog.
 * @param icon
 *      The icon if the message box.
 * @param title
 *      The title of the message box.
 * @param text
 *      The text of the message box.
 * @return
 *      True if the yes button has been pressed.
 */
bool SequenceScriptThread::showYesNoDialog(QString icon, QString title, QString text)
{
    m_dialogIsShown = true;
    QWidget *parent = (m_sendWindow->isVisible()) ? static_cast<QWidget *>(m_sendWindow) : static_cast<QWidget *>(m_mainWindow);
    bool yesButtonPressed = m_standardDialogs->showYesNoDialog(icon, title, text, parent);
    m_dialogIsShown = false;
    return yesButtonPressed;
}

/**
 * Loads one script.
 * @param scriptPath
 *      The script path.
 * @param isRelativePath
 *      True of scriptPath is a relative path.
 * @return
 *      The created script wrapper on success.
 */
SequenceScriptEngineWrapper* SequenceScriptThread::loadScript(QString scriptPath)
{

    m_scriptFileObject->setScriptFileName(scriptPath);
    SequenceScriptEngineWrapper* scriptEngineWrapper= 0;
    QWidget *parent = (m_sendWindow->isVisible()) ? static_cast<QWidget *>(m_sendWindow) : static_cast<QWidget *>(m_mainWindow);

    QString unsavedInfoFile = ScriptWindow::getUnsavedInfoFileName(scriptPath);
    if(QFileInfo().exists(unsavedInfoFile))
    {//The file has unsaved changes.

        m_dialogIsShown = true;
        if(!m_standardDialogs->showYesNoDialog("Warning", "Warning", scriptPath + " is opened by an instance of ScriptEditor and contains unsaved changes. Execute anyway?",
                                               parent))
        {
            m_dialogIsShown = false;
            return scriptEngineWrapper;
        }
        m_dialogIsShown = false;
    }

    QFile scriptFile(scriptPath);
    if(!scriptFile.open(QIODevice::ReadOnly))
    {
        m_dialogIsShown = true;
        m_standardDialogs->messageBox("Critical", "error", "could not open script file: " + scriptPath,parent);
        m_dialogIsShown = false;
    }
    else
    {
        scriptEngineWrapper = new SequenceScriptEngineWrapper();
        scriptEngineWrapper->runsInDebugger = m_runsInDebugger;
        //create the script engine
        QScriptEngine* scriptEngine = new QScriptEngine();

        qRegisterMetaType<SequenceTableView*>("SequenceTableView*");
        qRegisterMetaType<QVector<unsigned char>>("QVector<unsigned char>");
        qRegisterMetaType<QList<quint32>>("QList<quint32>");
        qRegisterMetaType<QList<qint32>>("QList<qint32>");
        qRegisterMetaType<QList<int>>("QList<int>");
        qRegisterMetaType<QList<double>>("QList<double>");


        qScriptRegisterSequenceMetaType<QVector<unsigned char> >(scriptEngine);
        qScriptRegisterSequenceMetaType<QList<quint32> >(scriptEngine);
        qScriptRegisterSequenceMetaType<QList<qint32> >(scriptEngine);
        qScriptRegisterSequenceMetaType<QList<int> >(scriptEngine);
        qScriptRegisterSequenceMetaType<QList<double> >(scriptEngine);

        //register the script thread object
        scriptEngine->globalObject().setProperty("seq", scriptEngine->newQObject(this));

        m_converterObject.registerScriptMetaTypes(scriptEngine);

        if(m_runsInDebugger)
        {
            m_debugger = new QScriptEngineDebugger(this);
            m_debugWindow = m_debugger->standardWindow();
            m_debugWindow->setWindowModality(Qt::NonModal);
            m_debugWindow->resize(1280, 704);
            m_debugger->attachTo(scriptEngine);
            m_debugger->action(QScriptEngineDebugger::InterruptAction)->trigger();
            m_debugWindow->setWindowTitle(scriptFile.fileName());
            connect(m_mainWindow, SIGNAL(bringWindowsToFrontSignal()), this, SLOT(bringWindowsToFrontSlot()), Qt::DirectConnection);
        }

        //set ScriptContext
        QScriptContext *context = scriptEngine->currentContext();
        QScriptContext *parent=context->parentContext();
        if(parent!=0)
        {
            context->setActivationObject(context->parentContext()->activationObject());
            context->setThisObject(context->parentContext()->thisObject());
        }

        QScriptValue result = scriptEngine->evaluate(scriptFile.readAll(), scriptPath);
        scriptFile.close();

        scriptEngineWrapper->scriptEngine = scriptEngine;

        if (result.isError())
        {
            m_dialogIsShown = true;
            QWidget *parent = (m_sendWindow->isVisible()) ? static_cast<QWidget *>(m_sendWindow) : static_cast<QWidget *>(m_mainWindow);
            m_scriptFileObject->showExceptionInMessageBox(result, scriptPath, scriptEngineWrapper->scriptEngine, parent, m_mainWindow->getScriptWindow());
            m_dialogIsShown = false;
            delete scriptEngineWrapper;
            scriptEngineWrapper = 0;
        }
    }
    return scriptEngineWrapper;
}

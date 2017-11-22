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

#ifndef SEQUENCETABLEVIEW_H
#define SEQUENCETABLEVIEW_H

#include <QMainWindow>
#include <QWidget>
#include <QThread>
#include <QMutex>
#include <QTableWidget>
#include<QFile>
#include "QScriptEngine"
#include "QPlainTextEdit"
#include "QComboBox"
#include <QTimer>
#include <QSplitter>
#include <mainwindow.h>
#include <QScriptEngineDebugger>
#include <scriptHelper.h>
#include "scriptFile.h"
#include "crc.h"
#include "settingsdialog.h"
#include "scriptStandardDialogs.h"
#include "scriptObject.h"
#include "scriptwindow.h"
#include "scriptConverter.h"

class SendThread;
class SendWindow;
class SequenceTableView;

///Scriptengine wrapper for calling SequenceTableView::executeScript
class SequenceScriptEngineWrapper : public QObject
{
    Q_OBJECT

public:

    SequenceScriptEngineWrapper() : QObject(0), scriptEngine(0), sendDataFunction(0), runsInDebugger(false){}
    virtual ~SequenceScriptEngineWrapper()
    {

        if(scriptEngine)
        {
            delete scriptEngine;
        }
        if(sendDataFunction)
        {
            delete sendDataFunction;
        }

    }

    ///Pointer to the script engine.
    QScriptEngine* scriptEngine;

    ///Pointer to the send data script function.
    QScriptValue* sendDataFunction;

    ///True if the scripts runs in a script debugger.
    bool runsInDebugger;

};

class SequenceScriptThread : public QThread, public ScriptObject
{
    Q_OBJECT
    friend class SequenceTableView;

    ///Returns a semicolon separated list with all public functions, signals and properties.
    Q_PROPERTY(QString publicScriptElements READ getPublicScriptElements)

public:
    SequenceScriptThread( SendWindow* sendWindow, MainWindow* mainWindow,  SequenceTableView* sequencteTable, bool runsInDebugger) : QThread(0), m_sendWindow(sendWindow),
    m_mainWindow(mainWindow), m_sequencteTable(sequencteTable), m_blockTime(DEFAULT_BLOCK_TIME), m_dialogIsShown(false),
    m_scriptFunctionIsFinished(true), m_standardDialogs(0), m_runsInDebugger(runsInDebugger), m_debugger(0), m_debugWindow(0){}
    virtual ~SequenceScriptThread(){}

    ///Returns a semicolon separated list with all public functions, signals and properties.
    virtual QString getPublicScriptElements(void)
    {
        return MainWindow::parseApiFile("seq.api");
    }

    ///Appends text to the script window console.
    Q_INVOKABLE void appendTextToConsole(QString string, bool newLine=true, bool bringToForeground=false){ emit appendTextToConsoleSignal(string, newLine,bringToForeground);}

    /****************Deprecated functions (replaced by the conv object)******************************************************/
    ///Converts a byte array which contains ascii characters into a ascii string (QString).
    Q_INVOKABLE QString byteArrayToString(QVector<unsigned char> data){return ScriptConverter::byteArrayToString(data);}

    ///Converts a byte array into a hex string.
    Q_INVOKABLE QString byteArrayToHexString(QVector<unsigned char> data){return ScriptConverter::byteArrayToHexString(data);}

    ///Converts an ascii string into a byte array.
    Q_INVOKABLE QVector<unsigned char> stringToArray(QString str){return ScriptConverter::stringToArray(str);}

    ///Adds an ascii string to a byte array.
    Q_INVOKABLE QVector<unsigned char> addStringToArray(QVector<unsigned char> array, QString str){return ScriptConverter::addStringToArray(array, str);}
    /*************************************************************************************************************************/

    ///Calculates a crc8.
    Q_INVOKABLE static quint8 calculateCrc8(const QVector<unsigned char> data){return CRC::calculateCrc8(data);}

    ///Calculates a crc8 with a given polynomial.
    Q_INVOKABLE static quint8 calculateCrc8WithPolynomial(const QVector<unsigned char> data, const unsigned char polynomial, const unsigned char startValue=0)
                {return CRC::calculateCrc8(data, polynomial,startValue);}

    ///Calculates a crc16.
    Q_INVOKABLE static quint16 calculateCrc16(const QVector<unsigned char> data){return CRC::calculateCrc16(data);}

    ///Calculates a crc32.
    Q_INVOKABLE static quint32 calculateCrc32(const QVector<unsigned char> data){return CRC::calculateCrc32(data);}

    ///Calculates a crc64.
    Q_INVOKABLE static quint64 calculateCrc64(const QVector<unsigned char> data){return CRC::calculateCrc64(data);}

    ///Sets a string in the global sequence string map.
    Q_INVOKABLE void setGlobalString(QString name, QString string);

    ///Returns a string from the global sequence string map.
    ///Note: Returns an empty string if name is not in the map.
    Q_INVOKABLE QString getGlobalString(QString name, bool removeValue=false);

    ///Sets a data vector in the global sequence data vector map.
    Q_INVOKABLE void setGlobalDataArray(QString name, QVector<unsigned char> data);

    ///Returns a data vector from the global sequence data vector map.
    ///Note: Returns an empty data vector if name is not in the map.
    Q_INVOKABLE QVector<unsigned char> getGlobalDataArray(QString name, bool removeValue=false);

    ///Sets a unsigned number in the global sequence unsigned number map.
    Q_INVOKABLE void setGlobalUnsignedNumber(QString name, quint32 number);

    ///Returns a unsigned number from the global sequence unsigned number map.
    ///The first element is the result status (1=name found, 0=name not found)
    ///The second element is the read value.
    Q_INVOKABLE QList<quint32> getGlobalUnsignedNumber(QString name,bool removeValue=false);

    ///Sets a signed number in the global sequence signed number map.
    Q_INVOKABLE void setGlobalSignedNumber(QString name, qint32 number);

    ///Returns a signed number from the global sequence signed number map.
    ///The first element is the result status (1=name found, 0=name not found)
    ///The second element is the read value.
    Q_INVOKABLE QList<qint32> getGlobalSignedNumber(QString name,bool removeValue=false);

    ///Returns the current version of ScriptCommunicator.
    Q_INVOKABLE QString getCurrentVersion(void){return MainWindow::VERSION;}


    ///Wrapper for showing a QMessageBox dialog.
    Q_INVOKABLE void messageBox(QString icon, QString title, QString text);

    ///Shows a yes/no dialog.
    Q_INVOKABLE bool showYesNoDialog(QString icon, QString title, QString text);

    ///Convenience function to get a string from the user.
    ///Shows a QInputDialog::getText dialog (line edit).
    Q_INVOKABLE QString showTextInputDialog(QString title, QString label, QString displayedText="");

    ///Convenience function to get a multiline string from the user.
    ///Shows a QInputDialog::getMultiLineText dialog (plain text edit).
    Q_INVOKABLE QString showMultiLineTextInputDialog(QString title, QString label, QString displayedText="");

    ///Convenience function to let the user select an item from a string list.
    ///Shows a QInputDialog::getItem dialog (combobox).
    Q_INVOKABLE QString showGetItemDialog(QString title, QString label, QStringList displayedItems,
                               int currentItemIndex=0, bool editable=false);

    ///Convenience function to get an integer input from the user.
    ///Shows a QInputDialog::getInt dialog (spinbox).
    Q_INVOKABLE QList<int> showGetIntDialog(QString title, QString label, int initialValue, int min, int max, int step);

    ///Convenience function to get a floating point number from the user.
    ///Shows a QInputDialog::getDouble dialog (spinbox).
    Q_INVOKABLE QList<double> showGetDoubleDialog(QString title, QString label, double initialValue, double min, double max, int decimals);

    ///Convenience function to get color settings from the user.
    Q_INVOKABLE QList<int> showColorDialog(quint8 initInitalRed=255, quint8 initInitalGreen=255, quint8 initInitalBlue=255, quint8 initInitalAlpha=255, bool alphaIsEnabled=false);

    ///Sets the script block time.
    ///Note: After this execution time (sendData and the script main function (all outside a function))
    ///the script is regarded as blocked and will be stopped.
    Q_INVOKABLE void setBlockTime(quint32 blockTime){m_blockTime = blockTime;}

    ///Returns all functions and properties of an object.
    Q_INVOKABLE QStringList getAllObjectPropertiesAndFunctions(QScriptValue object);

    ///The default value for m_blockTime.
    static const quint32 DEFAULT_BLOCK_TIME= 10000;

    ///Returns m_runsInDebugger.
    bool getRunsInDebugger(void){return m_runsInDebugger;}

    ///Closes the debugger.
    void closeDebugger(void);

signals:

    ///Is connected with ScriptWindow::appendTextToConsoleSlot (appends text to the console in the script window).
    ///This signal must not be used from script.
    void appendTextToConsoleSignal(QString text, bool newLine, bool bringToForeground);

public slots:
    ///Executes a script before sending the data.
    void executeScriptSlot(QString* sendScript, QByteArray* sendData, SequenceScriptEngineWrapper** scriptEngineWrapper);

    ///Brings the debug window to foreground.
    ///Note: This is an internal function and must not be used by a script.
    void bringWindowsToFrontSlot(void)
    {
        if(m_debugWindow && m_debugWindow->isVisible())
        {
            m_debugWindow->setWindowState( (m_debugWindow->windowState() & ~Qt::WindowMinimized) | Qt::WindowActive);
            m_debugWindow->raise();  // for MacOS
            m_debugWindow->activateWindow(); // for Windows
        }
    }

protected:
    ///The thread main function.
    void run()
    {


        m_standardDialogs = new ScriptStandardDialogs(this);
        m_standardDialogs->intSignals(m_mainWindow->getScriptWindow(), m_runsInDebugger);

        m_scriptFileObject = new ScriptFile(this, "", false);
        m_scriptFileObject->intSignals(m_mainWindow->getScriptWindow(), m_runsInDebugger);

        Qt::ConnectionType directConnectionType = m_runsInDebugger ? Qt::DirectConnection : Qt::BlockingQueuedConnection;
        QObject::connect(this, SIGNAL(appendTextToConsoleSignal(QString, bool,bool)),
                m_mainWindow->getScriptWindow(), SLOT(appendTextToConsoleSlot(QString, bool,bool)), directConnectionType);

        if(!m_runsInDebugger)
        {
            exec();
        }
    }

private:

    ///Loads one script.
    SequenceScriptEngineWrapper *loadScript(QString scriptPath);

    ///Pointer to the main window.
    SendWindow* m_sendWindow;

    ///Pointer to the main window.
    MainWindow* m_mainWindow;

    ///pointer to the sequence table.
    SequenceTableView* m_sequencteTable;

    ///After this execution time (SequenceTableView::executeScript) the script thread is regarded as blocked.
    quint32 m_blockTime;

    ///True if the script shows a dialog.
    bool m_dialogIsShown;

    ///True if the script function is finished.
    bool m_scriptFunctionIsFinished;

    ///The script standard dialogs.
    ScriptStandardDialogs* m_standardDialogs;

    ///True if the scripts runs in a script debugger.
    bool m_runsInDebugger;

    ///The script debugger;
    QScriptEngineDebugger *m_debugger ;

    ///The debug window.
    QMainWindow *m_debugWindow;

    ///The script file object.
    ScriptFile* m_scriptFileObject;

    ///The script converter object.
    ScriptConverter m_converterObject;

};
///Table view class which holds the sequences in the send window.
class SequenceTableView : public QTableWidget
{
    Q_OBJECT
    friend class SequenceScriptThread;

public:
    SequenceTableView(QWidget * parent = 0) : QTableWidget(parent), m_sendWindow(0), m_mainWindow(0), m_scriptSingle(0), m_scriptCyclic(0),
        m_scriptIsBlocked(false)
    {

    }

    ///Sets the pointer to the send window.
    void setSendWindow(SendWindow* window){m_sendWindow = window;}

    ///Sets the pointer to the main window.
    void setMainWindow(MainWindow* window){m_mainWindow = window;}

    ///This function sends the selected sequence.
    void sendSequence(int row, bool debug, QWidget* callerWidget);

    ///Executes a script before sending the data.
    QByteArray executeScript(QString sendScript, QByteArray sendData, SequenceScriptEngineWrapper** scriptEngineWrapper,
                             bool isSingle, bool debug=false, bool firstCyclicSend=false);

    ///Closes the debugger.
    void closeDebugger(bool isSingle);

protected:

    ///This slot function is called if the user release a mouse button at the table view.
    ///If the mouse button is the right button then the corresponding sequence from the row is send.
    void mouseReleaseEvent(QMouseEvent* event);

    ///The user has pressed a key.
    void keyPressEvent(QKeyEvent *event);

signals:
    void executeScriptSingle(QString* sendScript, QByteArray* sendData, SequenceScriptEngineWrapper** scriptEngineWrapper);
    void executeScriptCyclic(QString* sendScript, QByteArray* sendData, SequenceScriptEngineWrapper** scriptEngineWrapper);

private:

    ///Creates the script thread.
    void createThread(bool isSingle, bool debug=false);

    ///Terminates the script thread.
    void terminateThread(bool isSingle);

    ///Pointer to the main window.
    SendWindow* m_sendWindow;

    ///Pointer to the main window.
    MainWindow* m_mainWindow;

    ///The single sequence script thread.
    SequenceScriptThread* m_scriptSingle;

    ///The single sequence script thread.
    SequenceScriptThread* m_scriptCyclic;

    ///True if the send script is blocked.
    bool m_scriptIsBlocked;

};

#endif // SEQUENCETABLEVIEW_H

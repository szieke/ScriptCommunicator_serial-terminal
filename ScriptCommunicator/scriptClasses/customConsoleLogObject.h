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

#ifndef CUSTOMCONSOLELOGOBJECT_H
#define CUSTOMCONSOLELOGOBJECT_H

#include <QObject>
#include <QTimer>
#include <QFileInfo>
#include <QScriptEngine>
#include <QVector>
#include <mainwindow.h>
#include "scriptsqldatabase.h"
#include "scriptFile.h"
#include "scriptXml.h"
#include <QScriptEngineDebugger>
#include <scriptHelper.h>
#include "scriptObject.h"
#include "scriptwindow.h"
#include "scriptConverter.h"


class MainWindow;
class CustomConsoleLogObject;

///Thrread which executes the custom console/log script.
class CustomConsoleLogThread : public QThread, public ScriptObject
{
    Q_OBJECT
    friend class CustomConsoleLogObject;

    ///Returns a semicolon separated list with all public functions, signals and properties.
    Q_PROPERTY(QString publicScriptElements READ getPublicScriptElements)


public:
    CustomConsoleLogThread(CustomConsoleLogObject* consoleLogObject, MainWindow* mainWindow,
                           QString scriptPath, bool runsInDebugger) : QThread(0), m_createString(0),
    m_consoleLogObject(consoleLogObject), m_scriptEngine(0), m_scriptSql(0), m_scriptPath(scriptPath),
    m_blockTime(DEFAULT_BLOCK_TIME), m_scriptFileObject(0), m_mainWindow(mainWindow), m_runsInDebugger(runsInDebugger),
    m_debugger(0), m_debugWindow(0), m_isSuspendedByDebuger(false)
    {

    }
    virtual ~CustomConsoleLogThread()
    {
        try
        {
            if(m_scriptEngine != 0)
            {
                m_scriptEngine->deleteLater();
                m_scriptEngine = 0;
            }

            delete m_createString;
        }
        catch(...)
        {

        }
    }
    ///Returns a semicolon separated list with all public functions, signals and properties.
    virtual QString getPublicScriptElements(void)
    {
        return MainWindow::parseApiFile("cust.api");
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

    /****************Deprecated functions (replaced by the scriptFile object)******************************************************/

    ///Reads a text file and returns the content.
    Q_INVOKABLE QString readFile(QString path, bool isRelativePath=true, quint64 startPosition=0, qint64 numberOfBytes=-1)
    {return m_scriptFileObject->readFile(path, isRelativePath, startPosition, numberOfBytes);}

    ///Reads a binary file and returns the content.
    Q_INVOKABLE QVector<unsigned char> readBinaryFile(QString path, bool isRelativePath=true, quint64 startPosition=0, qint64 numberOfBytes=-1)
    {return m_scriptFileObject->readBinaryFile(path, isRelativePath, startPosition, numberOfBytes);}

    ///Returns the size of a file.
    Q_INVOKABLE qint64 getFileSize(QString path, bool isRelativePath=true)
    {return m_scriptFileObject->getFileSize(path, isRelativePath);}

    ///Checks if a file exists.
    Q_INVOKABLE bool checkFileExists(QString path, bool isRelativePath=true)
    {return m_scriptFileObject->checkFileExists(path, isRelativePath);}

    ///Checks if a directory exists.
    Q_INVOKABLE bool checkDirectoryExists(QString directory, bool isRelativePath=true)
    {return m_scriptFileObject->checkDirectoryExists(directory, isRelativePath);}

    ///Creates a directory.
    Q_INVOKABLE bool createDirectory(QString directory, bool isRelativePath=true)
    {return m_scriptFileObject->createDirectory(directory, isRelativePath);}

    ///Renames a directory.
    Q_INVOKABLE bool renameDirectory(QString directory, QString newName)
    {return m_scriptFileObject->renameDirectory(directory, newName);}

    ///Renames a file.
    Q_INVOKABLE bool renameFile(QString path, QString newName)
    {return m_scriptFileObject->renameFile(path, newName);}

    ///Deletes a file.
    Q_INVOKABLE bool deleteFile(QString path, bool isRelativePath=true)
    {return m_scriptFileObject->deleteFile(path, isRelativePath);}

    ///Deletes a directory (must be empty).
    Q_INVOKABLE bool deleteDirectory(QString directory, bool isRelativePath=true)
    {return m_scriptFileObject->deleteDirectory(directory, isRelativePath);}

    ///Removes the directory, including all its contents.
    ///If a file or directory cannot be removed, deleteDirectoryRecursively() keeps going and attempts
    ///to delete as many files and sub-directories as possible, then returns false.
    ///If the directory was already removed, the method returns true (expected result already reached).
    Q_INVOKABLE bool deleteDirectoryRecursively(QString directory, bool isRelativePath=true)
    {return m_scriptFileObject->deleteDirectoryRecursively(directory, isRelativePath);}

    ///Reads the content of a directory and his sub directories.
    Q_INVOKABLE QStringList readDirectory(QString directory, bool isRelativePath=true, bool recursive=true, bool returnFiles=true, bool returnDirectories=true)
    {return m_scriptFileObject->readDirectory(directory, isRelativePath, recursive, returnFiles, returnDirectories);}

    ///Writes a text file (if replaceFile is true, the existing file is overwritten, else the content is appended).
    Q_INVOKABLE bool writeFile(QString path, bool isRelativePath, QString content, bool replaceFile, qint64 startPosition=-1)
    {return m_scriptFileObject->writeFile(path, isRelativePath, content, replaceFile, startPosition);}

    ///Writes a binary file (if replaceFile is true, the existing file is overwritten, else the content is appended).
    Q_INVOKABLE bool writeBinaryFile(QString path, bool isRelativePath, QVector<unsigned char> content, bool replaceFile, qint64 startPosition=-1)
    {return m_scriptFileObject->writeBinaryFile(path, isRelativePath, content, replaceFile, startPosition);}

    ///Converts a relative path into an absolute path.
    Q_INVOKABLE QString createAbsolutePath(QString fileName){return m_scriptFileObject->createAbsolutePath(fileName);}

    ///Returns the folder in which the script resides.
    Q_INVOKABLE QString getScriptFolder(void)
    {
        QFileInfo fi(m_scriptPath);
        return fi.absolutePath();
    }


    /*************************************************************************************************************************/

    ///Loads/includes one script (QtScript has no built in include mechanism).
    Q_INVOKABLE bool loadScript(QString scriptPath, bool isRelativePath=true)
    {
        scriptPath = isRelativePath ? createAbsolutePath(scriptPath) : scriptPath;
        QString unsavedInfoFile = ScriptWindow::getUnsavedInfoFileName(scriptPath);
        if(QFileInfo().exists(unsavedInfoFile))
        {//The file has unsaved changes.

            emit showMessageBoxSignal(QMessageBox::Critical, "Warning", scriptPath + " is opened by an instance of ScriptEditor and contains unsaved changes.", QMessageBox::Ok);
        }
        return m_scriptFileObject->loadScript(scriptPath, false, m_scriptEngine, m_mainWindow, m_mainWindow->getScriptWindow(), false, NULL);
    }



    ///Returns the current version of ScriptCommunicator.
    Q_INVOKABLE QString getCurrentVersion(void){return MainWindow::VERSION;}

    ///Sets the script block time.
    ///Note: After this execution time (createString and the script main function (all outside a function))
    ///the script is regarded as blocked and will be terminated.
    Q_INVOKABLE void setBlockTime(quint32 blockTime){m_blockTime = blockTime;}

    ///Creates a XML reader.
    Q_INVOKABLE QScriptValue createXmlReader();

    ///Creates a XML writer.
    Q_INVOKABLE QScriptValue createXmlWriter();

    ///Returns all functions and properties of an object.
    Q_INVOKABLE QStringList getAllObjectPropertiesAndFunctions(QScriptValue object);

    ///The default value for m_blockTime.
    static const quint32 DEFAULT_BLOCK_TIME= 10000;

    ///Sets the script file path.
    void setScriptPath(QString scriptPath)
    {
        m_scriptPath = scriptPath;
        m_scriptFileObject->setScriptFileName(m_scriptPath);
    }

    ///Returns m_runsInDebugger.
    bool getRunsInDebugger(void){return m_runsInDebugger;}

    ///Returns true if the script runs in the script debugger and the debug window has been closed.
    bool getRunsInDebuggerAndDebugWindowIsClosed()
    {
        bool result = false;

        if(m_runsInDebugger && m_debugWindow)
        {
            if(!m_debugWindow->isVisible())
            {
                result = true;
            }
        }


        return result;
    }

    ///Closes the debugger.
    void closeDebugger(void);

signals:
    ///The main interface thread emits this signal to show a QMessageBox dialog in the main window.
    ///This signal must not be used from script.
    void showMessageBoxSignal(QMessageBox::Icon icon, QString title, QString text, QMessageBox::StandardButtons buttons);

    ///Is connected with ScriptWindow::appendTextToConsoleSlot (appends text to the console in the script window).
    ///This signal must not be used from script.
    void appendTextToConsoleSignal(QString text, bool newLine, bool bringToForeground);

public slots:

     ///Executes the script function 'createString'.
    void executeScriptSlot(QByteArray* data, QString* timeStamp,
                       bool isSend, bool isUserMessage, bool isFromCan, bool isLog, QString* result, bool *errorOccured);

    ///Loads a custom console/log script.
    void loadCustomScriptSlot(QString scriptPath, bool* hasSucceeded);

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

    ///The script is suspended by the debugger.
    void suspendedByDebuggerSlot();

    ///The script is resumed by the debugger.
    void resumedByDebuggerSlot();

#ifdef Q_OS_MAC
    ///Debug timer slot (checks if the script is suspended by the debugger or is running).
    void debugTimerSlot(void);
#endif

protected:
    ///The thread main function.
    void run()
    {
        m_createString = new QScriptValue(0);
        m_scriptSql = new ScriptSql();
        m_scriptFileObject = new ScriptFile(this, m_scriptPath);
        m_scriptFileObject->intSignals(m_mainWindow->getScriptWindow(), m_runsInDebugger, false);

        connect(this, SIGNAL(appendTextToConsoleSignal(QString, bool,bool)),
                        m_mainWindow->getScriptWindow(), SLOT(appendTextToConsoleSlot(QString, bool,bool)), Qt::QueuedConnection);

        if(!m_runsInDebugger)
        {
            exec();
        }
    }

private:
    ///The create string script function.
    QScriptValue* m_createString;

    ///The custum console/log object.
    CustomConsoleLogObject* m_consoleLogObject;

    ///The script engine.
    QScriptEngine* m_scriptEngine;

    ///The script sql object.
    ScriptSql* m_scriptSql;

    ///The script path of the script to which this object belongs to.
    QString m_scriptPath;

    ///After this execution time (CustomConsoleLogThread::executeScriptSlot) the script thread is regarded as blocked.
    quint32 m_blockTime;

    ///The script file object.
    ScriptFile* m_scriptFileObject;

    ///Pointer to the main window.
    MainWindow* m_mainWindow;

    ///True if the scripts runs in a script debugger.
    bool m_runsInDebugger;

    ///The script debugger;
    QScriptEngineDebugger *m_debugger ;

    ///The debug window.
    QMainWindow *m_debugWindow;

    ///True if the script is suspended by the debugger.
    bool m_isSuspendedByDebuger;

    ///The script converter object.
    ScriptConverter m_converterObject;

#ifdef Q_OS_MAC
    ///The debug timer (checks if the script is suspended by the debugger or is running).
    QTimer m_debugTimer;
#endif

};

///Custom console and log object (the corresponding scripts are executed here).
class CustomConsoleLogObject : public QObject
{
        Q_OBJECT
    friend class CustomConsoleLogThread;

public:
    CustomConsoleLogObject(MainWindow* mainWindow);
    ~CustomConsoleLogObject();


    ///Loads a custom console/log script.
    bool loadCustomScript(QString scriptPath, bool debug);

    ///Unloads the current script.
    void unloadCustomScript(void);

    ///Calls the script function 'createString' in the script thread.
    QString callScriptFunction(QByteArray *data, QString& timeStamp, bool isSend, bool isUserMessage, bool isFromCan, bool isLog, bool *errorOccured);

    ///Returns true if a script has been loaded successfully.
    bool scriptHasBeenLoaded();

    ///Returns true if ScriptCommunicator is blocked.
    bool scriptIsBlocked(){return m_scriptIsBlocked;}

    ///Returns true if the script runs in the script debugger and the debug window has been closed.
    bool getRunsInDebuggerAndDebugWindowIsClosed()
    {
        bool result = false;
        if(m_script)
        {
            result= m_script->getRunsInDebuggerAndDebugWindowIsClosed();
        }
        return result;
    }

signals:

    ///Signal for executing the script function 'createString'.
    void executeScriptSignal(QByteArray* data, QString* timeStamp,
                           bool isSend, bool isUserMessage, bool isFromCan, bool isLog,
                           QString* result, bool* errorOccured);

    ///Signal for loading a custom console/log script.
    void loadCustomScriptSignal(QString scriptPath, bool* hasSucceeded);

private:

    ///Creates the script thread.
    void createThread(bool debug);

    ///Terminates the script thread.
    void terminateThread();

    ///The script path of the script to which this object belongs to.
    QString m_scriptPath;

    ///Pointer to the main window.
    MainWindow* m_mainWindow;

    ///The script which executed the create string function.
    CustomConsoleLogThread* m_script;

    ///True if the script function is finished.
    bool m_scriptFunctionIsFinished;

    ///True if ScriptCommunicator is blocked.
    bool m_scriptIsBlocked;
};

#endif // CUSTOMCONSOLELOGOBJECT_H

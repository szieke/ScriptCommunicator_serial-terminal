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

#include "customConsoleLogObject.h"
#include <QMessageBox>
#include <QTextStream>
#include "mainwindow.h"
#include "scriptwindow.h"
#include <QCoreApplication>
#include <QApplication>
#include <QAction>


///Is set to true if a thread has been terminated.
///This variabke is used un the main function.
extern bool g_aThreadHasBeenTerminated;

/**
 * Constructor.
 * @param mainWindow
 *      Pointer to the main window.
 */
CustomConsoleLogObject::CustomConsoleLogObject(MainWindow* mainWindow) : QObject(mainWindow),
    m_scriptPath(), m_mainWindow(mainWindow), m_script(0), m_scriptFunctionIsFinished(false), m_scriptIsBlocked(false)
{
}

/**
 * Destructor.
 */
CustomConsoleLogObject::~CustomConsoleLogObject()
{

    try
    {
        if(m_script)
        {
            m_script->exit();
            QThread::msleep(10);

            m_script->deleteLater();
        }
    }
    catch(...)
    {

    }

}
/**
 * Creates the script thread.
 * @param debug
 *      True if the script shall be executed in the script debugger.
 */
void CustomConsoleLogObject::createThread(bool debug)
{

    m_script = new CustomConsoleLogThread(this, m_mainWindow, m_scriptPath, debug);

    qRegisterMetaType<QMessageBox::Icon>("QMessageBox::Icon");
    qRegisterMetaType<QMessageBox::StandardButtons>("QMessageBox::StandardButtons");


    if(!debug)
    {
        m_script->moveToThread(m_script);

        connect(m_script, SIGNAL(showMessageBoxSignal(QMessageBox::Icon,QString,QString,QMessageBox::StandardButtons)),
                m_mainWindow, SLOT(showMessageBoxSlot(QMessageBox::Icon,QString,QString,QMessageBox::StandardButtons)),
                Qt::QueuedConnection);

        connect(this, SIGNAL(executeScriptSignal(QByteArray*,QString*,bool,bool,bool,bool,QString*,bool*)),
                m_script, SLOT(executeScriptSlot(QByteArray*,QString*,bool,bool,bool,bool,QString*,bool*)), Qt::QueuedConnection);

        connect(this, SIGNAL(loadCustomScriptSignal(QString,bool*)),
                m_script, SLOT(loadCustomScriptSlot(QString,bool*)), Qt::QueuedConnection);


        m_script->start(QThread::HighPriority);

        //Give the thread time to run.
        QThread::msleep(10);
    }
    else
    {
        connect(m_script, SIGNAL(showMessageBoxSignal(QMessageBox::Icon,QString,QString,QMessageBox::StandardButtons)),
                m_mainWindow, SLOT(showMessageBoxSlot(QMessageBox::Icon,QString,QString,QMessageBox::StandardButtons)),
                Qt::QueuedConnection);
        m_script->run();
    }

}

/**
 * Terminates the script thread.
 */
void CustomConsoleLogObject::terminateThread()
{
    //Disconnect all external signals.
    disconnect(this, SIGNAL(executeScriptSignal(QByteArray*,QString*,bool,bool,bool,bool,QString*,bool*)),
               m_script, SLOT(executeScriptSlot(QByteArray*,QString*,bool,bool,bool,bool,QString*,bool*)));

    disconnect(this, SIGNAL(loadCustomScriptSignal(QString,bool*)),
               m_script, SLOT(loadCustomScriptSlot(QString,bool*)));


    QApplication::removePostedEvents(m_script);
    m_script->terminate();
    m_script = 0;
    g_aThreadHasBeenTerminated = true;

}

/**
 * Returns true if a script has been loaded successfully.
 */
bool CustomConsoleLogObject::scriptHasBeenLoaded()
{
    bool result = false;
    if(m_script)
    {
        result = (m_script->m_scriptEngine == 0) ? false : true;
    }
    return result;

}

/**
 * Unloads the current script.
 */
void CustomConsoleLogObject::unloadCustomScript(void)
{
    if(m_script)
    {
        if(m_script->getRunsInDebugger())
        {
            m_script->closeDebugger();

        }
        else
        {
            m_script->exit();
            quint32 counter = 0;

            do
            {
                counter++;
                if(counter > 2000)
                {
                    break;
                }
                QThread::msleep(1);
                QCoreApplication::processEvents();


            }while(m_script->isRunning());
        }

        m_script->deleteLater();
        m_script = 0;
    }
}

/**
 * Loads a custom console/log script.
 * @param scriptPath
 *      The script.
 * @param debug
 *      True if the script shall be executed in the script debugger.
 * @return
 *      True on success
 */
bool CustomConsoleLogObject::loadCustomScript(QString scriptPath, bool debug)
{

    bool hasSucceeded = false;
    m_scriptIsBlocked = false;

    m_scriptPath = scriptPath;
    if(!m_script)
    {
        createThread(debug);
    }
    m_script->setScriptPath(m_scriptPath);


    if(!debug)
    {
        m_scriptFunctionIsFinished = false;
        QDateTime savedTime = QDateTime::currentDateTime();

        emit loadCustomScriptSignal(scriptPath, &hasSucceeded);
        while(!m_scriptFunctionIsFinished)
        {
            QThread::usleep(1);

            if(savedTime.msecsTo(QDateTime::currentDateTime()) > m_script->m_blockTime)
            {//Thread is blocked.
                QMessageBox::critical(m_mainWindow, "error",scriptPath + " is blocked.");
                QApplication::removePostedEvents(m_script);
                m_scriptIsBlocked = true;
                terminateThread();
                createThread(debug);
                break;
            }
        }
    }
    else
    {
        m_script->loadCustomScriptSlot(scriptPath, &hasSucceeded);

        if(!hasSucceeded)
        {
            m_script->closeDebugger();
        }
    }

    return hasSucceeded;
}

/**
 * The script is suspended by the debugger.
 */
void CustomConsoleLogThread::suspendedByDebuggerSlot()
{
    m_isSuspendedByDebuger = true;
}

/**
 * The script is resumed by the debugger.
 */
void CustomConsoleLogThread::resumedByDebuggerSlot()
{
    m_isSuspendedByDebuger = false;
}

#ifdef Q_OS_MAC
/**
* Debug timer slot (checks if the script is suspended by the debugger or is running).
*/
void CustomConsoleLogThread::debugTimerSlot(void)
{
    static QScriptEngineDebugger::DebuggerState state = QScriptEngineDebugger::SuspendedState;

    if(m_debugger->state() != state)
    {
        state = m_debugger->state();
        if(state == QScriptEngineDebugger::RunningState)
        {//The script is suspended (seems to be a bug on Mac OS X).

            m_isSuspendedByDebuger = true;
        }
        else
        {
            m_isSuspendedByDebuger = false;
        }
    }
}
#endif

/**
 * Loads a custom console/log script.
 * @param scriptPath
 *      The script.
 * @param hasSucceeded
 *      Is set to tru on success.
 */
void CustomConsoleLogThread::loadCustomScriptSlot(QString scriptPath, bool* hasSucceeded)
{
    if(!scriptPath.isEmpty())
    {

        QString unsavedInfoFile = ScriptWindow::getUnsavedInfoFileName(scriptPath);
        if(QFileInfo().exists(unsavedInfoFile))
        {//The file has unsaved changes.

            emit showMessageBoxSignal(QMessageBox::Critical, "Warning", scriptPath + " is opened by an instance of ScriptEditor and contains unsaved changes.", QMessageBox::Ok);
        }

        QFile scriptFile(scriptPath);
        if(!scriptFile.open(QIODevice::ReadOnly))
        {
            emit showMessageBoxSignal(QMessageBox::Critical, "Error", "could not open script file: " + scriptPath, QMessageBox::Ok);
        }
        else
        {
            m_scriptPath = scriptPath;
            if(m_scriptEngine != 0)
            {
                delete m_scriptEngine;
                m_scriptEngine = 0;
            }
            //Create the script engine.
            m_scriptEngine = new QScriptEngine(0);

            qRegisterMetaType<CustomConsoleLogObject*>("CustomConsoleLogObject*");
            qRegisterMetaType<QVector<unsigned char>>("QVector<unsigned char>");
            qScriptRegisterSequenceMetaType<QVector<unsigned char> >(m_scriptEngine);

            //Register the custom console/log object.
            m_scriptEngine->globalObject().setProperty("cust", m_scriptEngine->newQObject(this));
            m_scriptSql->registerScriptMetaTypes(m_scriptEngine);
            m_converterObject.registerScriptMetaTypes(m_scriptEngine);
            m_scriptEngine->globalObject().setProperty("scriptFile", m_scriptEngine->newQObject(this));

            connect(this, SIGNAL(appendTextToConsoleSignal(QString, bool,bool)),
                            m_mainWindow->getScriptWindow(), SLOT(appendTextToConsoleSlot(QString, bool,bool)), Qt::QueuedConnection);



            ScriptXmlReader::registerScriptMetaTypes(m_scriptEngine);
            ScriptXmlWriter::registerScriptMetaTypes(m_scriptEngine);

            if(m_runsInDebugger)
            {
                m_debugger = new QScriptEngineDebugger(this);
                m_debugWindow = m_debugger->standardWindow();
                m_debugWindow->setWindowModality(Qt::NonModal);
                m_debugWindow->resize(1280, 704);
                m_debugger->attachTo(m_scriptEngine);
                m_debugger->action(QScriptEngineDebugger::InterruptAction)->trigger();
                m_debugWindow->setWindowTitle(scriptFile.fileName());

                connect(m_mainWindow, SIGNAL(bringWindowsToFrontSignal()), this, SLOT(bringWindowsToFrontSlot()), Qt::DirectConnection);

#ifdef Q_OS_MAC
//Using this debugger signals causes the debugger to block (only on Mac OS X)

            connect(&m_debugTimer, SIGNAL(timeout()),this, SLOT(debugTimerSlot()));
            m_debugTimer.start(200);

#else
            connect(m_debugger, SIGNAL(evaluationSuspended()),
                    this, SLOT(suspendedByDebuggerSlot()), Qt::DirectConnection);
            connect(m_debugger, SIGNAL(evaluationResumed()),
                    this, SLOT(resumedByDebuggerSlot()), Qt::DirectConnection);
#endif
            }

            //set ScriptContext
            QScriptContext *context = m_scriptEngine->currentContext();
            QScriptContext *parent=context->parentContext();
            if(parent!=0)
            {
                context->setActivationObject(context->parentContext()->activationObject());
                context->setThisObject(context->parentContext()->thisObject());
            }

            QScriptValue result = m_scriptEngine->evaluate(scriptFile.readAll(), scriptPath);
            scriptFile.close();


            if (!result.isError())
            {
                *m_createString = m_scriptEngine->evaluate("createString");
                *hasSucceeded =  m_createString->isError() ? false : true;
            }

            if(m_scriptEngine->hasUncaughtException())
            {
                m_scriptFileObject->showExceptionInMessageBox(m_scriptEngine->uncaughtException(), scriptPath, m_scriptEngine,
                                                              m_mainWindow, m_mainWindow->getScriptWindow());
            }

        }
    }

    m_consoleLogObject->m_scriptFunctionIsFinished = true;

}

/**
 * Executes the script function 'createString'.
 * @param data
 *      The data argument for the 'createString' function.
 * @param timeStamp
 *      The timeStamp argument for the 'createString' function.
 * @param isSend
 *      The isSend argument for the 'createString' function.
 * @param isUserMessage
 *      The isUserMessage argument for the 'createString' function.
 * @param isFromCan
 *      The isFromCan argument for the 'createString' function.
 * @param isLog
 *      The isLog argument for the 'createString' function.
 *  * @param errorOccured
 *      True if an error has occured.
 * @param result
 *      The result (the created string).
 */
void CustomConsoleLogThread::executeScriptSlot(QByteArray* data, QString* timeStamp,
                                               bool isSend, bool isUserMessage, bool isFromCan, bool isLog,
                                               QString* result, bool* errorOccured)
{
    *errorOccured = false;
    QScriptValue scriptArray = m_scriptEngine->newArray(data->size());
    for(int i = 0; i < data->size(); i++)
    {
        scriptArray.setProperty(i, QScriptValue(m_scriptEngine, (unsigned char)data->at(i)));
    }
    quint32 type;
    if(isUserMessage)
    {
        type = 4;
    }
    else
    {
        if (isSend)
        {
            type = isFromCan ? 3 : 1;
        }
        else
        {
            type = isFromCan ? 2 : 0;
        }
    }

    //Call the createString function.
    QScriptValue val = m_createString->call(QScriptValue(), QScriptValueList() << scriptArray << *timeStamp << type << isLog);
    *result = val.toVariant().toString();


    if(m_scriptEngine->hasUncaughtException())
    {
        *errorOccured = true;
        m_scriptFileObject->showExceptionInMessageBox(m_scriptEngine->uncaughtException(), m_scriptPath, m_scriptEngine,
                                                      m_mainWindow, m_mainWindow->getScriptWindow());
    }
    m_consoleLogObject->m_scriptFunctionIsFinished = true;
}

/**
 * Calls the script function 'createString' in the script thread.
 * @param data
 *      The data argument.
 * @param timeStamp
 *      The time stamp (the format is set in the settings dialog).
 * @param isSend
 *      True if the data has been sent.
 * @param isUserMessage
 *      True if the data is a user message (from MessageDialog or from a script).
 * @param isFromCan
 *      True if the data is from CAN.
 * @param isLog
 *      True if this call is for the custom log (false=custom console).
 *  * @param errorOccured
 *      True if an error has occured.
 * @return
 *      The created string (result from the 'createString' call.
 *      If the call fails then a error message will be returned.
 */
QString CustomConsoleLogObject::callScriptFunction(QByteArray* data, QString& timeStamp,
                                                   bool isSend, bool isUserMessage, bool isFromCan,
                                                   bool isLog, bool* errorOccured)
{
    QString result;
    *errorOccured = false;

    if(!m_script->getRunsInDebugger())
    {
        if(!m_scriptIsBlocked)
        {

            m_scriptFunctionIsFinished = false;
            QDateTime callTime = QDateTime::currentDateTime();

            emit executeScriptSignal(data, &timeStamp, isSend, isUserMessage, isFromCan, isLog, &result, errorOccured);
            while(!m_scriptFunctionIsFinished)
            {
                QThread::usleep(1);

                if(callTime.msecsTo(QDateTime::currentDateTime()) > m_script->m_blockTime)
                {//Thread is blocked.
                    m_scriptIsBlocked = true;
                    terminateThread();
                    createThread(false);
                    *errorOccured = true;
                    break;
                }
            }
        }

        if(m_scriptIsBlocked)
        {
            if(isLog)
            {
                result = "\n";
            }
            else
            {
                result = "<br>";
            }
            result += m_scriptPath + " is blocked. The script has to be reloaded (uncheck and the check the correspondig check box)";
        }
    }
    else
    {
        if(!m_script->m_isSuspendedByDebuger)
        {
            m_script->executeScriptSlot(data, &timeStamp, isSend, isUserMessage, isFromCan, isLog, &result,errorOccured);
        }
    }

    return result;
}

/**
 * Creates a XML reader.
 * @return
 *      The created XML reader.
 */
QScriptValue CustomConsoleLogThread::createXmlReader()
{
    ScriptXmlReader* reader =  new ScriptXmlReader(m_scriptFileObject, this);
    return m_scriptEngine->newQObject(reader, QScriptEngine::ScriptOwnership);
}

/**
 * Creates a XML writer.
 * @return
 *      The created XML writer.
 */
QScriptValue CustomConsoleLogThread::createXmlWriter()
{
    ScriptXmlWriter* reader =  new ScriptXmlWriter(m_scriptFileObject, this);
    return m_scriptEngine->newQObject(reader, QScriptEngine::ScriptOwnership);
}


/**
 * Returns all functions and properties of an object.
 * @param object
 *      The object.
 * @return
 *      All functions and properties of the object.
 */
QStringList CustomConsoleLogThread::getAllObjectPropertiesAndFunctions(QScriptValue object)
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
 * Closes the debugger.
 */
void CustomConsoleLogThread::closeDebugger(void)
{

    if(m_runsInDebugger && m_debugger)
    {
        m_debugger->detach();
        m_debugWindow->close();
    }
}

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
#include "scriptThread.h"
#include "ui_scriptwindow.h"
#include<QTimer>
#include <QMenu>
#include <QFileDialog>
#include <QBuffer>
#include<QDomDocument>
#include "plotwindow.h"
#include "scriptComboBox.h"
#include "scriptLineEdit.h"
#include "mainwindow.h"
#include "scriptTableWidget.h"
#include "scriptTextEdit.h"
#include "scriptCheckBox.h"
#include "scriptButton.h"
#include "scriptPlotWindow.h"
#include "scriptDialog.h"
#include "scriptProgressBar.h"
#include "scriptLabel.h"
#include "scriptSlider.h"
#include "scriptMainWindow.h"
#include "scriptAction.h"
#include "scriptTabWidget.h"
#include "scriptGroupBox.h"
#include "scriptRadioButton.h"
#include "scriptSpinBox.h"
#include "scriptTimeEdit.h"
#include "scriptDateEdit.h"
#include "scriptListWidget.h"
#include "scriptTreeWidget.h"
#include "scriptToolButton.h"
#include "scriptCheetahSpi.h"
#include "scriptPcan.h"
#include <QDateTime>
#include "scriptSplitter.h"
#include "scriptDoubleSpinBox.h"
#include "scriptToolBox.h"
#include "scriptCalendarWidget.h"
#include "scriptDateTimeEdit.h"
#include "scriptSerialPort.h"
#include "scriptTcpClient.h"
#include "scriptTcpServer.h"
#include "scriptUdpSocket.h"
#include "scriptsqldatabase.h"
#include "scriptXml.h"
#include <QScriptEngineDebugger>
#include <QSerialPortInfo>
#include "ui_mainwindow.h"
#include "scriptTimer.h"



//Global data maps (scripts can exchange data with the maps).
static QMap<QString, QString> g_stringMap;
static QMap<QString, QVector<unsigned char>> g_dataMap;
static QMap<QString, quint32> g_unsignedNumberMap;
static QMap<QString, qint32> g_signedNumberMap;
static QMap<QString, double> g_realNumberMap;

//Mutexes for the global data maps access.
static QMutex g_stringMapMutex;
static QMutex g_dataMapMutex;
static QMutex g_unsignedNumberMapMutex;
static QMutex g_signedNumberMapMutex;
static QMutex g_realNumberMapMutex;


///Is set to true if a thread has been terminated.
///This variabke is used un the main function.
extern bool g_aThreadHasBeenTerminated;

/**
 * Constructor.
 * @param scriptWindow
 *      Pointer at the script window.
 * @param sendId
 *      The send id which is used for sending data.
 * @param scriptName
 *      Path to the script.
 * @param scriptUi
 *      The script user interface (load from an ui file).
 */
ScriptThread::ScriptThread(ScriptWindow* scriptWindow, quint32 sendId, QString scriptName, QWidget *scriptUi,
                           SettingsDialog *settingsDialog, bool scriptRunsInDebugger) :
    m_sendingSucceeded(false), m_shallExit(false), m_shallPause(false) ,m_scriptRunsInDebugger(scriptRunsInDebugger), m_state(INVALID),
    m_pauseTimer(0),m_scriptEngine(0), m_settingsDialog(settingsDialog), m_scriptSql(), m_blockTime(DEFAULT_BLOCK_TIME),
    m_standardDialogs(0), m_scriptFileObject(0), m_isSuspendedByDebuger(false), m_debugger(0), m_debugWindow(0), m_hasMainWindowGuiElements(false),
    sendDataFromMainInterfaceFunction(), m_libraries(0)
{
    m_scriptWindow = scriptWindow;

    m_sendId = sendId;
    m_scriptFileName = scriptName;
    m_userInterface.push_back(new ScriptWidget(scriptUi, this, m_scriptWindow));



}

/**
 * Destructor.
 */
ScriptThread::~ScriptThread()
{
    for(auto el : m_userInterface)
    {
        if(el->getWidgetPointer())
        {
            el->close();
            el->getWidgetPointer()->deleteLater();
        }
        el->deleteLater();
    }

    //Delete all created gui elements (created by the script).
    for(auto el : m_allCreatedGuiElementsFromScript)
    {
        el->deleteLater();
    }

    //Unload all loaded libraries.
    for(auto el : m_libraries)
    {
        el->unload();
        el->deleteLater();
    }
    m_libraries.clear();

}

///Returns a semicolon separated list with all public functions, signals and properties.
QString ScriptThread::getPublicScriptElements(void)
{
    return MainWindow::parseApiFile("scriptThread.api");
}



/**
 * Sets the priority of the script thread (which executes the current script).
 * Possible values are:
 * - LowestPriority
 * - LowPriority
 * - NormalPriority
 * - HighPriority
 * - HighestPriority
 *
 * Note: Per default script threads have LowestPriority.
 *
 * @param priority
 * @return
 */
bool ScriptThread::setScriptThreadPriority(QString priority)
{

    bool result = false;

    if(!m_scriptRunsInDebugger)
    {
        if(priority == "LowestPriority")
        {
            setPriority(QThread::LowestPriority);
            result = true;
        }
        else if(priority == "LowPriority")
        {
            setPriority(QThread::LowPriority);
            result = true;
        }
        else if(priority == "NormalPriority")
        {
            setPriority(QThread::NormalPriority);
            result = true;
        }
        else if(priority == "HighPriority")
        {
            setPriority(QThread::HighPriority);
            result = true;
        }
        else if(priority == "HighestPriority")
        {
            setPriority(QThread::HighestPriority);
            result = true;
        }
        else
        {
            result = false;
        }
    }
    else
    {
        result = true;
    }
    return result;
}

/**
 * The script is suspended by the debugger.
 */
void ScriptThread::suspendedByDebuggerSlot()
{
    m_shallPause = true;
    m_isSuspendedByDebuger = true;
    setThreadState(PAUSED);
    m_pauseTimer->start(1);
}

/**
 * The script is resumed by the debugger.
 */
void ScriptThread::resumedByDebuggerSlot()
{
    m_shallPause = false;
    m_isSuspendedByDebuger = false;
}

/**
 * The thread main function. Here the script is executed.
 */
void ScriptThread::run()
{
    try
    {
        Qt::ConnectionType directConnectionType = m_scriptRunsInDebugger ? Qt::DirectConnection : Qt::BlockingQueuedConnection ;

        setThreadState(CREATED);
        if(!m_scriptRunsInDebugger)
        {
            setPriority(QThread::NormalPriority);
        }

        qRegisterMetaType<QTextCursor>("QTextCursor");
        qRegisterMetaType<QVector<int>>("QVector<int>");
        qRegisterMetaType<QCPRange>("QCPRange");
        qRegisterMetaType<QVector<unsigned char>>("QVector<unsigned char>");
        qRegisterMetaType<const char*>("const char*");
        qRegisterMetaType<quint32*>("quint32*");
        qRegisterMetaType<ThreadSate>("ThreadSate");
        qRegisterMetaType<ScriptThread*>("ScriptThread*");
        qRegisterMetaType<Qt::Orientation>("Qt::Orientation");
        qRegisterMetaType<QTimer*>("QTimer*");
        qRegisterMetaType<ScriptSerialPort*>("ScriptSerialPort*");
        qRegisterMetaType<QString*>("QString*");
        qRegisterMetaType<ScriptTcpClient*>("ScriptTcpClient*");
        qRegisterMetaType<ScriptTcpServer*>("ScriptTcpServer*");
        qRegisterMetaType<ScriptUdpSocket*>("ScriptUdpSocket*");
        qRegisterMetaType<ScriptCheetahSpi*>("ScriptCheetahSpi*");
        qRegisterMetaType<ScriptPlotWindow*>("ScriptPlotWindow*");
        qRegisterMetaType<ScriptPcan*>("ScriptPcan*");
        qRegisterMetaType<Qt::WindowFlags>("Qt::WindowFlags");
        qRegisterMetaType<QItemSelection>("QItemSelection");
        qRegisterMetaType<ScriptTreeWidgetItem*>("ScriptTreeWidgetItem*");
        qRegisterMetaType<QPersistentModelIndex>("QPersistentModelIndex");
        qRegisterMetaType<QList<QPersistentModelIndex>>("QList<QPersistentModelIndex>");
        qRegisterMetaType<QAbstractItemModel::LayoutChangeHint>("QAbstractItemModel::LayoutChangeHint");
        qRegisterMetaType<QList<double>>("QList<double>");
        qRegisterMetaType<QList<quint8>>("QList<quint8>");
        qRegisterMetaType<QList<quint32>>("QList<quint32>");
        qRegisterMetaType<QList<qint32>>("QList<qint32>");
        qRegisterMetaType<ScriptPlotWidget*>("ScriptPlotWidget*");
        qRegisterMetaType< QMessageBox::Icon>("QMessageBox::Icon");
        qRegisterMetaType<QMessageBox::StandardButtons>("QMessageBox::StandardButtons");
        qRegisterMetaType<Context2D*>("Context2D*");
        qRegisterMetaType<QVector<quint8>>("QVector<quint8>");
        qRegisterMetaType<QVector<quint32>>("QVector<quint32>");
        qRegisterMetaType<QVector<QVector<unsigned char>>>("QVector<QVector<unsigned char>>");


        connect(this, SIGNAL(setSerialPortPinsSignal(bool,bool)),
                m_scriptWindow->m_mainWindow, SLOT(setSerialPortPinsSlot(bool,bool)), directConnectionType);

        connect(this, SIGNAL(setMainWindowTitleSignal(QString)),
                m_scriptWindow->m_mainWindow, SLOT(setWindowTitle(QString)), directConnectionType);

        connect(this, SIGNAL(addTabsToMainWindowSignal(QTabWidget*)),
                m_scriptWindow->m_mainWindow, SLOT(addTabsToMainWindowSlot(QTabWidget*)), Qt::QueuedConnection);

        connect(m_scriptWindow->m_mainWindow->getUserInterface()->actionClear, SIGNAL(triggered()),
                this, SLOT(mainWindowClearConsoleSlot()), Qt::QueuedConnection);

        connect(m_scriptWindow->m_mainWindow->getUserInterface()->actionLockScrolling, SIGNAL(toggled(bool)),
                this, SLOT(mainWindowLockScrollingSlot(bool)), Qt::QueuedConnection);

        connect(this, SIGNAL(setMainWindowAndTaskBarIconSignal(QString)),
                m_scriptWindow->m_mainWindow, SLOT(setMainWindowAndTaskBarIconSlot(QString)), Qt::QueuedConnection);

        connect(this, SIGNAL(addToolBoxPagesToMainWindowSignal(QToolBox*)),
                m_scriptWindow->m_mainWindow, SLOT(addToolBoxPagesToMainWindowSlot(QToolBox*)), Qt::QueuedConnection);

        connect(this, SIGNAL(enableAllTabsForOneScriptThreadSignal(QObject*,bool)),
                m_scriptWindow->m_mainWindow, SLOT(enableAllTabsForOneScriptThreadSlot(QObject*,bool)), directConnectionType);

        connect(this, SIGNAL(addDataToMainWindowSendHistorySignal(QByteArray)),
                m_scriptWindow->m_mainWindow, SLOT(addDataToMainWindowSendHistorySlot(QByteArray)), Qt::QueuedConnection);


        connect(this, SIGNAL(getSerialPortSignalsSignal(uint32_t*)),
                m_scriptWindow->m_mainInterfaceThread, SLOT(getSerialPortSignals(uint32_t*)), directConnectionType);


        connect(this, SIGNAL(createGuiElementSignal(QString,QObject**, ScriptWindow*, ScriptThread*, QObject*)),
                m_scriptWindow, SLOT(createGuiElementSlot(QString,QObject**,ScriptWindow*,ScriptThread*,QObject*)), directConnectionType);

        connect(this, SIGNAL(loadUserInterfaceFileSignal(QWidget**,QString)),
                m_scriptWindow, SLOT(loadUserInterfaceFileSlot(QWidget**,QString)), directConnectionType);

        connect(this, SIGNAL(addMessageToLogAndConsolesSignal(QString, bool)),m_scriptWindow->m_mainWindow, SLOT(messageEnteredSlot(QString, bool)),
                directConnectionType);

        connect(this, SIGNAL(setAllSettingsSignal(Settings&,bool)),
                m_settingsDialog, SLOT(setAllSettingsSlot(Settings&,bool)), directConnectionType);

        connect(this, SIGNAL(appendTextToConsoleSignal(QString, bool,bool)),
                m_scriptWindow, SLOT(appendTextToConsoleSlot(QString, bool,bool)), Qt::QueuedConnection);

        connect(m_scriptWindow->m_mainInterfaceThread, SIGNAL(dataReceivedSignal(QByteArray)),
                this, SLOT(dataReceivedSlot(QByteArray)), Qt::QueuedConnection);

        connect(m_scriptWindow->m_mainInterfaceThread, SIGNAL(canMessagesReceivedSignal(QVector<QByteArray>)),
                this, SLOT(canMessagesReceivedSlot(QVector<QByteArray>)), Qt::QueuedConnection);


        connect(m_scriptWindow->m_mainInterfaceThread, SIGNAL(sendDataWithWorkerScriptsSignal(QByteArray)),
                this, SLOT(sendDataFromMainInterfaceSlot(QByteArray)), Qt::QueuedConnection);

        connect(this, SIGNAL(exitScriptCommunicatorSignal()),
                m_scriptWindow, SLOT(exitScriptCommunicatorSlot()), Qt::QueuedConnection);

        connect(m_scriptWindow->m_mainInterfaceThread, SIGNAL(dataConnectionStatusSignal(bool, QString, bool)),
                this, SLOT(dataConnectionStatusSlot(bool,QString,bool)), Qt::DirectConnection);

        connect(this, SIGNAL(sendDataSignal(const QByteArray, uint)),
                m_scriptWindow->m_mainInterfaceThread, SLOT(sendDataSlot(const QByteArray, uint)), Qt::BlockingQueuedConnection);

        connect(m_scriptWindow->m_mainInterfaceThread, SIGNAL(sendingFinishedSignal(bool,uint)),
                this, SLOT(sendingFinishedSlot(bool,uint)), Qt::DirectConnection);

        connect(this, SIGNAL(threadStateChangedSignal(ThreadSate, ScriptThread*)),
                m_scriptWindow, SLOT(threadStateChangedSlot(ThreadSate, ScriptThread*)), Qt::QueuedConnection);

        connect(this, SIGNAL(connectDataConnectionSignal(Settings, bool)),m_scriptWindow->m_mainInterfaceThread,
                SLOT(connectDataConnectionSlot(Settings, bool)), Qt::BlockingQueuedConnection);

        connect(this, SIGNAL(setScriptStateSignal(quint8,QString,bool*)),m_scriptWindow,
                SLOT(setScriptStateSlot(quint8,QString,bool*)), directConnectionType);

        connect(m_scriptWindow, SIGNAL(globalDataArrayChangedSignal(QString*,QVector<unsigned char>*)),
                this, SLOT(globalDataArrayChangedSlot(QString*,QVector<unsigned char>*)), Qt::DirectConnection);

        connect(m_scriptWindow, SIGNAL(globalSignedChangedSignal(QString*,qint32)),
                this, SLOT(globalSignedChangedSlot(QString*,qint32)), Qt::DirectConnection);

        connect(m_scriptWindow, SIGNAL(globalStringChangedSignal(QString*,QString*)),
                this, SLOT(globalStringChangedSlot(QString*,QString*)), Qt::DirectConnection);

        connect(m_scriptWindow, SIGNAL(globalUnsignedChangedSignal(QString*,quint32)),
                this, SLOT(globalUnsignedChangedSlot(QString*,quint32)), Qt::DirectConnection);

        connect(m_scriptWindow, SIGNAL(globalRealChangedSignal(QString*,double)),
                this, SLOT(globalRealChangedSlot(QString*,double)), Qt::DirectConnection);

        connect(this, SIGNAL(getScriptTableNameSignal(QString*)),
                m_scriptWindow, SLOT(getScriptTableNameSlot(QString*)), directConnectionType);

        //start the pause timer
        m_pauseTimer = new QTimer(this);
        m_pauseTimer->setInterval(100);
        m_pauseTimer->start();
        connect(m_pauseTimer, SIGNAL(timeout()),this, SLOT(pauseTimerSlot()));

        //get the connection state of the main interface
        m_isConnected = m_scriptWindow->m_mainInterfaceThread->isConnected();

        m_isConnectedWithCan = m_scriptWindow->m_mainInterfaceThread->isConnectedWithCan();

        //create the script engine
        m_scriptEngine = new QScriptEngine();
        ScriptMap::registerScriptMetaTypes(m_scriptEngine);
        ScriptXmlReader::registerScriptMetaTypes(m_scriptEngine);
        ScriptXmlWriter::registerScriptMetaTypes(m_scriptEngine);
        ScriptTableCellPosition::registerType(m_scriptEngine);


        qScriptRegisterSequenceMetaType<QVector<unsigned char> >(m_scriptEngine);
        qScriptRegisterSequenceMetaType<QVector<quint8> >(m_scriptEngine);
        qScriptRegisterSequenceMetaType<QVector<quint32> >(m_scriptEngine);
        qScriptRegisterSequenceMetaType<QVector<QVector<unsigned char>> >(m_scriptEngine);
        qScriptRegisterSequenceMetaType<QList<double> >(m_scriptEngine);
        qScriptRegisterSequenceMetaType<QList<quint8> >(m_scriptEngine);
        qScriptRegisterSequenceMetaType<QList<int> >(m_scriptEngine);
        qScriptRegisterSequenceMetaType<QList<quint32> >(m_scriptEngine);
        qScriptRegisterSequenceMetaType<QList<qint32> >(m_scriptEngine);


         if(!m_scriptRunsInDebugger)
         {
            connect(m_scriptEngine, SIGNAL(signalHandlerException(QScriptValue)),
                    this, SLOT(scriptSignalHandlerSlot(QScriptValue)));
         }

        //register the script thread object
        m_scriptEngine->globalObject().setProperty("scriptThread", m_scriptEngine->newQObject(this));

        m_scriptSql.registerScriptMetaTypes(m_scriptEngine);
        m_converterObject.registerScriptMetaTypes(m_scriptEngine);

        m_standardDialogs = new ScriptStandardDialogs(this);
        m_standardDialogs->intSignals(m_scriptWindow, m_scriptRunsInDebugger);

        m_scriptFileObject = new ScriptFile(this, m_scriptFileName);
        m_scriptFileObject->intSignals(m_scriptWindow, m_scriptEngine, m_scriptRunsInDebugger);
        m_scriptEngine->globalObject().setProperty("scriptFile", m_scriptEngine->newQObject(m_scriptFileObject));


        if(m_userInterface[0]->getWidgetPointer())
        {//the script has an user interface

            //install all elements from the script user interface
            installAllChilds(m_userInterface[0]->getWidgetPointer(), m_scriptEngine, true);
            m_userInterface[0]->show();
        }

        //Must be here (if a script blocks the main function, then loadScript will block too).
        setThreadState(RUNNING);


        if(m_scriptRunsInDebugger)
        {
            m_debugger = new QScriptEngineDebugger(this);
            m_debugWindow = m_debugger->standardWindow();
            m_debugWindow->setWindowModality(Qt::NonModal);
            m_debugWindow->resize(1280, 704);
            m_debugger->attachTo(m_scriptEngine);
            m_debugger->action(QScriptEngineDebugger::InterruptAction)->trigger();
            m_debugWindow->setWindowTitle(m_scriptFileName);

            connect(&m_debugReceiveTimer, SIGNAL(timeout()),this, SLOT(debugReceiveTimerSlot()));
            m_debugReceiveTimer.setInterval(20);



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



        if (loadScript(m_scriptFileName, false))
        {
            if(!m_scriptRunsInDebugger)
            {
                exec();
            }
            else
            {
                while(!m_shallExit)
                {
                    QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
                    if(!m_debugWindow->isVisible())
                    {
                        stopScript();
                    }
                }

            }

            //stop the timer
            m_pauseTimer->stop();
            blockSignals(false);

            if(m_scriptRunsInDebugger)
            {
#ifdef Q_OS_MAC
                m_debugTimer.stop();
#endif
                m_debugger->detach();
            }
            //call the script stop function
            QScriptValue stopFunction = m_scriptEngine->evaluate("stopScript");
            if(m_scriptRunsInDebugger){m_debugger->attachTo(m_scriptEngine);}

            if (stopFunction.isError())
            {//The script has no stop function

            }
            else
            {
                stopFunction.call();

                if(m_scriptEngine->hasUncaughtException())
                {//In stopScript an error has been occured.

                    QWidget* parent = (m_scriptWindow->isVisible()) ? static_cast<QWidget *>(m_scriptWindow) : static_cast<QWidget *>(m_scriptWindow->getMainWindow());
                    m_scriptFileObject->showExceptionInMessageBox(m_scriptEngine->uncaughtException(), m_scriptFileName, m_scriptEngine, parent, m_scriptWindow);
                }
            }



        }//if (loadScript(m_scriptFileName))

        delete m_pauseTimer;
        delete m_scriptEngine;
        m_isSuspendedByDebuger = false;



    }
    catch(...)
    {
        messageBox("Critical", m_scriptFileName, "error during script execution");
    }

    setThreadState(EXITED);
}

/**
 * Creates a serial port.
 * @return
 *      The created serial port.
 */
QScriptValue ScriptThread::createSerialPort(void)
{
    ScriptSerialPort* serialPort =  new ScriptSerialPort(this, m_scriptWindow->m_mainInterfaceThread);
    return m_scriptEngine->newQObject(serialPort, QScriptEngine::ScriptOwnership);
}

/**
 * Creates cheetah spi interface.
 * @return
 *      The created interface.
 */
QScriptValue ScriptThread::createCheetahSpiInterface(void)
{
    ScriptCheetahSpi* spiInterface = new ScriptCheetahSpi(this);
    return m_scriptEngine->newQObject(spiInterface, QScriptEngine::ScriptOwnership);
}

/**
 * Creates a pcan interface.
 * @return
 *      The created pcan interface.
 */
QScriptValue ScriptThread::createPcanInterface(void)
{
    ScriptPcan* pcan = new ScriptPcan(this);
    return m_scriptEngine->newQObject(pcan, QScriptEngine::ScriptOwnership);

}

/**
 * Creates a timer.
 * @return
 *      The created timer.
 */
QScriptValue ScriptThread::createTimer(void)
{
    ScriptTimer* timer =  new ScriptTimer(this);
    return m_scriptEngine->newQObject(timer, QScriptEngine::ScriptOwnership);
}

/**
 * Creates a TCP socket.
 * @return
 *      The created socket.
 */
QScriptValue ScriptThread::createTcpClient(void)
{
    ScriptTcpClient* socket =  new ScriptTcpClient(new QTcpSocket(this), this, m_scriptWindow->m_mainInterfaceThread);
    return m_scriptEngine->newQObject(socket, QScriptEngine::ScriptOwnership);
}

/**
 * Creates an UDP socket.
 * @return
 *      The created socket.
 */
QScriptValue ScriptThread::createUdpSocket(void)
{
    ScriptUdpSocket* socket =  new ScriptUdpSocket(this, m_scriptWindow->m_mainInterfaceThread);
    return m_scriptEngine->newQObject(socket, QScriptEngine::ScriptOwnership);
}

/**
 * Creates a TCP server.
 * @return
 *      The created server.
 */
QScriptValue ScriptThread::createTcpServer(void)
{
    ScriptTcpServer* server =  new ScriptTcpServer(this, m_scriptWindow->m_mainInterfaceThread);
    return m_scriptEngine->newQObject(server, QScriptEngine::ScriptOwnership);
}

/**
 * Creates a plot window.
 * @return
 *      The create plot window.
 */
QScriptValue ScriptThread::createPlotWindow()
{
    ScriptPlotWindow* scriptPlotWindow = 0;
    QObject* obj = 0;
    emit createGuiElementSignal("ScriptPlotWindow", &obj, m_scriptWindow, this, 0);

    if(obj != 0)
    {
        scriptPlotWindow = static_cast<ScriptPlotWindow*>(obj);
        m_allCreatedGuiElementsFromScript.push_back(scriptPlotWindow);
    }
    return m_scriptEngine->newQObject(scriptPlotWindow, QScriptEngine::QtOwnership);
}

/**
 * Creates a XML reader.
 * @return
 *      The created XML reader.
 */
QScriptValue ScriptThread::createXmlReader()
{
    ScriptXmlReader* reader =  new ScriptXmlReader(m_scriptFileObject, this);
    return m_scriptEngine->newQObject(reader, QScriptEngine::ScriptOwnership);
}

/**
 * Creates a XML writer.
 * @return
 *      The created XML writer.
 */
QScriptValue ScriptThread::createXmlWriter()
{
    ScriptXmlWriter* reader =  new ScriptXmlWriter(m_scriptFileObject, this);
    return m_scriptEngine->newQObject(reader, QScriptEngine::ScriptOwnership);
}

/**
 * Deletes an object created by the script.
 * Note: This function must not used any more.
 * Objects are deleted automatically by the script engine garbage collector.
 * @param obj
 *      The object.
 */
void ScriptThread::deleteObject(QObject* obj)
{
    (void)obj;
    //Do nothing.
}

/**
 * This function installs one custom widget.
 * @param child
 *      The child object.
 * @param scriptEngine
 *      The script engine.
 */
void ScriptThread::installsCustomWidget(QObject* child, QScriptEngine* scriptEngine)
{
     QStringList files;

     //Look in the extra paths.
     for(auto el : m_scriptWindow->m_mainWindow->getExtraPluginPaths())
     {
         QStringList tmpFiles = QDir(el).entryList(QDir::Files);

         //Create the complete paths.
         for(qint32 i = 0; i < tmpFiles.length(); i++)
         {
            files << el + "/" + tmpFiles[i];
         }

     }

     QString searchFolder = MainWindow::getPluginsFolder() + "/designer";
     QDir dir(searchFolder);
     QStringList tmpFiles = dir.entryList(QDir::Files);

     //Create the complete paths.
     for(qint32 i = 0; i < tmpFiles.length(); i++)
     {
        files << searchFolder + "/" + tmpFiles[i];
     }

     for(int i = 0; i < files.length(); i++)
     {
         QLibrary lib(files[i]);
         lib.load();
         if(lib.isLoaded())
         {
             GetScriptCommunicatorWidgetName func = (GetScriptCommunicatorWidgetName) lib.resolve("GetScriptCommunicatorWidgetName");
             if(func)
             {
                 if(strcmp(child->metaObject()->className(), func()) == 0)
                 {
                     CreateScriptCommunicatorWidget func = (CreateScriptCommunicatorWidget) lib.resolve("CreateScriptCommunicatorWidget");
                     if(func)
                     {
                         QObject* element = func(this, static_cast<QWidget*>(child), m_scriptRunsInDebugger);
                         scriptEngine->globalObject().setProperty("UI_" + child->objectName(), scriptEngine->newQObject(element));
                         break;
                     }
                 }
             }
         }//if(lib.isLoaded())
     }

}

/**
 * This function installs one (child) object. This object can be accessed from the script.
 * @param child
 *      The child object.
 * @param scriptEngine
 *      The script engine.
 */
void ScriptThread::installOneChild(QObject* child, QScriptEngine* scriptEngine)
{
    QString objectName = "UI_" + child->objectName();

    if((QString(child->metaObject()->className()) == QString("QComboBox")) ||
            (QString(child->metaObject()->className()) == QString("QFontComboBox")))
    {
        ScriptComboBox* element = new ScriptComboBox(static_cast<QComboBox*>(child), this);
        scriptEngine->globalObject().setProperty(objectName, scriptEngine->newQObject(element));
        element->setObjectName(objectName);
    }
    else if(QString(child->metaObject()->className()) == QString("QLineEdit"))
    {
        ScriptLineEdit* element = new ScriptLineEdit(static_cast<QLineEdit*>(child), this);
        scriptEngine->globalObject().setProperty(objectName, scriptEngine->newQObject(element));
        element->setObjectName(objectName);
    }
    else if(QString(child->metaObject()->className()) == QString("QTableWidget"))
    {
        ScriptTableWidget* element = new ScriptTableWidget(static_cast<QTableWidget*>(child), this);
        scriptEngine->globalObject().setProperty(objectName, scriptEngine->newQObject(element));
        element->setObjectName(objectName);
    }
    else if(QString(child->metaObject()->className()) == QString("QTextEdit"))
    {
        ScriptTextEdit* element = new ScriptTextEdit(static_cast<QTextEdit*>(child), this, m_scriptWindow);
        scriptEngine->globalObject().setProperty(objectName, scriptEngine->newQObject(element));
        element->setObjectName(objectName);
    }
    else if(QString(child->metaObject()->className()) == QString("QCheckBox"))
    {
        ScriptCheckBox* element = new ScriptCheckBox(static_cast<QCheckBox*>(child), this);
        scriptEngine->globalObject().setProperty(objectName, scriptEngine->newQObject(element));
        element->setObjectName(objectName);
    }
    else if(QString(child->metaObject()->className()) == QString("QPushButton"))
    {
        ScriptButton* element = new ScriptButton(static_cast<QPushButton*>(child), this);
        scriptEngine->globalObject().setProperty(objectName, scriptEngine->newQObject(element));
        element->setObjectName(objectName);
    }
    else if(QString(child->metaObject()->className()) == QString("QToolButton"))
    {
        ScriptToolButton* element = new ScriptToolButton(static_cast<QToolButton*>(child), this);
        scriptEngine->globalObject().setProperty(objectName, scriptEngine->newQObject(element));
        element->setObjectName(objectName);
    }
    else if(QString(child->metaObject()->className()) == QString("QWidget"))
    {
        ScriptWidget* element = new ScriptWidget(static_cast<QWidget*>(child), this, m_scriptWindow);
        scriptEngine->globalObject().setProperty(objectName, scriptEngine->newQObject(element));
        element->setObjectName(objectName);
    }
    else if(QString(child->metaObject()->className()) == QString("QDialog"))
    {
        ScriptDialog* element = new ScriptDialog(static_cast<QDialog*>(child), this);
        scriptEngine->globalObject().setProperty(objectName, scriptEngine->newQObject(element));
        element->setObjectName(objectName);

        connect(m_scriptWindow->m_mainWindow, SIGNAL(bringWindowsToFrontSignal()),element, SLOT(bringWindowsToFrontSlot()), Qt::DirectConnection);
    }
    else if(QString(child->metaObject()->className()) == QString("QProgressBar"))
    {
        ScriptProgressBar* element = new ScriptProgressBar(static_cast<QProgressBar*>(child), this);
        scriptEngine->globalObject().setProperty(objectName, scriptEngine->newQObject(element));
        element->setObjectName(objectName);
    }
    else if(QString(child->metaObject()->className()) == QString("QLabel"))
    {
        ScriptLabel* element = new ScriptLabel(static_cast<QLabel*>(child), this);
        scriptEngine->globalObject().setProperty(objectName, scriptEngine->newQObject(element));
        element->setObjectName(objectName);
    }
    else if(QString(child->metaObject()->className()) == QString("QSlider"))
    {
        ScriptSlider* element = new ScriptSlider(static_cast<QAbstractSlider*>(child), this);
        scriptEngine->globalObject().setProperty(objectName, scriptEngine->newQObject(element));
        element->setObjectName(objectName);
    }
    else if(QString(child->metaObject()->className()) == QString("QDial"))
    {
        ScriptSlider* element = new ScriptSlider(static_cast<QAbstractSlider*>(child), this);
        scriptEngine->globalObject().setProperty(objectName, scriptEngine->newQObject(element));
        element->setObjectName(objectName);
    }
    else if(QString(child->metaObject()->className()) == QString("QMainWindow"))
    {
        ScriptMainWindow* element = new ScriptMainWindow(static_cast<QMainWindow*>(child), this);
        scriptEngine->globalObject().setProperty(objectName, scriptEngine->newQObject(element));
        element->setObjectName(objectName);

        connect(m_scriptWindow->m_mainWindow, SIGNAL(bringWindowsToFrontSignal()),element, SLOT(bringWindowsToFrontSlot()), Qt::DirectConnection);
    }
    else if(QString(child->metaObject()->className()) == QString("QAction"))
    {
        ScriptAction* element = new ScriptAction(static_cast<QAction*>(child), this);
        scriptEngine->globalObject().setProperty(objectName, scriptEngine->newQObject(element));
        element->setObjectName(objectName);
    }
    else if(QString(child->metaObject()->className()) == QString("QTabWidget"))
    {
        ScriptTabWidget* element = new ScriptTabWidget(static_cast<QTabWidget*>(child), this);
        scriptEngine->globalObject().setProperty(objectName, scriptEngine->newQObject(element));
        element->setObjectName(objectName);
    }
    else if(QString(child->metaObject()->className()) == QString("QGroupBox"))
    {
        ScriptGroupBox* element = new ScriptGroupBox(static_cast<QGroupBox*>(child), this);
        scriptEngine->globalObject().setProperty(objectName, scriptEngine->newQObject(element));
        element->setObjectName(objectName);
    }
    else if(QString(child->metaObject()->className()) == QString("QRadioButton"))
    {
        ScriptRadioButton* element = new ScriptRadioButton(static_cast<QRadioButton*>(child), this);
        scriptEngine->globalObject().setProperty(objectName, scriptEngine->newQObject(element));
        element->setObjectName(objectName);
    }
    else if(QString(child->metaObject()->className()) == QString("QSpinBox"))
    {
        ScriptSpinBox* element = new ScriptSpinBox(static_cast<QSpinBox*>(child), this);
        scriptEngine->globalObject().setProperty(objectName, scriptEngine->newQObject(element));
        element->setObjectName(objectName);
    }
    else if(QString(child->metaObject()->className()) == QString("QDoubleSpinBox"))
    {
        ScriptDoubleSpinBox* element = new ScriptDoubleSpinBox(static_cast<QDoubleSpinBox*>(child), this);
        scriptEngine->globalObject().setProperty(objectName, scriptEngine->newQObject(element));
        element->setObjectName(objectName);
    }
    else if(QString(child->metaObject()->className()) == QString("QTimeEdit"))
    {
        ScriptTimeEdit* element = new ScriptTimeEdit(static_cast<QTimeEdit*>(child), this);
        scriptEngine->globalObject().setProperty(objectName, scriptEngine->newQObject(element));
        element->setObjectName(objectName);
    }
    else if(QString(child->metaObject()->className()) == QString("QDateEdit"))
    {
        ScriptDateEdit* element = new ScriptDateEdit(static_cast<QDateEdit*>(child), this);
        scriptEngine->globalObject().setProperty(objectName, scriptEngine->newQObject(element));
        element->setObjectName(objectName);
    }
    else if(QString(child->metaObject()->className()) == QString("QDateTimeEdit"))
    {
        ScriptDateTimeEdit* element = new ScriptDateTimeEdit(static_cast<QDateTimeEdit*>(child), this);
        scriptEngine->globalObject().setProperty(objectName, scriptEngine->newQObject(element));
        element->setObjectName(objectName);
    }
    else if(QString(child->metaObject()->className()) == QString("QListWidget"))
    {
        ScriptListWidget* element = new ScriptListWidget(static_cast<QListWidget*>(child), this);
        scriptEngine->globalObject().setProperty(objectName, scriptEngine->newQObject(element));
        element->setObjectName(objectName);
    }
    else if(QString(child->metaObject()->className()) == QString("QTreeWidget"))
    {
        ScriptTreeWidget* element = new ScriptTreeWidget(static_cast<QTreeWidget*>(child), this);
        scriptEngine->globalObject().setProperty(objectName, scriptEngine->newQObject(element));
        element->setObjectName(objectName);
    }
    else if(QString(child->metaObject()->className()) == QString("QSplitter"))
    {
        ScriptSplitter* element = new ScriptSplitter(static_cast<QSplitter*>(child), this);
        scriptEngine->globalObject().setProperty(objectName, scriptEngine->newQObject(element));
        element->setObjectName(objectName);
    }
    else if(QString(child->metaObject()->className()) == QString("QToolBox"))
    {
        ScriptToolBox* element = new ScriptToolBox(static_cast<QToolBox*>(child), this);
        scriptEngine->globalObject().setProperty(objectName, scriptEngine->newQObject(element));
        element->setObjectName(objectName);
    }
    else if(QString(child->metaObject()->className()) == QString("QCalendarWidget"))
    {
        ScriptCalendarWidget* element = new ScriptCalendarWidget(static_cast<QCalendarWidget*>(child), this);
        scriptEngine->globalObject().setProperty(objectName, scriptEngine->newQObject(element));
        element->setObjectName(objectName);
    }
    else if(QString(child->metaObject()->className()) == QString("QTableView"))
    {
        messageBox("Warning", "not supported gui element", "QTableView is not supported, use QTableWidget instead");
    }
    else if(QString(child->metaObject()->className()) == QString("QListView"))
    {
        messageBox("Warning", "not supported gui element", "QListView is not supported, use QListWidget instead");
    }
    else if(QString(child->metaObject()->className()) == QString("QTreeView"))
    {
        messageBox("Warning", "not supported gui element", "QTreeView is not supported, use QTreeWidget instead");
    }
    else if(QString(child->metaObject()->className()) == QString("QPlainTextEdit"))
    {
        messageBox("Warning", "not supported gui element", "QPlainTextEdit is not supported, use QTextEdit instead");
    }
    else if(QString(child->metaObject()->className()) == QString("QFormInternal::TranslationWatcher") ||
            QString(child->metaObject()->className()) == QString("QVBoxLayout") ||
            QString(child->metaObject()->className()) == QString("QScrollBar") ||
            QString(child->metaObject()->className()) == QString("QBoxLayout") ||
            QString(child->metaObject()->className()) == QString("QWidgetTextControl") ||
            QString(child->metaObject()->className()) == QString("QTextDocument") ||
            QString(child->metaObject()->className()) == QString("QTextDocumentLayout") ||
            QString(child->metaObject()->className()) == QString("QTextImageHandler") ||
            QString(child->metaObject()->className()) == QString("QTextFrame") ||
            QString(child->metaObject()->className()) == QString("QToolBoxButton") ||
            QString(child->metaObject()->className()) == QString("QScrollArea") ||
            QString(child->metaObject()->className()) == QString("QGridLayout") ||
            QString(child->metaObject()->className()) == QString("QHBoxLayout") ||
            QString(child->metaObject()->className()) == QString("QWidgetLineControl") ||
            QString(child->metaObject()->className()) == QString("QValidator") ||
            QString(child->metaObject()->className()) == QString("QCalendarModel") ||
            QString(child->metaObject()->className()) == QString("QCalendarView") ||
            QString(child->metaObject()->className()) == QString("QStyledItemDelegate") ||
            QString(child->metaObject()->className()) == QString("QHeaderView") ||
            QString(child->metaObject()->className()) == QString("QItemSelectionModel") ||
            QString(child->metaObject()->className()) == QString("QTableCornerButton") ||
            QString(child->metaObject()->className()) == QString("QPrevNextCalButton") ||
            QString(child->metaObject()->className()) == QString("QMenu") ||
            QString(child->metaObject()->className()) == QString("QCalendarTextNavigator") ||
            QString(child->metaObject()->className()) == QString("QCalendarDelegate") ||
            QString(child->metaObject()->className()) == QString("QSplitterHandle") ||
            QString(child->metaObject()->className()) == QString("QStackedWidget") ||
            QString(child->metaObject()->className()) == QString("QStandardItemModel") ||
            QString(child->metaObject()->className()) == QString("QTreeModel") ||
            QString(child->metaObject()->className()) == QString("QListModel") ||
            QString(child->metaObject()->className()) == QString("QStackedLayout") ||
            QString(child->metaObject()->className()) == QString("QTableModel") ||
            QString(child->metaObject()->className()) == QString("QTabBar") ||
            QString(child->metaObject()->className()) == QString("QStatusBar"))
    {//not supported class

    }
    else
    {//unknown class (this could be a custom script widget)

        installsCustomWidget(child, scriptEngine);
    }

}


/**
 * Installs obj and all child objects from obj. This objects can be accessed from the script.
 * @param obj
 *      The object.
 * @param scriptEngine
 *      The script engine.
 * @param firstObj
 *      True, if obj is the first object (this function is called recursive for every object)
 */
void ScriptThread::installAllChilds(QObject* obj, QScriptEngine* scriptEngine, bool firstObj)
{
    if(firstObj)
    {
        //the first object is the dialog
        installOneChild(obj, scriptEngine);

    }
    for(auto child : obj->children())
    {
        installOneChild(child, scriptEngine);
        installAllChilds(child,scriptEngine);

    }
}


/**
 * Forces the script thread to sleep for ms milliseconds).
 * @param timeMs
 *      The time in ms.
 */
void ScriptThread::sleepFromScript(quint32 timeMs )
{
    msleep(timeMs);
    QCoreApplication::processEvents();
}

/**
 * Wrapper for QFileDialog::getSaveFileName and QFileDialog::getOpenFileName
 * (shows a QFileDialog::getSaveFileName or a QFileDialog::getOpenFileName dialog).
 * @param isSaveDialog
 *      True for a QFileDialog::getSaveFileName and false for a QFileDialog::getOpenFileName dialog.
 * @param caption
 *      The caption of the dialog.
 * @param dir
 *      The initial dir for showing the dialog.
 * @param filter
 *      Filter for the file dialog.
 * @param parent
 *      The parent of the dialog.
 * @return
 *      The path of the selected file
 */
QString ScriptThread::showFileDialog(bool isSaveDialog, QString caption, QString dir, QString filter, QWidget* parent)
{
    return m_standardDialogs->showFileDialog(isSaveDialog, caption, dir, filter, (parent == 0) ? m_userInterface[0]->getWidgetPointer() : parent);
}


/**
 * Wrapper for QFileDialog::getExistingDirectory.
 * @param caption
 *      The caption of the dialog.
 * @param dir
 *      The initial dir for showing the dialog.
 * @param parent
 *      The parent of the dialog.
 * @return
 *      The selected directory.
 */
QString ScriptThread::showDirectoryDialog(QString caption, QString dir, QWidget* parent)
{
    return m_standardDialogs->showDirectoryDialog(caption, dir, (parent == 0) ? m_userInterface[0]->getWidgetPointer() : parent);
}

/**
 * Shows a message box.
 * (emits the ScriptThread::showMessageBoxSignal signal).
 * @param icon
 *      The icon if the message box. Possible values are:
 *      - Information
 *      - Warning
 *      - Critical
 *      - Question
 * @param title
 *      The title of the message box.
 * @param text
 *      The text of the message box.
 * @param parent
 *      The parent of the dialog.
 */
void ScriptThread::messageBox(QString icon, QString title, QString text, QWidget* parent)
{
    m_standardDialogs->messageBox(icon, title, text, (parent == 0) ? m_userInterface[0]->getWidgetPointer() : parent);
}

/**
 * Shows a yes/no dialog.
 * @param icon
 *      The icon if the message box. Possible values are:
 *      - Information
 *      - Warning
 *      - Critical
 *      - Question
 * @param title
 *      The title of the message box.
 * @param text
 *      The text of the message box.
 * @param parent
 *      The parent of the dialog.
 * @return
 *      True if the yes button has been pressed.
 */
bool ScriptThread::showYesNoDialog(QString icon, QString title, QString text, QWidget* parent)
{
    return m_standardDialogs->showYesNoDialog(icon, title, text, (parent == 0) ? m_userInterface[0]->getWidgetPointer() : parent);
}

/**
 * This slot is connected with the MainInterfaceThread::sendingFinishedSignal signal.
 * The main interface thread emits this signal if the sending of data has been finished.
 * @param id
 *      The send id (identifies the sender).
 */
void ScriptThread::sendingFinishedSlot(bool success, uint id)
{
    if(id == m_sendId)
    {
        m_sendingSucceeded = success;
    }
}
#ifdef Q_OS_MAC
/**
* Debug timer slot (checks if the script is suspended by the debugger or is running).
*/
void ScriptThread::debugTimerSlot(void)
{
    static QScriptEngineDebugger::DebuggerState state = QScriptEngineDebugger::SuspendedState;

    if(m_debugger->state() != state)
    {
        state = m_debugger->state();
        if(state == QScriptEngineDebugger::RunningState)
        {//The script is suspended (seems to be a bug on Mac OS X).

            m_pauseTimer->stop();
            m_shallPause = true;
            m_isSuspendedByDebuger = true;

            setThreadState(PAUSED);

            for(auto el : m_userInterface)
            {
                el->setEnabled(false);
            }
            for(auto el : m_allCreatedGuiElementsFromScript)
            {
                el->setEnabled(false);
            }
            emit enableAllTabsForOneScriptThreadSignal(this, false);
            emit pauseAllCreatedInterfaces(true);

            //Block the signals of all timer.
            for(auto child : children())
            {
                if(QString(child->metaObject()->className()) == QString("QTimer"))
                {
                    static_cast<QTimer*>(child)->blockSignals(true);
                }
            }

            blockSignals(true);

        }
        else
        {
            m_shallPause = false;
            m_isSuspendedByDebuger = false;

            if(m_userInterface[0]->getWidgetPointer() != 0)
            {
                bool allWindowsAreClosed = true;

                for(auto el : m_userInterface)
                {
                    if(el->isVisible())
                    {//the user has not closed the dialog
                        allWindowsAreClosed = false;
                        break;
                    }
                }
                if(allWindowsAreClosed  && !m_hasMainWindowGuiElements)
                {
                    if(m_isSuspendedByDebuger)
                    {
                        stopScript();
                    }
                }
            }
            blockSignals(false);

            //Unblock the signals of all timer.
            for(auto child : children())
            {
                if(QString(child->metaObject()->className()) == QString("QTimer"))
                {
                    static_cast<QTimer*>(child)->blockSignals(false);
                }
            }

            emit pauseAllCreatedInterfaces(false);
            for(auto el : m_userInterface)
            {
                el->setEnabled(true);
            }
            for(auto el : m_allCreatedGuiElementsFromScript)
            {
                el->setEnabled(true);
            }
            emit enableAllTabsForOneScriptThreadSignal(this, true);

            QCoreApplication::processEvents();
            setThreadState(RUNNING);
            m_pauseTimer->start(100);

        }
    }

}
#endif

/**
 * This slot is called periodically by the timer m_pauseTimer.
 * This function checks if the thread has to be paused and do the necessary actions.
 */
void ScriptThread::pauseTimerSlot()
{
    if(m_shallPause)
    {
        setThreadState(PAUSED);
        m_pauseTimer->stop();

        for(auto el : m_userInterface)
        {
            el->setEnabled(false);
        }
        for(auto el : m_allCreatedGuiElementsFromScript)
        {
            el->setEnabled(false);
        }
        emit enableAllTabsForOneScriptThreadSignal(this, false);

        emit pauseAllCreatedInterfaces(true);

        //Block the signals of all timer.
        for(auto child : children())
        {
            if(QString(child->metaObject()->className()) == QString("QTimer"))
            {
                static_cast<QTimer*>(child)->blockSignals(true);
            }
        }

        blockSignals(true);


        while(m_shallPause && !m_shallExit)
        {

            QCoreApplication::processEvents(QEventLoop::AllEvents, 100);

            if(m_userInterface[0]->getWidgetPointer() != 0)
            {
                bool allWindowsAreClosed = true;

                for(auto el : m_userInterface)
                {
                    if(el->isVisible())
                    {//the user has not closed the dialog
                        allWindowsAreClosed = false;
                        break;
                    }
                }
                if(allWindowsAreClosed  && !m_hasMainWindowGuiElements)
                {
                    if(m_isSuspendedByDebuger)
                    {
                        stopScript();
                    }
                    break;
                }
            }
        }

        blockSignals(false);

        //Unblock the signals of all timer.
        for(auto child : children())
        {
            if(QString(child->metaObject()->className()) == QString("QTimer"))
            {
                static_cast<QTimer*>(child)->blockSignals(false);
            }
        }

        emit pauseAllCreatedInterfaces(false);
        for(auto el : m_userInterface)
        {
            el->setEnabled(true);
        }
        for(auto el : m_allCreatedGuiElementsFromScript)
        {
            el->setEnabled(true);
        }
        emit enableAllTabsForOneScriptThreadSignal(this, true);

        QCoreApplication::processEvents();
        setThreadState(RUNNING);

        m_pauseTimer->start(100);
    }
}


/**
 * This slot function is called when a script function connected to a signal causes an exception.
 * @param exception
 *      The exception.
 */
void ScriptThread::scriptSignalHandlerSlot(const QScriptValue & exception)
{
    QWidget* parent = (m_scriptWindow->isVisible()) ? static_cast<QWidget *>(m_scriptWindow) : static_cast<QWidget *>(m_scriptWindow->getMainWindow());
    m_scriptFileObject->showExceptionInMessageBox(exception, m_scriptFileName, m_scriptEngine, parent, m_scriptWindow);
    stopScript();
}

/**
 * Loads/includes one script (QtScript has no built in include mechanism).
 * @param scriptPath
 *      The script path.
 * @param isRelativePath
 *      True of scriptPath is a relative path.
 * @return
 *      True for success.
 */
bool ScriptThread::loadScript(QString scriptPath, bool isRelativePath)
{
    QWidget* parent = (m_scriptWindow->isVisible()) ? static_cast<QWidget *>(m_scriptWindow) : static_cast<QWidget *>(m_scriptWindow->getMainWindow());
    bool scriptShallBeStopped = false;
    bool result =  m_scriptFileObject->loadScript(scriptPath, isRelativePath, m_scriptEngine, parent, m_scriptWindow, true, &scriptShallBeStopped);

    if(!result && scriptShallBeStopped)
    {
        stopScript();
    }

    return result;
}

/**
 * The slot is called if the main interface thread has received data.
 * This slot is connected to the MainInterfaceThread::dataReceivedSignal signal.
 * @param data
 *      The received data.
 */
void ScriptThread::canMessagesReceivedSlot(QVector<QByteArray> messages)
{

    if(!messages.isEmpty() &&  (m_state == RUNNING))
    {
        if(QObject::receivers(SIGNAL(canMessagesReceivedSignal(QVector<quint8>, QVector<quint32>, QVector<quint32>,
                                                               QVector<QVector<unsigned char>>))) > 0)
        {
            QVector<quint8> types;
            QVector<quint32> messageIds;
            QVector<quint32> timestamps;
            QVector<QVector<unsigned char>> data;

            for(auto el : messages)
            {
                QVector<unsigned char> dataVector;

                for(auto val : el)
                {
                    dataVector.push_back((unsigned char) val);
                }
                quint8 type = dataVector[0];

                quint32 messageId = (dataVector[1] << 24) + (dataVector[2] << 16) + (dataVector[3] << 8) + (dataVector[4] & 0xff);
                quint32 timeStamp = (dataVector[5] << 24) + (dataVector[6] << 16) + (dataVector[7] << 8) + (dataVector[8] & 0xff);

                types.push_back(type);
                messageIds.push_back(messageId);
                timestamps.push_back(timeStamp);

                //Push the data bytes.
                data.push_back(dataVector.mid(PCANBasicClass::BYTES_FOR_CAN_TYPE + PCANBasicClass::BYTES_FOR_CAN_ID
                                              + PCANBasicClass::BYTES_FOR_CAN_TIMESTAMP));

            }

            emit canMessagesReceivedSignal(types, messageIds, timestamps, data);
        }
    }

}


/**
 * Emits the dataReceivedSignal with the saved received data (if the script is running in the script debugger).
 */
void ScriptThread::debugReceiveTimerSlot(void)
{
    m_debugReceiveTimer.stop();
    emit dataReceivedSignal(m_savedReceivedData);
    m_savedReceivedData.clear();
}

/**
 * Is called, if data from the main interface (MainInterfaceThread) has been received.
 * It converts the received QByteArray into a QVector and emits the dataReceivedSignal.
 * The script can connect to this signal.
 * @param data
 *      The received data.
 */
void ScriptThread::dataReceivedSlot(QByteArray data)
{
    if(m_state == RUNNING)
    {
        if(QObject::receivers(SIGNAL(dataReceivedSignal(QVector<unsigned char>))) > 0)
        {
            QVector<unsigned char> dataVector;

            for(auto val : data)
            {
                dataVector.push_back((unsigned char) val);
            }
            if(m_scriptRunsInDebugger)
            {
                m_savedReceivedData.append(dataVector);
                if(!m_debugReceiveTimer.isActive())
                {
                    m_debugReceiveTimer.start();
                }
            }
            else
            {
                emit dataReceivedSignal(dataVector);
            }
        }
    }

}

/**
 * This slot is connected with MainInterfaceThread::dataConnectionStatusSignal.
 * The connected status (main interface) is reported with this signal.
 * @param isConnected
 *      True for connected.
 * @param message
 *      String with additional information.
 * @param isWaiting
 *      True if the interface is waiting for a client/connection
 */
void ScriptThread::dataConnectionStatusSlot(bool isConnected, QString message, bool isWaiting)
{
    (void)message;
    (void)isWaiting;
    m_isConnected = isConnected;
    m_isConnectedWithCan = m_scriptWindow->m_mainInterfaceThread->isConnectedWithCan();

}


/**
 * Sends a byte array (QByteArray) with the main interface (in MainInterfaceThread).
 * @param byteArray
 *     The byte array.
 * @param repetitionCount
 *     The byte array is repeated until the number has been reached.
 * @param pause
 *     The pause (ms) between two repetitions.
 *  @param addToMainWindowSendHistory
 *      True if the data shall be added to the send histoy in the main window.
 * @return
 *      True for success.
 */
bool ScriptThread::sendByteArray(QByteArray byteArray, int repetitionCount, int pause, bool addToMainWindowSendHistory)
{

    bool hasSucceeded = false;

    for(qint32 i = 0; i <= repetitionCount; i++)
    {
        m_sendingSucceeded = false;

        if(!m_isConnected || m_shallExit)
        {//The main interface is not connected or the script thread shall exit.
            break;
        }

        //send the data.
        emit sendDataSignal(byteArray, m_sendId);
        hasSucceeded =  m_sendingSucceeded;

        if(!hasSucceeded)
        {
            break;
        }

        if(repetitionCount > 0)
        {
            QCoreApplication::processEvents();
            if(m_shallPause)
            {
                pauseTimerSlot();
            }
            msleep(pause);
        }
    }

    if(addToMainWindowSendHistory && hasSucceeded)
    {
        emit addDataToMainWindowSendHistorySignal(byteArray);
    }

    return hasSucceeded;

}

/** Sends a data array (QVector) with the main interface (in MainInterfaceThread).
 *  @param data
 *          The data array.
 *  @param repetitionCount
 *      The data array is repeated until the number has been reached.
 *  @param pause
 *      The pause (ms) between two repetitions.
 *  @param addToMainWindowSendHistory
 *      True if the data shall be added to the send history in the main window.
 * @return
 *      True for success.
 */
bool ScriptThread::sendDataArray(QVector<unsigned char> data, int repetitionCount, int pause, bool addToMainWindowSendHistory)
{
    return sendByteArray(QByteArray(reinterpret_cast<const char*>(data.constData()), data.size()), repetitionCount, pause, addToMainWindowSendHistory);
}

/** Sends a can message with the main interface (in MainInterfaceThread).
 *  @param type
 *          The can message type. Following values are possible:
 *          0: standard can message (11-bit identifier)
 *          1: standard remote-transfer-request message (11-bit identifier)
 *          2: extended can message (29-bit identifier)
 *          3: extended remote-transfer-request message (29-bit identifier)
 *  @param data
 *          The can data.
 *  @param repetitionCount
 *      The can message is repeated until the number has been reached.
 *  @param pause
 *      The pause (ms) between two repetitions.
 *  @param addToMainWindowSendHistory
 *      True if the data shall be added to the send history in the main window.
 * @return
 *      True for success.
 */
bool ScriptThread::sendCanMessage(quint8 type, quint32 canId, QVector<unsigned char> data, int repetitionCount, int pause, bool addToMainWindowSendHistory)
{
    QByteArray byteArray;
    byteArray.push_back(type);

    byteArray.push_back((canId >> 24) & 0xff);
    byteArray.push_back((canId >> 16) & 0xff);
    byteArray.push_back((canId >> 8) & 0xff);
    byteArray.push_back(canId & 0xff);

    byteArray.append(QByteArray(reinterpret_cast<const char*>(data.constData()), data.size()));

    return sendByteArray(byteArray, repetitionCount, pause, addToMainWindowSendHistory);
}

/**
 * Starts the script thread in a debugger;
 */
void ScriptThread::startDebugging()
{
    m_scriptRunsInDebugger = true;
    run();
}

/**
* Terminates the current script thread.
*/
void ScriptThread::terminateScriptThread(void)
{
    m_state = EXITED;
    g_aThreadHasBeenTerminated = true;

    //Close all user interfaces.
    for(auto el : m_userInterface)
    {
        if(el->getWidgetPointer())
        {
            el->close();
        }
    }

    //Close all created gui elements (created by the script).
    for(auto el : m_allCreatedGuiElementsFromScript)
    {
        el->close();
    }

    //Disconnect all signals which are routed to the current script.
    QObject::disconnect(m_scriptWindow->m_mainInterfaceThread, SIGNAL(dataReceivedSignal(QByteArray)),
                    this, SLOT(dataReceivedSlot(QByteArray)));
    QObject::disconnect(m_scriptWindow->m_mainInterfaceThread, SIGNAL(canMessagesReceivedSignal(QVector<QByteArray>)),
                    this, SLOT(canMessagesReceivedSlot(QVector<QByteArray>)));
    QObject::disconnect(m_scriptWindow->m_mainInterfaceThread, SIGNAL(dataConnectionStatusSignal(bool, QString)),
                    this, SLOT(dataConnectionStatusSlot(bool, QString)));
    QObject::disconnect(m_scriptWindow->m_mainInterfaceThread, SIGNAL(sendingFinishedSignal(bool,uint)),
                    this, SLOT(sendingFinishedSlot(bool,uint)));

    terminate();
}

/**
 * Waits until the main interface is connected or a timeout occurs.
 *
 * @param connectTimeout
 *      Connect timeout(ms).
 */
void ScriptThread::waitForMainInterfaceToConnect(quint32 connectTimeout)
{
    QDateTime startTime = QDateTime::currentDateTime();
    do
    {
        msleep(1);
        if(m_shallPause)
        {
            pauseTimerSlot();
        }
        QCoreApplication::processEvents();


    }while ((startTime.msecsTo(QDateTime::currentDateTime()) < (qint64)connectTimeout)  && !m_isConnected  && !m_shallExit);
}

/**
 * Connects the main interface (UDP or TCP socket).
 * Note: A successful call will modify the corresponding settings in the settings dialog.
 *
 * @param isTcp
 *      True for TCP and false for UDP.
 * @param isServer
 *      True if the connection type is a (TCP) server.
 * @param ip
 *      The partner ip address.
 * @param partnerPort
 *      The partner port.
 * @param ownPort
 *      The own port.
 * @param connectTimeout
 *      Connection timeout (ms).
 * @return
 *      True on success.
 */
bool ScriptThread::connectSocket(bool isTcp, bool isServer, QString ip, quint32 destinationPort, quint32 ownPort, quint32 connectTimeout)
{
    bool succeeded = false;

    m_settingsDialog->updateSettings();
    Settings oldSettings = *m_settingsDialog->settings();
    Settings settings = *m_settingsDialog->settings();

    if(isTcp)
    {
        settings.connectionType = isServer ? CONNECTION_TYPE_TCP_SERVER : CONNECTION_TYPE_TCP_CLIENT;
        settings.socketSettings.socketType = isServer ? "Tcp server" : "Tcp client";

    }
    else
    {
        settings.connectionType = CONNECTION_TYPE_UDP_SOCKET;
        settings.socketSettings.socketType = "Udp socket";
    }

    settings.socketSettings.destinationIpAddress = ip;
    settings.socketSettings.ownPort = ownPort;
    settings.socketSettings.destinationPort = destinationPort;
    emit setAllSettingsSignal(settings, false);
    emit connectDataConnectionSignal(settings, true);

    waitForMainInterfaceToConnect(connectTimeout);

    if(!m_isConnected)
    {
        emit setAllSettingsSignal(oldSettings, false);
        emit connectDataConnectionSignal(oldSettings, false);
    }

    succeeded = m_isConnected;
    return succeeded;

}

/**
 * Connects the main interface (PCAN).
 * Note: A successful call will modify the corresponding settings in the settings dialog.
 * @param channel
 *      The PCAN channel.
 * @param baudrate
 *      The baudrate. Possible values are:
 *      1000, 800, 500, 250, 125,100,95,83,50,47,33,20,10,5.
 * @param connectTimeout
 *      Connect timeout(ms)
 * @param busOffAutoReset
 *      True if the PCAN driver shall reset automatically the CAN controller of a PCAN Channel when a bus-off state is detected.
 * @param powerSupply
 *      True if the external 5V on the D-Sub connector shall be switched on.
 * @param filterExtended
 *      True if the filer message type is extended (29-bit identifier) or false if the filter message type
 *      is standard (11-bit identifier).
 * @param filterFrom
 *      The lowest CAN ID to be received.
 * @param filterTo
 *      The highest CAN ID to be received.
 * @return
 *      True on success.
 */
bool ScriptThread::connectPcan(quint8 channel, quint32 baudrate, quint32 connectTimeout, bool busOffAutoReset, bool powerSupply,
                               bool filterExtended, quint32 filterFrom, quint32 filterTo)
{
    bool succeeded = false;

    m_settingsDialog->updateSettings();
    Settings oldSettings = *m_settingsDialog->settings();

    Settings newSettings = *m_settingsDialog->settings();
    newSettings.connectionType = CONNECTION_TYPE_PCAN;
    newSettings.pcanInterface.baudRate = PCANBasicClass::convertBaudrateString(QString("%1").arg(baudrate));
    newSettings.pcanInterface.busOffAutoReset = busOffAutoReset;
    newSettings.pcanInterface.channel = channel;
    newSettings.pcanInterface.powerSupply = powerSupply;
    newSettings.pcanInterface.filterExtended = filterExtended;
    newSettings.pcanInterface.filterFrom = QString::number(filterFrom, 16);
    newSettings.pcanInterface.filterTo = QString::number(filterTo, 16);
    emit setAllSettingsSignal(newSettings, false);
    emit connectDataConnectionSignal(newSettings, true);

    waitForMainInterfaceToConnect(connectTimeout);

    if(!m_isConnected)
    {
        emit setAllSettingsSignal(oldSettings, false);
        emit connectDataConnectionSignal(oldSettings, false);
    }

    succeeded = m_isConnected;
    return succeeded;
}

/**
 * Sets the serial port (main interface) RTS and DTR pins
 * @param setRTS
 *      True if the RTS pin shall be set.
 * @param setDTS
 *      True if the DTS pin shall be set.
 */
void ScriptThread::setSerialPortPins(bool setRTS, bool setDTR)
{
    emit setSerialPortPinsSignal(setRTS, setDTR);
}

/**
 * Connects the main interface (serial port).
 * Note: A successful call will modify the corresponding settings in the settings dialog.
 * @param name
 *      The serial port name.
 * @param baudRate
 *      The baudrate.
 * @param connectTimeout
 *      Connect timeout(ms)
 * @param dataBits
 *      The data bits.
 * @param parity
 *      The parity. Possible values are: "None ", "Even ", "Odd ", "Space" and "Mark".
 * @param stopBits
 *      The number of stop bits. Possible values are: "1 ", "1.5" and "2".
 * @param flowControl
 *      The flow control. Possible values are: "RTS/CTS", "XON/XOFF" and "None".
 * @return
 *      True on success.
 */
bool ScriptThread::connectSerialPort(QString name, qint32 baudRate, quint32 connectTimeout, quint32 dataBits, QString parity, QString stopBits, QString flowControl)
{

    bool succeeded = false;

    m_settingsDialog->updateSettings();
    Settings oldSettings = *m_settingsDialog->settings();
    Settings settings = *m_settingsDialog->settings();
    settings.connectionType = CONNECTION_TYPE_SERIAL_PORT;
    settings.serialPort.name = name;

    settings.serialPort.baudRate = baudRate;
    settings.serialPort.stringBaudRate = QString("%1").arg(baudRate);

    settings.serialPort.stringDataBits = dataBits;
    settings.serialPort.stringDataBits = QString("%1").arg(dataBits);

    settings.serialPort.stringParity = parity;
    if(parity== "None")
    {
        settings.serialPort.parity = QSerialPort::NoParity;
    }
    else if(parity== "Even")
    {
        settings.serialPort.parity = QSerialPort::EvenParity;
    }
    else if(parity== "Odd")
    {
        settings.serialPort.parity = QSerialPort::OddParity;
    }
    else if(parity== "Space")
    {
        settings.serialPort.parity = QSerialPort::SpaceParity;
    }
    else if(parity== "Mark")
    {
        settings.serialPort.parity = QSerialPort::MarkParity;
    }
    else
    {
        settings.serialPort.parity = QSerialPort::UnknownParity;
    }


    settings.serialPort.stringStopBits = stopBits;
    if(stopBits == "1.5")
    {
        settings.serialPort.stopBits = QSerialPort::OneAndHalfStop;
    }
    else if(stopBits == "2")
    {
        settings.serialPort.stopBits = QSerialPort::TwoStop;
    }
    else
    {//1
        settings.serialPort.stopBits = QSerialPort::OneStop;
    }

    settings.serialPort.stringFlowControl = flowControl;
    if(flowControl == "RTS/CTS")
    {
        settings.serialPort.flowControl = QSerialPort::HardwareControl;
    }
    else if(flowControl == "XON/XOFF")
    {
        settings.serialPort.flowControl = QSerialPort::SoftwareControl;
    }
    else
    {//None
        settings.serialPort.flowControl = QSerialPort::NoFlowControl;
    }


    emit setAllSettingsSignal(settings, false);
    emit connectDataConnectionSignal(settings, true);

    waitForMainInterfaceToConnect(connectTimeout);

    if(!m_isConnected)
    {
        emit setAllSettingsSignal(oldSettings, false);
        emit connectDataConnectionSignal(oldSettings, false);
    }

    succeeded = m_isConnected;
    return succeeded;

}

/**
 * Connects the main interface (cheetah spi).
 * Note: A successful call will modify the corresponding settings in the settings dialog.
 * @param port
 *      The cheetah spi port.
 * @param mode
 *      The spi mode (0-3)
 * @param baudrate
 *      The baudrate of the interface (kHz).
 * @param chipSelectBits
 *      The chip select bits (1-7).
 * @param connectTimeout
 *      Connect timeout(ms)
 * @return
 *      True on success.
 */
bool ScriptThread::connectCheetahSpi(quint32 port, qint16 mode, quint32 baudrate, quint8 chipSelectBits, quint32 connectTimeout)
{
    bool succeeded = false;

    m_settingsDialog->updateSettings();
    Settings oldSettings = *m_settingsDialog->settings();
    Settings settings = *m_settingsDialog->settings();
    settings.connectionType = CONNECTION_TYPE_CHEETAH_SPI_MASTER;
    settings.cheetahSpi.port = port;
    settings.cheetahSpi.mode = mode;
    settings.cheetahSpi.baudRate = baudrate;
    settings.cheetahSpi.chipSelect = chipSelectBits;

     emit setAllSettingsSignal(settings, false);
    emit connectDataConnectionSignal(settings, true);

    waitForMainInterfaceToConnect(connectTimeout);

    if(!m_isConnected)
    {
        emit setAllSettingsSignal(oldSettings, false);
        emit connectDataConnectionSignal(oldSettings, false);
    }

    succeeded = m_isConnected;
    return succeeded;
}

/**
 * This function stops the current script thread.
 */
void ScriptThread::stopScript(void)
{

    if(!m_shallExit)
    {
        m_shallExit = true;
        if(!m_scriptRunsInDebugger)
        {
            exit();
        }
    }
}


/**
 * Starts the program program with the arguments arguments in a new process,
 * and detaches from it. Returns true on success, otherwise returns false.
 * If the calling process exits, the detached process will continue to run unaffected.
 * The process will be started in the directory workingDirectory.
 * If workingDirectory is empty, the working directory is inherited from the calling process.
 * @param program
 *      The program to start.
 * @param arguments
 *      The arguments.
 * @param workingDirectory
 *  The working directory.
 * @return
 *  True on success.
 */
bool ScriptThread::createProcessDetached(QString program, QStringList arguments,
                                         QString  workingDirectory)
{
    return QProcess::startDetached(program, arguments, workingDirectory);
}


/**
 * Starts the program program with the arguments arguments in a new process,
 * waits for it to finish, and then returns the exit code of the process.
 * The environment and working directory are inherited from the calling process.
 * @param program
 *      The program to start.
 * @param arguments
 *      The arguments.
 * @return
 */
int ScriptThread::createProcess(QString program, QStringList arguments)
{
    return QProcess::execute(program, arguments);
}

/**
 * Starts the program program with the arguments arguments in a new process.
 * Any data the new process writes to the console is forwarded to the return process object.
 * The environment and working directory are inherited from the calling process.
 * Note: Blocks until the process has been created or until startWaitTime milliseconds have passed (-1=infinite).
 * @param program
 *      The program.
 * @param arguments
 *      The program arguments.
 * @param startWaitTime
 *      The max. wait time.
 * @param workingDirectory
 *      The working directory. If empty then the working directory of the process is set the ScriptCommunicator directory.
 * @return
 *      The created process on success. An invalid value on failure.
 */
QScriptValue ScriptThread::createProcessAsynchronous (QString program, QStringList arguments,
                                                      int startWaitTime, QString workingDirectory)
{
    QScriptValue result;
    QProcess* process = new QProcess(this);
    process->setWorkingDirectory(workingDirectory.isEmpty()? QCoreApplication::applicationDirPath() : workingDirectory);
    process->start(program, arguments);

    if(process->waitForStarted(startWaitTime))
    {
        result = m_scriptEngine->newQObject(static_cast<QObject*>(process), QScriptEngine::ScriptOwnership);
    }

    return result;
}

/**
 * Blocks until the process has finished or until msecs milliseconds have passed (-1=infinite).
 * @param process
 *      The process.
 * @param waitTime
 *      The max. wait time.
 * @return
 *      True if the process is finished.
 */
bool ScriptThread::waitForFinishedProcess(QScriptValue process, int waitTime)
{
    bool result = false;
    if(process.isValid() && process.isQObject())
    {
        QObject* obj = process.toQObject();
        if(QString(obj->metaObject()->className()) == QString("QProcess"))
        {
            QProcess* proc = static_cast<QProcess*>(obj);
            result = proc->waitForFinished(waitTime);
        }
    }

    return result;
}

/**
 * Returns the exit code of process.
 * @param process
 *      The process.
 * @return
 *      The exit code.
 */
int ScriptThread::getProcessExitCode(QScriptValue process)
{
    int result = -1;
    if(process.isValid() && process.isQObject())
    {
        QObject* obj = process.toQObject();
        if(QString(obj->metaObject()->className()) == QString("QProcess"))
        {
            QProcess* proc = static_cast<QProcess*>(obj);
            result = proc->exitCode();
        }
    }

    return result;
}

/**
 * Kills the current process, causing it to exit immediately.
 * @param process
 *      The process.
 */
void ScriptThread::killProcess(QScriptValue process)
{
    if(process.isValid() && process.isQObject())
    {
        QObject* obj = process.toQObject();
        if(QString(obj->metaObject()->className()) == QString("QProcess"))
        {
            QProcess* proc = static_cast<QProcess*>(obj);
            proc->kill();
        }
    }
}

/**
 * Attempts to terminate the process.
 * The process may not exit as a result of calling this function (it is given the chance to prompt
 * the user for any unsaved files, etc).
 * @param process
 *      The process.
 */
void ScriptThread::terminateProcess(QScriptValue process)
{
    if(process.isValid() && process.isQObject())
    {
        QObject* obj = process.toQObject();
        if(QString(obj->metaObject()->className()) == QString("QProcess"))
        {
            QProcess* proc = static_cast<QProcess*>(obj);
            proc->terminate();
        }
    }
}

/**
 * Write data to the standard input of process. Returns true on success.
 * Note: Blocks until the writing is finished or until msecs milliseconds have passed (-1=infinite).
 * @param process
 *      The process.
 * @param data
 *      The data
 * @param waitTime
 *      The wait time.
 * @return
 *      True on sucess.
 */
bool ScriptThread::writeToProcessStdin(QScriptValue process, QVector<unsigned char> data, int waitTime)
{
    bool result = false;
    if(process.isValid() && process.isQObject())
    {
        QObject* obj = process.toQObject();
        if(QString(obj->metaObject()->className()) == QString("QProcess"))
        {
            QProcess* proc = static_cast<QProcess*>(obj);
            if(proc->write((const char *)data.constData(), data.length()) == data.length())
            {
                result = proc->waitForBytesWritten(waitTime);
            }
        }
    }

    return result;
}

/**
 * Returns true if the process is running.
 * @param process
 *      The process.
 */
bool ScriptThread::processIsRunning(QScriptValue process)
{
    bool isRunning = false;

    if(process.isValid() && process.isQObject())
    {
        QObject* obj = process.toQObject();
        if(QString(obj->metaObject()->className()) == QString("QProcess"))
        {
            QProcess* proc = static_cast<QProcess*>(obj);
            isRunning = (proc->state() == QProcess::Running) ? true : false;
        }
    }
    return isRunning;

}
/**
 * This function returns all data available from the standard output of process (can be called after the process is finished).
 * Note: If isBlocking is true then this function blocks until the blockByte has been received,
 * blockTime has elapsed (-1=infinite) or the process is finished.
 * @param process
 *      The process.
 * @param isBlocking
 * 		True if this function is blocking.
 * @param blockByte
 * 		The block byte.
 * @param blockTime
 *		The max. block time.
 * @return
 *      The data.
 */
QVector<unsigned char>  ScriptThread::readAllStandardOutputFromProcess(QScriptValue process, bool isBlocking,
                                                                       quint8 blockByte, qint32 blockTime)
{
    QVector<unsigned char> result;
    if(process.isValid() && process.isQObject())
    {
        QObject* obj = process.toQObject();
        if(QString(obj->metaObject()->className()) == QString("QProcess"))
        {
            QProcess* proc = static_cast<QProcess*>(obj);
            QByteArray byteArray;

            if(isBlocking)
            {
                QDateTime startTime = QDateTime::currentDateTime();

                while(!byteArray.contains(blockByte) && (startTime.msecsTo(QDateTime::currentDateTime()) < blockTime))
                {
                    proc->waitForReadyRead(1);
                    byteArray.append(proc->readAllStandardOutput());

                    if(proc->state() != QProcess::Running)
                    {
                        break;
                    }
                }

            }
            else
            {
                byteArray = proc->readAllStandardOutput();
            }

            for(auto val : byteArray)
            {
                result.push_back((unsigned char) val);
            }
        }
    }
    return result;
}

/**
 * This function returns all data available from the standard error of process (can be called after the process is finished).
 * Note: If isBlocking is true then this function blocks until the blockByte has been received or
 * blockTime has elapsed (-1=infinite) or the process is finished.
 * @param process
 *      The process.
 * @param isBlocking
 * 		True if this function is blocking.
 * @param blockByte
 * 		The block byte.
 * @param blockTime
 *		The max. block time.
 * @return
 *      The data.
 */
QVector<unsigned char>  ScriptThread::readAllStandardErrorFromProcess(QScriptValue process, bool isBlocking,
                                                                      quint8 blockByte, qint32 blockTime)
{
    QVector<unsigned char> result;
    if(process.isValid() && process.isQObject())
    {
        QObject* obj = process.toQObject();
        if(QString(obj->metaObject()->className()) == QString("QProcess"))
        {
            QProcess* proc = static_cast<QProcess*>(obj);
            QByteArray byteArray;

            if(isBlocking)
            {
                QDateTime startTime = QDateTime::currentDateTime();

                while(!byteArray.contains(blockByte) && (startTime.msecsTo(QDateTime::currentDateTime()) < blockTime))
                {
                    proc->waitForReadyRead(1);
                    byteArray.append(proc->readAllStandardError());
                    if(proc->state() != QProcess::Running)
                    {
                        break;
                    }

                }

            }
            else
            {
                byteArray = proc->readAllStandardError();
            }

            for(auto val : byteArray)
            {
                result.push_back((unsigned char) val);
            }
        }
    }
    return result;
}

/** Sends a string (QString) with the main interface (in MainInterfaceThread)).
 *  @param string
 *      The string.
 *  @param repetitionCount
 *      The data array is repeated until the number has been reached.
 *  @param pause
 *      The pause (ms) between two repetitions.
 *  @param addToMainWindowSendHistory
 *      True if the data shall be added to the send history in the main window.
 * @return
 *      True for success.
 */
bool ScriptThread::sendString(QString string, int repetitionCount, int pause, bool addToMainWindowSendHistory)
{
    return sendByteArray(string.toLocal8Bit(), repetitionCount, pause, addToMainWindowSendHistory);
}

/**
 * Loads a dynamic link library and calls the init function (void init(QScriptEngine* engine)).
 * With this function a script can extend his functionality.
 *
 * The library can register functions/objects which can be used/called from the script and the library
 * can call all script functions.
 * @param path
 *      The library path.
 * @param isRelativePath
 *      True if the library path is a relative path.
 * @return
 *      True on success.
 */
bool ScriptThread::loadLibrary(QString path, bool isRelativePath)
{
    bool result = false;

    try
    {
        path = isRelativePath ? createAbsolutePath(path) : path;

        QLibrary* lib = new QLibrary(path);
        if(lib->load())
        {

            typedef void (*initFunction)(QScriptEngine*);

            initFunction func = (initFunction) lib->resolve("init");
            if (func)
            {
                func(m_scriptEngine);
                result = true;
                m_libraries.append(lib);
            }
            else
            {
                lib->deleteLater();
                appendTextToConsole(QString("could not init function in library: %1").arg(path));
            }
        }
        else
        {
            lib->deleteLater();
            appendTextToConsole(QString("could not load library: %1").arg(path));
        }

    }
    catch(...)
    {
        appendTextToConsole(QString("could not load library: %1").arg(path));
    }

    return result;
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
QString ScriptThread::showTextInputDialog(QString title, QString label, QString displayedText, QWidget* parent)
{
    return m_standardDialogs->showTextInputDialog(title, label, displayedText, (parent == 0) ? m_userInterface[0]->getWidgetPointer() : parent);
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
 * @param parent
 *      The parent of the dialog.
 * @return
 *      The text in the input section after closing the dialog (empty if the ok button was not pressed).
 */
QString ScriptThread::showMultiLineTextInputDialog(QString title, QString label, QString displayedText, QWidget* parent)
{
    return m_standardDialogs->showMultiLineTextInputDialog(title, label, displayedText, (parent == 0) ? m_userInterface[0]->getWidgetPointer() : parent);
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
 * @param parent
 *      The parent of the dialog.
 * @return
 *      The text of the selected item after closing the dialog (empty if the ok button was not pressed).
 */
QString ScriptThread::showGetItemDialog(QString title, QString label, QStringList displayedItems,
                           int currentItemIndex, bool editable, QWidget* parent)
{
    return m_standardDialogs->showGetItemDialog(title, label, displayedItems, currentItemIndex, editable, (parent == 0) ? m_userInterface[0]->getWidgetPointer() : parent);
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
 * @param parent
 *      The parent of the dialog.
 * @result
 *      item 0: 1 if the ok button has been pressed, 0 otherwise
 *      item 1: The value of the spinbox after closing the dialog.
 */
QList<int> ScriptThread::showGetIntDialog(QString title, QString label, int initialValue, int min, int max, int step, QWidget* parent)
{
    return m_standardDialogs->showGetIntDialog(title, label, initialValue, min, max, step, (parent == 0) ? m_userInterface[0]->getWidgetPointer() : parent);
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
 * @param parent
 *      The parent of the dialog.
 * @result
 *      item 0: 1.0 if the ok button has been pressed, 0 otherwise
 *      item 1: The value of the spinbox after closing the dialog.
 */
QList<double> ScriptThread::showGetDoubleDialog(QString title, QString label, double initialValue, double min, double max, int decimals, QWidget* parent)
{
    return m_standardDialogs->showGetDoubleDialog(title, label, initialValue, min, max, decimals, (parent == 0) ? m_userInterface[0]->getWidgetPointer() : parent);
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
QList<int> ScriptThread::showColorDialog(quint8 initInitalRed, quint8 initInitalGreen, quint8 initInitalBlue, quint8 initInitalAlpha, bool alphaIsEnabled, QWidget* parent)
{
    return m_standardDialogs->showColorDialog(initInitalRed, initInitalGreen, initInitalBlue, initInitalAlpha, alphaIsEnabled, (parent == 0) ? m_userInterface[0]->getWidgetPointer() : parent);
}

/**
 * Scripts can switch on/off the adding of received data in the consoles (for fast data transfers).
 * @param show
 *      True=show received data.
 * @return
 *      The old value (on/off).
 */
bool ScriptThread::showReceivedDataInConsoles(bool show)
{
    m_settingsDialog->updateSettings();
    Settings settings = *m_settingsDialog->settings();
    bool oldValue = settings.showReceivedDataInConsole;
    settings.showReceivedDataInConsole = show;
    emit setAllSettingsSignal(settings, false);

    return oldValue;
}

/**
 * Scripts can switch on/off the adding of transmitted data in the consoles (for fast data transfers).
 * @param show
 *      True=show transmitted data.
 * @return
 *      The old value (on/off).
 */
bool ScriptThread::showTransmitDataInConsoles(bool show)
{
    m_settingsDialog->updateSettings();
    Settings settings = *m_settingsDialog->settings();
    bool oldValue = settings.showSendDataInConsole;
    settings.showSendDataInConsole = show;
    emit setAllSettingsSignal(settings, false);

    return oldValue;
}



///Sets a string in the global string map.
///(Scripts can exchange data with this map)
void ScriptThread::setGlobalString(QString name, QString string)
{
    g_stringMapMutex.lock();
    g_stringMap[name] = string;
    g_stringMapMutex.unlock();

    emit m_scriptWindow->globalStringChangedSignal(&name, &string);
}

///Returns a string from the global string map.
///(Scripts can exchange data with this map)
///Note: Returns an empty string if name is not in the map.
QString ScriptThread::getGlobalString(QString name, bool removeValue)
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

///Sets a data vector in the global data vector map.
///(Scripts can exchange data with this map)
void ScriptThread::setGlobalDataArray(QString name, QVector<unsigned char> data)
{
    g_dataMapMutex.lock();
    g_dataMap[name] = data;
    g_dataMapMutex.unlock();

    emit m_scriptWindow->globalDataArrayChangedSignal(&name, &data);
}

///Returns a data vector from the global data vector map.
///(Scripts can exchange data with this map)
///Note: Returns an empty data vector if name is not in the map.
QVector<unsigned char> ScriptThread::getGlobalDataArray(QString name, bool removeValue)
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

///Sets a unsigned number in the global unsigned number map.
///(Scripts can exchange data with this map)
void ScriptThread::setGlobalUnsignedNumber(QString name, quint32 number)
{
    g_unsignedNumberMapMutex.lock();
    g_unsignedNumberMap[name] = number;
    g_unsignedNumberMapMutex.unlock();

    emit m_scriptWindow->globalUnsignedChangedSignal(&name, number);
}

///Returns a unsigned number from the global unsigned number map.
///(Scripts can exchange data with this map)
///Returns an quint32 array.
///The first element is the result status (1=name found, 0=name not found)
///The second element is the read value.
QList<quint32> ScriptThread::getGlobalUnsignedNumber(QString name, bool removeValue)
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

///Sets a signed number in the global signed number map.
///(Scripts can exchange data with this map)
void ScriptThread::setGlobalSignedNumber(QString name, qint32 number)
{
    g_signedNumberMapMutex.lock();
    g_signedNumberMap[name] = number;
    g_signedNumberMapMutex.unlock();

    emit m_scriptWindow->globalSignedChangedSignal(&name, number);
}

///Returns a signed number from the global signed number map.
///(Scripts can exchange data with this map)
///The first element is the result status (1=name found, 0=name not found)
///The second element is the read value.
QList<qint32> ScriptThread::getGlobalSignedNumber(QString name, bool removeValue)
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

///Sets a real number in the global real number map.
///(Scripts can exchange data with this map)
void ScriptThread::setGlobalRealNumber(QString name, double number)
{
    g_realNumberMapMutex.lock();
    g_realNumberMap[name] = number;
    g_realNumberMapMutex.unlock();

    emit m_scriptWindow->globalRealChangedSignal(&name, number);
}

///Returns a real number from the global real number map.
///(Scripts can exchange data with this map)
///The first element is the result status (1=name found, 0=name not found)
///The second element is the read value.
QList<double> ScriptThread::getGlobalRealNumber(QString name, bool removeValue)
{
    QList<double> result;
    g_realNumberMapMutex.lock();

    if(g_realNumberMap.contains(name))
    {
        result.append(1.0);
        result.append(g_realNumberMap[name]);

        if(removeValue)
        {
            g_realNumberMap.remove(name);
        }
    }
    else
    {
        result.append(0.0);
        result.append(0.0);
    }
    g_realNumberMapMutex.unlock();
    return result;
}


/**
 * Returns all IP addresses found on the host machine.
 * @return
 *  The ip addresses.
 */
QStringList ScriptThread::getLocalIpAdress(void)
{
    QList<QHostAddress> ipList = QNetworkInterface::allAddresses();
    QStringList returnList;

    for (int i = 0; i < ipList.size(); ++i)
    {
         returnList << ipList.at(i).toString();
    }
    return returnList;

}
/**
 * Loads an user interface file.
 * Note: If an user interface was already loaded then the old user interface will be unloaded.
 * @param path
 *      The user interface file path.
 * @param isRelativePath
 *      True if the file path is a relative path.
 * @param showAfterLoading
 *      True if the first element of the user interface (normally a window) shall be shown.
 * @return
 *      True on success.
 */
bool ScriptThread::loadUserInterfaceFile(QString path, bool isRelativePath, bool showAfterLoading)
{
    bool result = false;

    result = true;

    path = isRelativePath ? createAbsolutePath(path) : path;
    QWidget* ui = 0;

    emit loadUserInterfaceFileSignal(&ui, path);

    if(ui != 0)
    {
        ScriptWidget* newElement = new ScriptWidget(ui, this, m_scriptWindow);

        if(m_userInterface[0]->getWidgetPointer()== 0)
        {
            m_userInterface[0]->deleteLater();
            m_userInterface[0] = newElement;

        }
        else
        {
            m_userInterface.push_back(newElement);
        }

        //install all elements from the script user interface
        installAllChilds(ui, m_scriptEngine, true);

        if(showAfterLoading)
        {
            m_userInterface[m_userInterface.size() - 1]->show();
        }
        result = true;
    }

    return result;

}

/**
 * Sets the current thread state.
 * @param state
 *      The thread state.
 */
void ScriptThread::setThreadState(ThreadSate state)
{
    m_state = state;
    if(m_isSuspendedByDebuger)
    {
        m_state = SUSPENDED_BY_DEBUGGER;
    }
    emit threadStateChangedSignal(m_state, this);
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
 * @return
 *      True if scriptTableEntryName has been found in the script-table and the state has a valid value.
 *
 */
bool ScriptThread::setScriptState(quint8 state, QString scriptTableEntryName)
{
    bool result;
    emit setScriptStateSignal(state, scriptTableEntryName, &result);
    return result;
}

/**
 * Returns the script-table (script window) name of the calling script.
 * @return
 *      The name.
 */
QString ScriptThread::getScriptTableName(void)
{
    QString scriptName;
    emit getScriptTableNameSignal(&scriptName);
    return scriptName;
}

/**
 * Returns a list with the name of all available serial ports.
 * @return
 *      The list.
 */
QStringList ScriptThread::availableSerialPorts(void)
{
    QStringList result;
    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
    {
        result << info.portName();
    }
    return result;
}

/**
 * Returns the script arguments (command-line argument -A).
 * @return
 *      The arguments.
 */
QStringList ScriptThread::getScriptArguments(void)
{
    return m_scriptWindow->getMainWindow()->getScriptArguments();
}

/**
 * Adds scrip tabs to the main window.
 * Note: This function fails in command-line mode.
 *
 * @param tabWidget
 *       The script tab widget.
 * @return
 *      True on success.
 */
bool ScriptThread::addTabsToMainWindow(ScriptTabWidget* tabWidget)
{
    bool result = false;

    if(!m_scriptWindow->getMainWindow()->isCommandLineMode())
    {//No command-line mode.
        QTabWidget* tabs = static_cast<QTabWidget*> (tabWidget->getWidgetPointer());
        m_hasMainWindowGuiElements = true;
        emit addTabsToMainWindowSignal(tabs);
        result = true;
    }
    return result;
}


/**
 * Adds script toolbox pages to the main window (all pages are removed from toolBox).
 * Note: This function fails in command-line mode.
 *
 * @param scriptToolBox
 *      The script tool box.
 * @return
 *      True on success.
 */
bool ScriptThread::addToolBoxPagesToMainWindow(ScriptToolBox* scriptToolBox)
{
    bool result = false;

    if(!m_scriptWindow->getMainWindow()->isCommandLineMode())
    {//No command-line mode.
        QToolBox* toolBox = static_cast<QToolBox*> (scriptToolBox->getWidgetPointer());
        m_hasMainWindowGuiElements = true;
        emit addToolBoxPagesToMainWindowSignal(toolBox);
        result = true;
    }
    return result;
}

/**
 * Sends received data (received with an script internal interface) to the main interface.
 * This data will be shown as received data in the consoles, the log and will be received by
 * worker scripts via the dataReceivedSignal.
 * @param data
 *      The data.
 */
void ScriptThread::sendReceivedDataToMainInterface(QVector<unsigned char> data)
{
    QByteArray array = QByteArray(reinterpret_cast<const char*>(data.constData()), data.length());
    m_scriptWindow->m_mainInterfaceThread->dataReceived(array);
}

/**
 * Returns the console settings (settings dialog).
 *      The console settings.
 */
QScriptValue ScriptThread::getConsoleSettings(void)
{
    const Settings* settings = m_settingsDialog->settings();
    QScriptValue ret = m_scriptEngine->newObject();

    ret.setProperty("showReceivedData", settings->showReceivedDataInConsole);
    ret.setProperty("showSendData", settings->showSendDataInConsole);
    ret.setProperty("maxChars", settings->maxCharsInConsole);
    ret.setProperty("lockScrolling", settings->lockScrollingInConsole);
    ret.setProperty("font", settings->stringConsoleFont);
    ret.setProperty("fontSize", settings->stringConsoleFontSize);
    ret.setProperty("updateInterval", settings->updateIntervalConsole);
    ret.setProperty("receiveColor", settings->consoleReceiveColor);
    ret.setProperty("sendColor", settings->consoleSendColor);
    ret.setProperty("backgroundColor", settings->consoleBackgroundColor);
    ret.setProperty("timestampColor", settings->consoleMessageAndTimestampColor);
    ret.setProperty("newLineAfterBytes", settings->consoleNewLineAfterBytes);
    ret.setProperty("newLineAfterPause", settings->consoleNewLineAfterPause);
    ret.setProperty("createNewLineAtByte", (settings->consoleNewLineAt != 0xffff) ? true : false);
    ret.setProperty("newLineAtByte", (quint8)settings->consoleNewLineAt);
    ret.setProperty("ceateTimestampAtByte", (settings->consoleCreateTimestampAt != 0) ? true : false);
    ret.setProperty("timestampAtByte", (quint8)settings->consoleCreateTimestampAt);
	ret.setProperty("generateCyclicTimeStamps", settings->generateTimeStampsInConsole);
    ret.setProperty("timeStampInterval", settings->timeStampIntervalConsole);
	ret.setProperty("timestampFormat", settings->consoleTimestampFormat);
    return ret;
}


/**
 * Sets the main window and the ScriptCommunicator task bar icon.
 * Supported formats: .ico, .gif, .png, .jpeg, .tiff, .bmp, .icns.
 * @param iconFile
 *      The file name of the icon.
 * @param isRelativePath
 *      True of path is a relative path.
 */
void ScriptThread::setMainWindowAndTaskBarIcon(QString iconFile, bool isRelativePath)
{
    iconFile = isRelativePath ? m_scriptFileObject->createAbsolutePath(iconFile) : iconFile;
    emit setMainWindowAndTaskBarIconSignal(iconFile);
}

/**
 * @brief Returns and all functions, signals and properties of an object.
 * @param object
 *      The object.
 * @param resultList
 *      All functions and properties of the object in a string list. If not needed then a 0 can be given.
 * @param resultString
 *      All functions and properties of the object in a string (separated by \n). If not needed then a 0 can be given.
 */
void ScriptThread::getAllObjectPropertiesAndFunctionsInternal(QScriptValue object, QStringList* resultList, QString* resultString)
{

    QObject* objPointer = object.toQObject();
    QVariant elements;

    if(objPointer)
    {
        elements = objPointer->property("publicScriptElements");
    }
    if(elements.isValid())
    {
        for(auto el : elements.toString().split(";"))
        {
            if(resultString)
            {
                (*resultString) += el + "\n";
            }

            if(resultList)
            {
                resultList->append(el);
            }
        }
    }
    else
    {

        if(resultString)
        {
            resultString->clear();
        }

        if(resultList)
        {
            resultList->clear();
        }
    }
}

/**
 * Returns and prints (if printInScriptWindowConsole is true) all functions, signals and properties of an object in the script window console.
 * Note: Only ScriptCommunicator classes are supported. Calling this function with a QtScript built-in class (e.g. Array) will result
 * in an empty list.
 *
 * @param object
 *      The object.
 * @param printInScriptWindowConsole
 *      True if the result shall be printed in the script window console.
 * @return
 *      All functions and properties of the object.
 */
QStringList ScriptThread::getAllObjectPropertiesAndFunctions(QScriptValue object, bool printInScriptWindowConsole)
{
    QString resultString;
    QStringList resultList;

    getAllObjectPropertiesAndFunctionsInternal(object, &resultList, printInScriptWindowConsole ? &resultString :  0);

    if(printInScriptWindowConsole && !resultString.isEmpty())
    {
        emit appendTextToConsoleSignal(resultString, true, false);
    }
    return resultList;
}

/**
 * Returns the serial port settings of the main interface.
 * @return
 *      The serial port settings.
 */
QScriptValue ScriptThread::getMainInterfaceSerialPortSettings(void)
{
    const Settings* settings = m_settingsDialog->settings();
    QScriptValue ret = m_scriptEngine->newObject();

    ret.setProperty("name", settings->serialPort.name);
    ret.setProperty("baudRate", settings->serialPort.baudRate);
    ret.setProperty("dataBits", settings->serialPort.dataBits);
    ret.setProperty("parity", settings->serialPort.stringParity);
    ret.setProperty("stopBits", settings->serialPort.stringStopBits);
    ret.setProperty("flowControl", settings->serialPort.stringFlowControl);
    ret.setProperty("rts", settings->serialPort.setRTS);
    ret.setProperty("dtr", settings->serialPort.setDTR);
    return ret;
}

/**
 * Returns the socket (UDP, TCP client/server) settings of the main interface.
 * @return
 *      The socket settings.
 */
QScriptValue ScriptThread::getMainInterfaceSocketSettings(void)
{
    const Settings* settings = m_settingsDialog->settings();
    QScriptValue ret = m_scriptEngine->newObject();

    ret.setProperty("destinationPort", settings->socketSettings.destinationPort);
    ret.setProperty("destinationIpAddress", settings->socketSettings.destinationIpAddress);
    ret.setProperty("ownPort", settings->socketSettings.ownPort);
    ret.setProperty("socketType", settings->socketSettings.socketType);
    ret.setProperty("proxySettings", settings->socketSettings.proxySettings);
    ret.setProperty("proxyIpAddress", settings->socketSettings.proxyIpAddress);
    ret.setProperty("proxyPort", settings->socketSettings.proxyPort);
    ret.setProperty("proxyUserName", settings->socketSettings.proxyUserName);
    ret.setProperty("proxyPassword", settings->socketSettings.proxyPassword);
    return ret;
}

/**
 * Returns the title of the main window.
 */
QString ScriptThread::getMainWindowTitle(void)
{
    return m_scriptWindow->getMainWindow()->windowTitle();
}
/**
 * Checks if the version of ScriptCommunicator is equal/greater then the version in minVersion.
 * The format of minVersion is: 'major'.'minor' (e.g. 04.11).
 * @param minVersion
 *      The minimum version.
 * @return
 *      True if equal or greater.
 */
bool ScriptThread::checkScriptCommunicatorVersion(QString minVersion)
{
    bool result = false;

    if(!minVersion.isEmpty())
    {
        QStringList tmpList = MainWindow::VERSION.split(".");
        quint32 currentMajor = tmpList[0].toUInt();
        quint32 currentMinor = tmpList[1].toUInt();

        tmpList = minVersion.split(".");
        if(tmpList.length() == 2)
        {
            quint32 neededMajor = tmpList[0].toUInt();
            quint32 neededMinor = tmpList[1].toUInt();

            if(neededMajor < currentMajor)
            {
                result = true;
            }
            else if(neededMajor == currentMajor)
            {
                if(neededMinor <= currentMinor)
                {
                    result = true;
                }
            }
        }
    }

    return result;
}

/**
 * Sends the send data from the main interface.
 * @param data
 *      The data.
 */
void ScriptThread::sendDataFromMainInterfaceSlot(const QByteArray data)
{

    if(m_state == RUNNING)
    {
        if(QObject::receivers(SIGNAL(sendDataFromMainInterfaceSignal(QVector<unsigned char>))) > 0)
        {
            QVector<unsigned char> dataVector;

            for(auto val : data)
            {
                dataVector.push_back((unsigned char) val);
            }
            emit sendDataFromMainInterfaceSignal(dataVector);
        }
    }
}

/**
 * Converts a string into a QMessageBox::Icon.
 * @param icon
 *      The icon if the message box. Possible values are:
 *      - Information
 *      - Warning
 *      - Critical
 * @return
 *      The icon.
 */
QMessageBox::Icon ScriptThread::stringToMessageBoxIcon(QString icon)
{
    QMessageBox::Icon retIcon = QMessageBox::NoIcon;

    if(icon == "Information")
    {
        retIcon = QMessageBox::Information;
    }
    else if(icon == "Warning")
    {
        retIcon = QMessageBox::Warning;
    }
    else if(icon == "Critical")
    {
        retIcon = QMessageBox::Critical;
    }
    else if(icon == "Question")
    {
        retIcon = QMessageBox::Question;
    }
    else
    {
        retIcon = QMessageBox::NoIcon;
    }

    return retIcon;
}

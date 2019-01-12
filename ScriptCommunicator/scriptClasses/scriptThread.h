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

#ifndef SCRIPTTHREAD_H
#define SCRIPTTHREAD_H

#include "QNetworkInterface"
#include <QThread>
#include <QMessageBox>
#include <QScriptEngine>
#include "settingsdialog.h"
#include "mainwindow.h"
#include "scriptsqldatabase.h"
#include "crc.h"
#include "scriptStandardDialogs.h"
#include "scriptFile.h"
#include <QFileInfo>
#include <QScriptEngineDebugger>
#include <QProcess>
#include <scriptHelper.h>
#include <QStandardPaths>
#include <QToolBox>
#include "scriptObject.h"
#include <QLibrary>
#include "scriptConverter.h"
#include "aardvarkI2cSpi.h"
#include "scriptInf.h"


class ScriptWidget;
class ScriptPlotWindow;
class ScriptTreeWidgetItem;
class ScriptPcan;
class ScriptWindow;
class ScriptTabWidget;
class ScriptToolBox;


typedef QObject* (*CreateScriptCommunicatorWidget)(QObject *parent, QWidget* uiElement, bool scriptRunsInDebugger);
typedef const char* (*GetScriptCommunicatorWidgetName)(void);

///Enumeration for which represents a thread state.
typedef enum
{
    INVALID,
    CREATED,
    RUNNING,
    PAUSED,
    EXITED,
    SUSPENDED_BY_DEBUGGER

}ThreadSate;
class ScriptThread;

///The class executes the scripts.
class ScriptThread : public QThread, public ScriptObject
{
    Q_OBJECT
    friend class ScriptWindow;

    ///Returns a semicolon separated list with all public functions, signals and properties.
    Q_PROPERTY(QString publicScriptElements READ getPublicScriptElements)

public:
    ScriptThread(ScriptWindow* scriptWindow, quint32 sendId, QString scriptName, QWidget *scriptUi, SettingsDialog *settingsDialog,
                 bool scriptRunsInDebugger);

    virtual ~ScriptThread();

    ///Returns a semicolon separated list with all public functions, signals and properties.
    virtual QString getPublicScriptElements(void){return MainWindow::parseApiFile("scriptThread.api");}

    ///Installs obj and all child objects from obj. This objects can be accessed from the script.
    void installAllChilds(QObject* obj, QScriptEngine* scriptEngine, bool firstObj = false);

    ///This function installs one (child) object. This object can be accessed from the script.
    void installOneChild(QObject* child, QScriptEngine* scriptEngine);

    ///Installs one custom widget.
    void installsCustomWidget(QObject* child, QScriptEngine* scriptEngine);

    ///Appends text to the script window console.
    Q_INVOKABLE void appendTextToConsole(QString string, bool newLine=true, bool bringToForeground=false){ emit appendTextToConsoleSignal(string, newLine,bringToForeground);}

    /****************Deprecated conversion functions (replaced by the conv object)******************************************************/
    ///Converts a byte array which contains ascii characters into a ascii string (QString).
    Q_INVOKABLE QString byteArrayToString(QVector<unsigned char> data){return ScriptConverter::byteArrayToString(data);}

    ///Converts a byte array into a hex string.
    Q_INVOKABLE QString byteArrayToHexString(QVector<unsigned char> data){return ScriptConverter::byteArrayToHexString(data);}

    ///Converts an ascii string into a byte array.
    Q_INVOKABLE QVector<unsigned char> stringToArray(QString str){return ScriptConverter::stringToArray(str);}

    ///Adds an ascii string to a byte array.
    Q_INVOKABLE QVector<unsigned char> addStringToArray(QVector<unsigned char> array, QString str){return ScriptConverter::addStringToArray(array, str);}
    /*************************************************************************************************************************/


    /****************Deprecated file system functions (replaced by the scriptFile object)******************************************************/
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
        QFileInfo fi(m_scriptFileName);
        return fi.absolutePath();
    }

    ///Zips a directory.
    Q_INVOKABLE bool zipDirectory(QString fileName, QString sourceDirName, QString comment="")
    {return m_scriptFileObject->zipDirectory(fileName, sourceDirName, 0, comment);}

    ///Adds files to a zip file.
    Q_INVOKABLE bool zipFiles(QString fileName, QVariantList fileList, QString comment="")
    { return m_scriptFileObject->zipFiles(fileName, fileList, comment);}

    ///Extracts a zip file.
    Q_INVOKABLE bool extractZipFile(QString fileName, QString destinationDirectory)
    {return m_scriptFileObject->extractZipFile(fileName, destinationDirectory);}


    /*************************************************************************************************************************/


    /****************Deprecated main interface functions (replaced by the scriptInf object)******************************************************/


    ///Creates an UDP socket.
    Q_INVOKABLE QScriptValue createUdpSocket(void){return m_scriptInf->createUdpSocket();}

    ///Creates a TCP server.
    Q_INVOKABLE QScriptValue createTcpServer(void){return m_scriptInf->createTcpServer();}

    ///Creates a TCP socket.
    Q_INVOKABLE QScriptValue createTcpClient(void){return m_scriptInf->createTcpClient();}

    ///Creates a serial port.
    Q_INVOKABLE QScriptValue createSerialPort(void){return m_scriptInf->createSerialPort();}

    ///Sends a data array (QVector) with the main interface (in MainInterfaceThread).
    Q_INVOKABLE bool sendDataArray(QVector<unsigned char> data, int repetitionCount=0, int pause=0, bool addToMainWindowSendHistory=false)
    {return m_scriptInf->sendDataArray(data, repetitionCount, pause, addToMainWindowSendHistory);}

    ///Sends a string (QString) with the main interface (in MainInterfaceThread).
    Q_INVOKABLE bool sendString(QString string, int repetitionCount=0, int pause=0, bool addToMainWindowSendHistory=false)
    {return m_scriptInf->sendString(string, repetitionCount, pause, addToMainWindowSendHistory);}

    ///Returns true if the main interface is connected.
    Q_INVOKABLE bool isConnected(void){return m_scriptInf->isConnected();}

    ///Disconnects the main interface.
    Q_INVOKABLE void disconnect(void){m_scriptInf->disconnect();}

    ///Sets the serial port (main interface) RTS and DTR pins.
    Q_INVOKABLE void setSerialPortPins(bool setRTS, bool setDTR){m_scriptInf->setSerialPortPins(setRTS, setDTR);}

    ///Returns the state of the serial port signals (pins).
    ///The signals are bit coded:
    ///NoSignal = 0x00,
    ///DataTerminalReadySignal = 0x04,
    ///DataCarrierDetectSignal = 0x08,
    ///DataSetReadySignal = 0x10,
    ///RingIndicatorSignal = 0x20,
    ///RequestToSendSignal = 0x40,
    ///ClearToSendSignal = 0x80,
    Q_INVOKABLE quint32 getSerialPortSignals(void){return m_scriptInf->getSerialPortSignals();}

    ///Connects the main interface (serial port).
    ///Note: A successful call will modify the corresponding settings in the settings dialog.
    Q_INVOKABLE bool connectSerialPort(QString name, qint32 baudRate = 115200, quint32 connectTimeout= 1000, quint32 dataBits = 8, QString parity = "None",
                                       QString stopBits = "1", QString flowControl = "None")
    {return m_scriptInf->connectSerialPort(name, baudRate, connectTimeout, dataBits, parity, stopBits, flowControl);}

    ///Connects the main interface (UDP or TCP socket).
    ///Note: A successful call will modify the corresponding settings in the settings dialog.
    Q_INVOKABLE bool connectSocket(bool isTcp, bool isServer, QString ip, quint32 destinationPort, quint32 ownPort, quint32 connectTimeout = 5000)
    {return m_scriptInf->connectSocket(isTcp, isServer, ip, destinationPort, ownPort, connectTimeout);}

    ///Returns the serial port settings of the main interface.
    Q_INVOKABLE QScriptValue getMainInterfaceSerialPortSettings(void){return m_scriptInf->getMainInterfaceSerialPortSettings();}

    ///Returns the socket (UDP, TCP client/server) settings of the main interface.
    Q_INVOKABLE QScriptValue getMainInterfaceSocketSettings(void){return m_scriptInf->getMainInterfaceSocketSettings();}

    ///Returns a list with the name of all available serial ports.
    Q_INVOKABLE QStringList availableSerialPorts(void){return m_scriptInf->availableSerialPorts();}

    ///Returns all IP addresses found on the host machine.
    Q_INVOKABLE QStringList getLocalIpAdress(void){return m_scriptInf->getLocalIpAdress();}

    /*************************************************************************************************************************/

    ///Forces the script thread to sleep for ms milliseconds.
    Q_INVOKABLE void sleepFromScript(quint32 timeMs);

    ///Creates a timer.
    Q_INVOKABLE QScriptValue createTimer(void);

    ///Creates a ScriptSound object.
    Q_INVOKABLE QScriptValue createSoundObject(QString filename, bool isRelativePath=true);

    ///Creates a plot window.
    Q_INVOKABLE QScriptValue createPlotWindow();

    ///Creates a XML reader.
    Q_INVOKABLE QScriptValue createXmlReader();

    ///Creates a XML writer.
    Q_INVOKABLE QScriptValue createXmlWriter();

    ///Loads/includes one script (QtScript has no built in include mechanism).
    Q_INVOKABLE bool loadScript(QString scriptPath, bool isRelativePath=true);

    ///Loads a dynamic link library and calls the init function (void init(QScriptEngine* engine)).
    ///With this function a script can extend his functionality.
    Q_INVOKABLE bool loadLibrary(QString path, bool isRelativePath=true);

    ///This function stops the current script thread.
    Q_INVOKABLE void stopScript(void);

    ///Starts the program program with the arguments arguments in a new process,
    ///and detaches from it. Returns true on success, otherwise returns false.
    ///If the calling process exits, the detached process will continue to run unaffected.
    ///The process will be started in the directory workingDirectory.
    ///If workingDirectory is empty, the working directory is inherited from the calling process.
    Q_INVOKABLE bool createProcessDetached(QString program, QStringList arguments,
                                                QString  workingDirectory);

    ///Starts the program program with the arguments arguments in a new process,
    ///waits for it to finish, and then returns the exit code of the process.
    ///The environment and working directory are inherited from the calling process.
     Q_INVOKABLE int createProcess(QString program, QStringList arguments);


    ///Starts the program program with the arguments arguments in a new process.
    ///Any data the new process writes to the console is forwarded to the return process object.
    ///The environment and working directory are inherited from the calling process.
    ///Note: Blocks until the process has been created or until startWaitTime milliseconds have passed (-1=infinite).
     Q_INVOKABLE QScriptValue createProcessAsynchronous (QString program, QStringList arguments,
                                                         int startWaitTime=30000, QString workingDirectory="");

    ///Blocks until the process has finished or until msecs milliseconds have passed (-1=infinite).
    Q_INVOKABLE bool waitForFinishedProcess(QScriptValue process, int waitTime=30000);

    ///Returns the exit code of process.
    Q_INVOKABLE int getProcessExitCode(QScriptValue process);

    ///Kills the current process, causing it to exit immediately.
    Q_INVOKABLE void killProcess(QScriptValue process);

    ///Attempts to terminate the process.
    ///The process may not exit as a result of calling this function (it is given the chance to prompt the user for any unsaved files, etc).
    Q_INVOKABLE void terminateProcess(QScriptValue process);

    ///Write data to the standard input of process. Returns true on success.
    ///Note: Blocks until the writing is finished or until msecs milliseconds have passed (-1=infinite).
    Q_INVOKABLE bool writeToProcessStdin(QScriptValue process, QVector<unsigned char> data, int waitTime=30000);

    ///Returns true if the process is running.
    Q_INVOKABLE bool processIsRunning(QScriptValue process);

    ///This function returns all data available from the standard output of process (can be called after the process is finished).
	///Note: If isBlocking is true then this function blocks until the blockByte has been received, blockTime has elapsed (-1=infinite) or
	///the process is finished.
    Q_INVOKABLE QVector<unsigned char> readAllStandardOutputFromProcess(QScriptValue process, bool isBlocking=false,
                                                                        quint8 blockByte='\n', qint32 blockTime=30000);

    ///This function returns all data available from the standard error of process (can be called after the process is finished).
	///Note: If isBlocking is true then this function blocks until the blockByte has been received, blockTime has elapsed (-1=infinite) or
	///the process is finished.
    Q_INVOKABLE QVector<unsigned char> readAllStandardErrorFromProcess(QScriptValue process, bool isBlocking=false,
                                                                       quint8 blockByte='\n', qint32 blockTime=30000);

    ///Loads an user interface file.
    ///Note: If an user interface was already loaded then the old user interface will be unloaded.
    Q_INVOKABLE bool loadUserInterfaceFile(QString path, bool isRelativePath=true, bool showAfterLoading = true);

    ///Returns true if the script shall exit.
    Q_INVOKABLE bool scriptShallExit(void){return m_shallExit;}

    ///Wrapper for QFileDialog::getSaveFileName and QFileDialog::getOpenFileName.
    Q_INVOKABLE QString showFileDialog(bool isSaveDialog, QString caption, QString dir, QString filter, QWidget* parent=0);

    ///Wrapper for QFileDialog::getOpenFileNames
    Q_INVOKABLE QStringList showOpenFileNamesDialog(QString caption, QString dir, QString filter, QWidget* parent=0);

    ///Wrapper for QFileDialog::getExistingDirectory.
    Q_INVOKABLE QString showDirectoryDialog(QString caption, QString dir, QWidget* parent=0);

    ///Shows a message box.
    Q_INVOKABLE void messageBox(QString icon, QString title, QString text, QWidget* parent=0);

    ///Shows a yes/no dialog.
    Q_INVOKABLE bool showYesNoDialog(QString icon, QString title, QString text, QWidget* parent=0);

    ///Convenience function to get a string from the user.
    ///Shows a QInputDialog::getText dialog (line edit).
    Q_INVOKABLE QString showTextInputDialog(QString title, QString label, QString displayedText="", QWidget* parent=0);

    ///Convenience function to get a multiline string from the user.
    ///Shows a QInputDialog::getMultiLineText dialog (plain text edit).
    Q_INVOKABLE QString showMultiLineTextInputDialog(QString title, QString label, QString displayedText="", QWidget* parent=0);

    ///Convenience function to let the user select an item from a string list.
    ///Shows a QInputDialog::getItem dialog (combobox).
    Q_INVOKABLE QString showGetItemDialog(QString title, QString label, QStringList displayedItems,
                               int currentItemIndex=0, bool editable=false, QWidget* parent=0);

    ///Convenience function to get an integer input from the user.
    ///Shows a QInputDialog::getInt dialog (spinbox).
    Q_INVOKABLE QList<int> showGetIntDialog(QString title, QString label, int initialValue, int min, int max, int step, QWidget* parent=0);

    ///Convenience function to get a floating point number from the user.
    ///Shows a QInputDialog::getDouble dialog (spinbox).
    Q_INVOKABLE QList<double> showGetDoubleDialog(QString title, QString label, double initialValue, double min, double max, int decimals, QWidget* parent=0);

    ///Convenience function to get color settings from the user.
    Q_INVOKABLE QList<int> showColorDialog(quint8 initInitalRed=255, quint8 initInitalGreen=255, quint8 initInitalBlue=255, quint8 initInitalAlpha=255, bool alphaIsEnabled=false, QWidget* parent=0);

    ///Calculates a crc8.
    Q_INVOKABLE static quint8 calculateCrc8(const QVector<unsigned char> data){return CRC::calculateCrc8(data);}

    ///Calculates a crc8 with a given polynomial.
    Q_INVOKABLE static quint8 calculateCrc8WithPolynomial(const QVector<unsigned char> data, const unsigned char polynomial, const unsigned char startValue=0)
                {return CRC::calculateCrc8(data, polynomial, startValue);}

    ///Calculates a crc16.
    Q_INVOKABLE static quint16 calculateCrc16(const QVector<unsigned char> data){return CRC::calculateCrc16(data);}

    ///Calculates a crc32.
    Q_INVOKABLE static quint32 calculateCrc32(const QVector<unsigned char> data){return CRC::calculateCrc32(data);}

    ///Calculates a crc64.
    Q_INVOKABLE static quint64 calculateCrc64(const QVector<unsigned char> data){return CRC::calculateCrc64(data);}

    ///Scripts can switch on/off the adding of received data in the consoles (for fast data transfers).
    Q_INVOKABLE bool showReceivedDataInConsoles(bool show);

    ///Scripts can switch on/off the adding of transmitted data in the consoles (for fast data transfers).
    Q_INVOKABLE bool showTransmitDataInConsoles(bool show);

    ///Adds a message into the log and the consoles (if they are active).
    Q_INVOKABLE void addMessageToLogAndConsoles(QString text, bool forceTimeStamp=false){emit addMessageToLogAndConsolesSignal(text, forceTimeStamp);}

    ///Sets a string in the global string map.
    ///(Scripts can exchange data with this map)
    Q_INVOKABLE void setGlobalString(QString name, QString string);

    ///Returns a string from the global string map.
    ///(Scripts can exchange data with this map)
    ///Note: Returns an empty string if name is not in the map.
    Q_INVOKABLE QString getGlobalString(QString name, bool removeValue=false);

    ///Sets a data vector in the global data vector map.
    ///(Scripts can exchange data with this map)
    Q_INVOKABLE void setGlobalDataArray(QString name, QVector<unsigned char> data);

    ///Returns a data vector from the global data vector map.
    ///(Scripts can exchange data with this map)
    ///Note: Returns an empty data vector if name is not in the map.
    Q_INVOKABLE QVector<unsigned char> getGlobalDataArray(QString name, bool removeValue=false);

    ///Sets a unsigned number in the global unsigned number map.
    ///(Scripts can exchange data with this map)
    Q_INVOKABLE void setGlobalUnsignedNumber(QString name, quint32 number);

    ///Returns a unsigned number from the global unsigned number map.
    ///(Scripts can exchange data with this map)
    ///The first element is the result status (1=name found, 0=name not found)
    ///The second element is the read value.
    Q_INVOKABLE QList<quint32> getGlobalUnsignedNumber(QString name,bool removeValue=false);

    ///Sets a signed number in the global signed number map.
    ///(Scripts can exchange data with this map)
    Q_INVOKABLE void setGlobalSignedNumber(QString name, qint32 number);

    ///Returns a signed number from the global signed number map.
    ///(Scripts can exchange data with this map)
    ///The first element is the result status (1=name found, 0=name not found)
    ///The second element is the read value.
    Q_INVOKABLE QList<qint32> getGlobalSignedNumber(QString name,bool removeValue=false);

    ///Sets a real number in the global real number map.
    ///(Scripts can exchange data with this map)
    Q_INVOKABLE void setGlobalRealNumber(QString name, double number);

    ///Returns a real number from the global real number map.
    ///(Scripts can exchange data with this map)
    ///The first element is the result status (1=name found, 0=name not found)
    ///The second element is the read value.
    Q_INVOKABLE QList<double> getGlobalRealNumber(QString name, bool removeValue=false);

    ///Sets the priority of the script thread (which executes the current script).
    ///Possible values are:
    ///- LowestPriority
    ///- LowPriority
    ///- NormalPriority
    ///- HighPriority
    ///- HighestPriority
    ///
    ///Note: Per default script threads have LowestPriority.
    Q_INVOKABLE bool setScriptThreadPriority(QString priority);

    ///Returns the current version of ScriptCommunicator.
    Q_INVOKABLE QString getCurrentVersion(void){return MainWindow::VERSION;}

    ///This function exits ScriptCommunicator.
    Q_INVOKABLE void exitScriptCommunicator(qint32 exitCode=0){emit exitScriptCommunicatorSignal(exitCode);}

    ///Sets the script block time (ms).
    ///Note: If the user presses the stop button the script must be exited after this time. If not then the script is
    ///regarded as blocked and will be terminated.
    Q_INVOKABLE void setBlockTime(quint32 blockTime){m_blockTime = blockTime;}

    ///Sets the state of a script (running, paused or stopped).
    ///Note: The script must be in the script table (script window) and a script can not
    ///       set it's own state.
    Q_INVOKABLE bool setScriptState(quint8 state, QString scriptTableEntryName);

    ///Returns the script-table (script window) name of the calling script.
    Q_INVOKABLE QString getScriptTableName(void);

    ///Returns the architecture of the CPU that the application is running on, in text format.
    Q_INVOKABLE QString currentCpuArchitecture(void){return QSysInfo::currentCpuArchitecture();}

    ///Returns the product name of the operating system this application is running in.
    Q_INVOKABLE QString productType(void){return QSysInfo::productType();}

    ///Returns the product version of the operating system in string form.
    Q_INVOKABLE QString productVersion(void){return QSysInfo::productVersion();}

    ///Returns the script arguments (command-line argument -A).
    Q_INVOKABLE QStringList getScriptArguments(void);

    ///Returns the ScriptCommunicator program folder.
    Q_INVOKABLE QString getScriptCommunicatorFolder(void){return MainWindow::getScriptCommunicatorFilesFolder();}

    ///Returns the directory containing user document files.
    Q_INVOKABLE QString getUserDocumentsFolder(void){return QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);}

    ///Adds script tabs to the main window (all tabs are removed from tabWidget).
	///Note: This function fails in command-line mode.
    Q_INVOKABLE bool addTabsToMainWindow(ScriptTabWidget* tabWidget);

    ///Adds script toolbox pages to the main window (all pages are removed from toolBox).
    ///Note: This function fails in command-line mode.
    Q_INVOKABLE bool addToolBoxPagesToMainWindow(ScriptToolBox* scriptToolBox);

    ///Sends received data (received with an script internal interface) to the main interface.
    ///This data will be shown as received data in the consoles, the log and will be received by
    ///worker scripts via the dataReceivedSignal.
    Q_INVOKABLE void sendReceivedDataToMainInterface(QVector<unsigned char> data);

    ///Checks if the version of ScriptCommunicator is equal/greater then the version in minVersion.
    ///The format of minVersion is: 'major'.'minor' (e.g. 04.11).
    Q_INVOKABLE bool checkScriptCommunicatorVersion(QString minVersion);

    ///Returns and prints (if printInScriptWindowConsole is true) all functions, signals and properties of an object in the script window console.
    ///Note: Only ScriptCommunicator classes are supported. Calling this function with a QtScript built-in class (e.g. Array) will result
    ///in an empty list.
    Q_INVOKABLE QStringList getAllObjectPropertiesAndFunctions(QScriptValue object, bool printInScriptWindowConsole=false);

    ///Returns the title of the main window.
    Q_INVOKABLE QString getMainWindowTitle(void);

    ///Sets the title of the main window.
    Q_INVOKABLE void setMainWindowTitle(QString newTitle){emit setMainWindowTitleSignal(newTitle);}

    ///Returns the current time stamp in a specific format (see QDateTime.toString for more details).
    ///If the format string is empty then the format from the settings dialog (console options tab) is used.
    Q_INVOKABLE QString getTimestamp(QString format = "")
    {
        return QDateTime::currentDateTime().toString(format.isEmpty() ? m_settingsDialog->settings()->consoleTimestampFormat : format);
    }

    ///Returns the console settings (settings dialog).
    Q_INVOKABLE QScriptValue getConsoleSettings(void);

    ///Sets the main window and the ScriptCommunicator task bar icon.
    ///Supported formats: .ico, .gif, .png, .jpeg, .tiff, .bmp, .icns.
    Q_INVOKABLE void setMainWindowAndTaskBarIcon(QString iconFile, bool isRelativePath=true);

    ///Returns and all functions, signals and properties of an object.
    static void getAllObjectPropertiesAndFunctionsInternal(QScriptValue object, QStringList* resultList, QString* resultString);

    ///Returns the tread state.
    ThreadSate getThreadState(){return m_state;}

    ///Returns the pointer to the script window.
    ScriptWindow* getScriptWindow(void){return m_scriptWindow;}

    ///Returns the script engine
    QScriptEngine* getScriptEngine(void){return m_scriptEngine;}

    ///Converts a string into a QMessageBox::Icon.
    static QMessageBox::Icon stringToMessageBoxIcon(QString icon);

    ///Terminates the current script thread.
    void terminateScriptThread(void);

    ///Starts the script thread in a debugger;
    void startDebugging();

    ///Returns m_scriptRunsInDebugger.
    bool runsInDebugger(void){return m_scriptRunsInDebugger;}

    ///Returns the path of the script which is executed by the thread.
    QString getScriptFileName(void){return m_scriptFileName;}

    ///Returns m_shallPause.
    bool getShallPause(void){return m_shallPause;}

    ///Returns true if the script has connected a function to dataReceivedSignal.
    bool dataReceivedSignalIsConnected(void){return (QObject::receivers(SIGNAL(dataReceivedSignal(QVector<unsigned char>))) > 0) ? true : false;}

    ///Returns m_sendId;
    quint32 getSendId(void){return m_sendId;}

    ///Returns m_registerMetaTypeCalledinScriptWidget.
    bool registerMetaTypeCalledinScriptWidget(void){return m_registerMetaTypeCalledinScriptWidget;}

    ///Sets m_registerMetaTypeCalledinScriptWidget.
    void setRegisterMetaTypeCalledinScriptWidget(bool called){m_registerMetaTypeCalledinScriptWidget = called;}

signals:

    ///Is emitted if the clear console button in the main window is pressed.
    ///Scripts can connect a function to this signal.
    void mainWindowClearConsoleClickedSignal(void);

    ///Is emitted if the lock scrolling button in the main window is pressed.
    ///Scripts can connect a function to this signal.
    void mainWindowLockScrollingClickedSignal(bool isChecked);

    ///Is emitted if a string in the global string map has been changed.
    ///Scripts can connect a function to this signal.
    void globalStringChangedSignal(QString name, QString string);

    ///Is emitted if a data vector in the global string data vector has been changed.
    ///Scripts can connect a function to this signal.
    void globalDataArrayChangedSignal(QString name, QVector<unsigned char> data);

    ///Is emitted if an unsigned number in the global unsigned number map has been changed
    ///Scripts can connect a function to this signal.
    void globalUnsignedChangedSignal(QString name, quint32 number);

    ///Is emitted if a signed number in the global signed number map has been changed
    ///Scripts can connect a function to this signal.
    void globalSignedChangedSignal(QString name, qint32 number);

    ///Is emitted if a real number in the global real number map has been changed
    ///Scripts can connect a function to this signal.
    void globalRealChangedSignal(QString name, double number);

    ///This signal is emitted if data has been received with the main interface (only if the main interface is not a CAN or I2C master interface,
    ///use canMessagesReceivedSignal if the main interface is a can interface and i2cMasterDataReceivedSignal if the main interface is
    ///an I2C interface).
    ///Scripts can connect a function to this signal.
    void dataReceivedSignal(QVector<unsigned char> data);

    ///This signal is emitted in setMainWindowAndTaskBarIcon.
    ///This signal must not be used from script.
    void setMainWindowAndTaskBarIconSignal(QString iconFile);

    ///Is connected with ScriptWindow::appendTextToConsoleSlot (appends text to the console in the script window).
    ///This signal must not be used from script.
    void appendTextToConsoleSignal(QString text, bool newLine, bool bringToForeground);

    ///Is connected with ScriptWindow::threadStateChangedSlot (to signalize the thread state change).
    ///This signal must not be used from script.
    void threadStateChangedSignal(ThreadSate state, ScriptThread* thread);

    ///This signal is used to create a gui element (for instance PlotWindow).
    ///Is connected with ScriptWindow::createGuiElementSlot (to signalize the thread state change).
    ///Note: Gui elements in Qt can only be created in the main thread.
    ///This signal must not be used from script.
    void createGuiElementSignal(QString elementType, QObject** createdGuiElement, ScriptWindow* scriptWindow, ScriptThread* thread, QObject* additionalArgument);

    ///With this signal scripts can change the current settings.
    ///This signal must not be used from script.
    void setAllSettingsSignal(Settings& settings, bool setTabIndex);

    ///With this signal the script thread requests the main window to add a message in the log and the consoles.
    ///This signal is connected to the MainWindow::messageEnteredSlot slot.
    ///This signal must not be used from script.
    void addMessageToLogAndConsolesSignal(QString text, bool forceTimeStamp);

    ///This event is emitted in setScriptState.
    ///This signal must not be used from script.
    void setScriptStateSignal(quint8 state, QString name, bool* result);

    ///This event is emitted in getScriptTableName.
    ///This signal must not be used from script.
    void getScriptTableNameSignal(QString* scriptName);
	
	///If pause is true all created interfaces are paused.
    ///This signal must not be used from script.
    void pauseAllCreatedInterfaces(bool pause);
	
	///Is emitted if ScriptCommunicator shall exit.
    ///This signal must not be used from script.
    void exitScriptCommunicatorSignal(qint32 exitCode);
	
	///Is connected with ScriptWindow::loadUserInterfaceFileSlot (loads an user interface file).
    ///This signal must not be used from script.
    void loadUserInterfaceFileSignal(QWidget** scriptUi, QString path);

    ///Is connected with MainWindow::addTabsToMainWindowSlot.
    ///This signal must not be used from script.
    void addTabsToMainWindowSignal(QTabWidget* tabWidget);

    ///Is connected with MainWindow::addToolBoxPagesToMainWindowSlot.
    ///This signal must not be used from script.
    void addToolBoxPagesToMainWindowSignal(QToolBox* toolBox);

    ///Enables/Disables all script tabs for one script thread.
    ///This signal must not be used from script.
    void enableAllTabsForOneScriptThreadSignal(QObject *scriptThread, bool enable);

    ///Is connected with MainWindow::setWindowTitle.
    ///This signal must not be used from script.
    void setMainWindowTitleSignal(QString newTitle);

protected:
    ///The thread main function.
    void run();

public slots:

    ///The script is suspended by the debugger.
    void suspendedByDebuggerSlot();

    ///The script is resumed by the debugger.
    void resumedByDebuggerSlot();

    ///Is called if a string in the global string map has been changed.
    void globalStringChangedSlot(QString* name, QString* string){emit globalStringChangedSignal(*name, *string);}

    ///Is called if a data vector in the global string data vector has been changed.
    void globalDataArrayChangedSlot(QString* name, QVector<unsigned char>* data){emit globalDataArrayChangedSignal(*name, *data);}

    ///Is called if an unsigned number in the global unsigned number map has been changed
    void globalUnsignedChangedSlot(QString* name, quint32 number){emit globalUnsignedChangedSignal(*name, number);}

    ///Is called if a signed number in the global signed number map has been changed
    void globalSignedChangedSlot(QString* name, qint32 number){emit globalSignedChangedSignal(*name, number);}

    ///Is called if a real number in the global real number map has been changed
    void globalRealChangedSlot(QString* name, double number){emit globalRealChangedSignal(*name, number);}

    ///This slot function is called if a script function connected to a signal causes an exception.
    void scriptSignalHandlerSlot(const QScriptValue & exception);

    ///This slot is called periodically by the timer m_pauseTimer.
    ///This function checks if the thread has to be paused and do the necessary actions.
    void pauseTimerSlot();

    ///Is called if the clear console button in the main window is pressed.
    void mainWindowClearConsoleSlot(void){emit mainWindowClearConsoleClickedSignal();}

    ///Is called if the lock scrolling button in the main window is pressed.
    void mainWindowLockScrollingSlot(bool isChecked){emit mainWindowLockScrollingClickedSignal(isChecked);}


#ifdef Q_OS_MAC
    ///Debug timer slot (checks if the script is suspended by the debugger or is running).
    void debugTimerSlot(void);
#endif

private:

    ///Contains all created gui elements (creates by the script).
    std::vector<ScriptWidget*> m_allCreatedGuiElementsFromScript;

    ///Sets the current thread state.
    void setThreadState(ThreadSate state);

    ///Pointer to the script window.
    ScriptWindow* m_scriptWindow;

    ///The send id, which is send to the send data during sending data.
    quint32 m_sendId;

    ///True, if the thread (the script) shall exit.
    bool m_shallExit;

    ///True, if the thread (the script) shall pause.
    bool m_shallPause;

    ///True if the script runs in a debugger;
    bool m_scriptRunsInDebugger;

    ///The current thread state.
    ThreadSate m_state;

    ///The path of the script which is executed by the thread.
    QString m_scriptFileName;

    ///The timer which calls periodically calls pauseTimerSlot.
    QTimer* m_pauseTimer;

    ///The script user interface (loaded from an ui file).
    QList<ScriptWidget*> m_userInterface;

    ///The script interpreter/engine
    QScriptEngine* m_scriptEngine;

    ///Pointer to the settings dialog.
    SettingsDialog *m_settingsDialog;

    ///The script sql object.
    ScriptSql m_scriptSql;

    ///The script block time (ms).
    ///Note: If the user presses the stop button the script must be stopped after this time. If not then the script will
    ///regarded as blocked and will be terminated.
    quint32 m_blockTime;

    ///The default value for m_blockTime.
    static const quint32 DEFAULT_BLOCK_TIME= 5000;

    ///The script standard dialogs.
    ScriptStandardDialogs* m_standardDialogs;

    ///The script file object.
    ScriptFile* m_scriptFileObject;

    ///True if the script is suspended by the debugger.
    bool m_isSuspendedByDebuger;

    ///The script debugger;
    QScriptEngineDebugger *m_debugger ;

    ///The debug window.
    QMainWindow *m_debugWindow;

    ///True if the script has GUI elements in the main window.
    bool m_hasMainWindowGuiElements;

#ifdef Q_OS_MAC
    ///The debug timer (checks if the script is suspended by the debugger or is running).
    QTimer m_debugTimer;
#endif

    ///This timer is used to emit the dataReceivedSignal in debugReceiveTimerSlot (if the script is running in the script debugger).
    QTimer m_debugReceiveTimer;

    ///Contains the saved received data (dataReceivedSlot) if the script is running in the script debugger.
    QVector<unsigned char> m_savedReceivedData;

    ///Contains all loaded libraries (in loadLibrary).
    QVector<QLibrary*> m_libraries;

    ///The script converter object.
    ScriptConverter m_converterObject;

    ///The ScriptInf object.
    ScriptInf* m_scriptInf;

    ///True if qRegisterMetaType was already called in ScriptWidget constructor.
    bool m_registerMetaTypeCalledinScriptWidget;

};

#endif // SCRIPTTHREAD_H

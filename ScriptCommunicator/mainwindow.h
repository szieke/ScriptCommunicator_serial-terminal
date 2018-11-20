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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtCore/QtGlobal>
#include <QMainWindow>
#include <QtSerialPort/QSerialPort>
#include <QThread>
#include <QTime>
#include <QFile>
#include <QTextStream>
#include "settingsdialog.h"
#include <QMessageBox>
#include <QXmlStreamWriter>
#include <QWidget>
#include <QTextEdit>
#include <QTcpServer>
#include <QTcpSocket>
#include <QUdpSocket>
#include <QLabel>
#include <QTimer>
#include <QKeyEvent>
#include <QScriptEngine>
#include "mainwindowHandleData.h"
#include <QSplitter>
#include <QListWidgetItem>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QLineEdit>
#include <QFileInfo>
#include <QDir>
#include <QToolBox>



QT_BEGIN_NAMESPACE

namespace Ui {
class MainWindow;
}

QT_END_NAMESPACE

class SendWindow;
class ScriptWindow;
class QMutex;
class MainInterfaceThread;
class AddMessageDialog;
class CanTab;
class MainWindow;
class SearchConsole;

class DragDropLineEdit : public QLineEdit
{
    Q_OBJECT

public:

    explicit DragDropLineEdit(QWidget* parent = 0) : QLineEdit(parent)
    {

    }
protected:
    ///Drag enter event.
    void dragEnterEvent(QDragEnterEvent *event);

    ///Drop event.
    void dropEvent(QDropEvent *event);

};

///Class for the consoles in the main window.
class SendConsole : public QTextEdit
{
     Q_OBJECT

public:
    explicit SendConsole(QWidget *parent = 0) : QTextEdit(parent), m_mainWindow(0), m_resizeTimer(), m_isMixedConsole(false)
    {
        connect(this->document(), SIGNAL(contentsChange(int,int,int)), this, SLOT(contentsChangeSlot(int,int,int)), Qt::QueuedConnection);
        connect(&m_resizeTimer, SIGNAL(timeout()),this, SLOT(resizeSlot()));
    }

    virtual ~SendConsole()
    {

    }

    ///Sets m_mainWindow.
    void setMainWindow(MainWindow* mainWindow)
    {
        m_mainWindow = mainWindow;
    }

    ///Sets m_isMixedConsole.
    void setIsMixedConsole(bool isMixedConsole){m_isMixedConsole = isMixedConsole;}

protected:

    ///The user scrolled within console.
    void wheelEvent(QWheelEvent *event);

    ///The user has pressed a key.
    void keyPressEvent(QKeyEvent *event);

    ///Is called if the window is resized.
    void resizeEvent(QResizeEvent* event);


public slots:

    ///Is called if the document's content changes.
   void contentsChangeSlot(int from, int charsRemoved, int charsAdded);

   ///Is called if m_resizeTimer elapsed. Here the text edit is resized.
   void resizeSlot(void);


private:
    ///Pointer to the main window.
    MainWindow* m_mainWindow;

    ///This timer is started in resizeEvent;
    QTimer m_resizeTimer;

    ///True if the current isntance is the mixed console.
    bool m_isMixedConsole;

};


///Class which represents the main window.
class MainWindow : public QMainWindow
{
    friend class CanTab;
    friend class SendConsole;
    friend class SearchConsole;
    friend class MainWindowHandleData;
    friend class SendWindowTextEdit;
    Q_OBJECT


public:
    explicit MainWindow(QStringList scripts, bool withScriptWindow, bool scriptWindowIsMinimized,
                        QStringList extraPluginPaths, QStringList scriptArguments, QString configFile, QString iconFile);
    ~MainWindow();

    ///Hide pop-up menu on toolbar
    QMenu *createPopupMenu(){ return NULL;}

    ///The current version of ScriptCommunicator.
    static const QString VERSION;

    ///The name of the intial main config file.
    static const QString INIT_MAIN_CONFIG_FILE;

    ///Parses a API file and returns a semicolon separated list with all public functions, signals and properties.
    static QString parseApiFile(QString name);

    ///Converts a byte array into his string representation (Byte 1 is converted into char '1'...).
    static QString byteArrayToNumberString(const QByteArray &data, bool isBinary,  bool isHex, bool withFormatBrackets, bool withLeadingZero = true,
                                           bool withSpaces = true, DecimalType decimalType = DECIMAL_TYPE_UINT8, Endianess endianess = LITTLE_ENDIAN_TARGET);

    ///Starts/executes the script editor.
    static bool startScriptEditor(QString scriptEditor, QStringList arguments, QWidget *parent, bool isInternalEditor);

    ///Opens the external script editor.
    static void openScriptEditor(QStringList arguments, const Settings *currentSettings, QWidget *parent);

    ///Limits the number of char in the given text edit to the value of maxChars.
    static void limtCharsInTextEdit(const QTextEdit *textEdit, const int maxChars);

    ///Sets the position and the size of a window.
    static void setWindowPositionAndSize(QWidget* widget, const QRect& positionAndSize);

    ///Returns the current position and size of the main window.
    static QRect windowPositionAndSize(QWidget* widget){return QRect(widget->pos(), widget->size());}

    ///Shows the send window.
    void show(void);

    ///Returns the program folder (in the user documents folder). If the folder doesn't exists it will be created.
    static QString getAndCreateProgramUserFolder(void);

    ///Returns the folder ich which the ScriptCommunicator files are locared (templates, example scripts and the manual)
    static QString getScriptCommunicatorFilesFolder(void);

    ///Returns the ScriptCommunicator plugin folder
    static QString getPluginsFolder(void);

    ///Converts an absolute file path to a relative file path (relativ to rootFile).
    static QString convertToRelativePath(QString rootFile, QString fileName)
    {
        QString result = fileName.isEmpty() ? "" : QDir(QFileInfo(rootFile).absolutePath()).relativeFilePath(fileName);
        return result.isEmpty() ? fileName : result;
    }

    ///Converts a relative file path (relativ to rootFile) to an absolute file path.
    static QString convertToAbsolutePath(QString rootFile, QString fileName);

    ///Reads the main config fie list.
    QStringList readMainConfigFileList(bool removeDefaultMarker = true);

    ///Saves the main config fie list.
    void saveMainConfigFileList(QStringList list);

    ///Is called if the window is resized.
    void resizeEvent(QResizeEvent* event);

    ///Returns m_settingsDialog.
    SettingsDialog* getSettingsDialog(void){return m_settingsDialog;}

    ///Returns the console (QTextWidget) from the current tab.
    QTextEdit* getConsoleFromCurrentTab(QWidget *widget);

    ///This function exits ScriptCommunicator.
    void exitScriptCommunicator(void);

    ///Returns m_handleData.
    MainWindowHandleData* getHandleDataObject(){return m_handleData;}

    ///Parses an Sce file.
    static bool parseSceFile(QString fileName, QStringList* scripts, QStringList* extraPluginPaths, QStringList* scriptArguments,
                                         bool* withScriptWindow, bool* scriptWindowIsMinimized, QString *minimumScVersion, QStringList* extraPlugInPath);

    ///Checks the parsed mimium ScriptCommunicator version (SCE file).
    static bool checkParsedScVersion(QString version);

    ///Checks the hash of a scez file.
    static bool checkScezFileHash(QString fileName);

    ///Returns m_scriptWindow.
    ScriptWindow* getScriptWindow(){return m_scriptWindow;}

    ///Returns the exra plugin paths (command-line argument -P).
    QStringList getExtraPluginPaths(void){return m_extraPluginPaths;}

    ///Returns the script arguments (command-line argument -A).
    QStringList getScriptArguments(void){return m_scriptArguments;}

    ///Returns true the command-line mode is active.
    bool isCommandLineMode(void){return m_commandLineScripts.isEmpty() ? false : true;}

     ///Returns true if the ScriptCommunicator starts the first time.
    bool isFirstProgramStart(void){return m_isFirstProgramStart;}

    ///Removes all script tabs and tool box pages for one script thread.
    void removeAllTabsAndToolBoxPages(QObject *scriptThread);

    ///Returns the user interface.
    Ui::MainWindow * getUserInterface(void){ return m_userInterface;}

    ///Returns m_closedByScript.
    bool closedByScript(void){return m_closedByScript;}

    ///Returns m_exitCode.
    qint32 getExitCode(void){return m_exitCode;}

    ///Sets the exit code from a script.
    void setExitCodeFromScript(quint32 code){m_exitCode = code; m_closedByScript = true;}


signals:
    ///With this signal the main window requests the main interface thread to connect with the man interface interface.
    ///This signal is connected to the MainInterfaceThread::connectDataConnectionSlot slot.
    void connectDataConnectionSignal(Settings settings, bool connect);

    ///The signal is emitted if the global settings have been changed.
    void globalSettingsChangedSignal(Settings globalSettings);

    ///With this signal the main window requests the main interface thread to exit.
    ///This signal is connected to the MainInterfaceThread::exitThreadSlot slot.
    void exitThreadSignal(void);

    ///All windows shall be in forground.
    void bringWindowsToFrontSignal(void);

public slots:

    ///Adds script tabs to the main window.
    void addTabsToMainWindowSlot(QTabWidget* tabWidget);

    ///Adds script toolbox pages to the main window.
    void addToolBoxPagesToMainWindowSlot(QToolBox* toolBox);

    ///Enables/Disables all script tabs for one script thread.
    void enableAllTabsForOneScriptThreadSlot(QObject *scriptThread, bool enable);

    ///Brings all windows to foreground.
    void bringWindowsToFrontSlot(void);

    ///Is called if the the selected row in the worker script list has been changed.
    void workerScriptsCurrentRowChangedSlot(int currentRow);

    ///Is called if the user presses the start script button.
    void startScriptButtonPressedSlot(void);

    ///Is called if the user presses the pause script button.
    void pauseScriptButtonPressedSlot(void);

    ///Create all buttons in the sequence page.
    void setUpSequencesPageSlot(void);

    ///Create all buttons in the script page.
    void setUpScriptPageSlot(void);

    ///Disables all mouse events for all windows.
    void disableMouseEventsSlot();

    ///Enables all mouse events for all windows.
    void enableMouseEventsSlot();

    ///The main window receive the current connection status with this slot (from the main interface thread).
    ///This slot is connected to the MainInterfaceThread::dataConnectionStatusSignal signal.
    void dataConnectionStatusSlot(bool isConnected, QString message, bool isWaiting);

    ///Shows additional Information about the connection in the mainwindow.
    void showAdditionalConnectionInformationSlot(QString text);

    ///Is called if a serial port check box has been changed (DTR, RTS).
    void serialPortPinsChangedSlot(void);

    ///Is called (by script) if the serial port pins shall be changed (DTR, RTS).
    void setSerialPortPinsSlot(bool setRTS, bool setDTR);

    ///The main interface thread can activate/deactivate the connect button with this slot.
    ///This slot is connected to the MainInterfaceThread::setConnectionButtonsSignal signal.
    void setConnectionButtonsSlot(bool enable);

    ///This slot function shows a message box.
    void showMessageBoxSlot(QMessageBox::Icon icon, QString title, QString text, QMessageBox::StandardButtons buttons,
                            QWidget* parent = 0);

    ///This slot function shows a yes/no dialog.
    void showYesNoDialogSlot(QMessageBox::Icon icon, QString title, QString text, QWidget* parent, bool* yesButtonPressed);

    ///This slot is called if the main configuration has to be saved.
    void configHasToBeSavedSlot(void);

    ///This slot is called if the connection type has been changed.
    void conectionTypeChangesSlot();

    ///Reopens all open logs.
    void reopenLogsSlot(void);

    ///Adds data to the main window send history.
    void addDataToMainWindowSendHistorySlot(QByteArray data);

    ///Sets the main window and the ScriptCommunicator task bar icon.
    ///Supported formats: .ico, .gif, .png, .jpeg, .tiff, .bmp, .icns.
   void setMainWindowAndTaskBarIconSlot(QString iconFile);

private slots:

    ///Menu debug sequence script slot function.
    void debugSequenceScript(void);

    ///Is called if text of the cyclic script text edit has been changed.
    void scriptTextEditSlot(void);

    ///Is called when the user double clicks the script text edit.
    void scriptTextEditDoubleClickedSlot(void);

    ///Slot for the config file lock file timer.
    void configLockFileTimerSlot();

    ///This slot function is called if the send area splitter handle has been moved.
    void sendAreaSplitterMoved(int pos, int index);

    ///This slot function is called if the send area inputs splitter handle has been moved.
    void sendAreaInputsSplitterMoved(int pos, int index);

    ///This slot function is called if the tool box splitter handle has been moved.
    void toolBoxSplitterMoved(int pos, int index);

    ///This slot function is called if the current index of the tool box has been changed.
    void currentToolBoxPageChangedSlot(int index);

    ///This slot function is called if the user changes the text of the send text edit box.
    void sendInputTextChangedSlot(void);

    ////Checks if the text has the correct format.
    void checkSendInputSlot(void);

    ///Is called if the user presses the clear history button.
    void clearHistoryButtonSlot();

    ///Is called if the user presses the send history button.
    void sendHistoryButtonSlot();

    ///Is called if the user presses the create script button (send history).
    void createScriptButtonSlot();

    ///Slot function for the send button.
    void sendButtonPressedSlot(bool debug = false);

    ///Is called if the history start index has been changed.
    void historyStartIndexChangedSlot(int value);

    ///Is called if the history end index has been changed.
    void historyEndIndexChangedSlot(int value);

    ///Is called if the history format has been changed.
    void historyFormatChanged(QString value);

    ///This slot is called if the value of the send format combobox has been changed.
    void currentSendFormatChangedSlot(QString format);

    ///Timer slot function for minimizing the script window (command line mode).
    void commandLineModeMinimizeTimerSlot();

    ///Is called if an item in the sequence list has been double clicked.
    void sequenceListItemDoubleClickedSlot(QListWidgetItem *item);

    ///Is called if an item in the worker script list has been double clicked.
    void workerScriptListItemDoubleClickedSlot(QListWidgetItem *item);

    ///Current data rates slot.
    void dataRateUpdateSlot(quint32 dataRateSend, quint32 dataRateReceive);

    ///A user message has been entered (in the add message dialog or in a script).
    void messageEnteredSlot(QString message, bool forceTimeStamp);

    ///A slider of a vertical scrolbar has been moved.
    void verticalSliderMovedSlot(int pos);

    ///Slot function for the connect button.
    void toggleConnectionSlot(bool connectionStatus);

    ///Slot function for the connect button.
    void connectButtonSlot(void);

    ///Slot function for the show sending window button.
    void showSendingWindowSlot();

    ///Slot function for the show script window button.
    void showScriptWindowSlot(void);

    ///Slot function for the show about window button.
    void showAboutWindowSlot(void);

    ///Slot function which shows/hide the add message dialog.
    void openAddMessageDialogSlot(void);

    ///Slot function which opens the manual.
    void openTheManualSlot(void);

    ///Slot function for the show settings window button.
    void showSettingsWindowSlot(void);

    ///Slot function for the clear console button.
    void clearConsoleSlot();

    ///This slot function is called if the log file has to be deleted.
    ///It is connected to the SettingsDialog::deleteLogFileSignal signal.
    void deleteLogFileSlot(QString logType);

    ///Menu copy config slot function.
    void copyConfigSlot(void);

    ///Menu create config slot function.
    void createConfigSlot();

    ///Menu save console slot function.
    void saveConsoleSlot();

    ///Is called if the tab index has been changed.
    void tabIndexChangedSlot(int index);

    ///Menu print console slot function.
    void printConsoleSlot();

    ///Menu submit bug slot function.
    void submitBugSlot();

    ///Menu request feature slot function.
    void requestFeatureSlot();

    ///Menu edit script slot function.
    void editScriptSlot();

    ///Menu add script slot function.
    void addScriptSlot();

    ///Menu check for updates slot function.
    void checkForUpdatesSlot();

    ///Slot function for the update manager replies.
    void updateManagerReplyFinished(QNetworkReply* reply);

    ///Menu get support slot function.
    void getSupportSlot();

    ///Menu video slot function.
    void watchVideoSlot();

    ///Menu delete previous config list slot function.
    void deletePreviousConfigListSlot(void);

    ///Menu set current config to default slot function.
    void setCurrentConfigToDefaultSlot(void);

    ///Menu reset default config slot function.
    void resetDefaultConfigSlot(void);

    ///Menu load config slot function.
    void loadConfigSlot(void);

    ///Menu load previous config slot function.
    void loadPreviousConfigSlot(void);

    ///This slot function is called if the html log file has to be activated.
    ///It is connected to the SettingsDialog::htmlLogActivatedSignal signal.
    void htmLogActivatedSlot(bool activated);

    ///This slot function is called if the text log file has to be activated.
    ///It is connected to the SettingsDialog::textLogActivatedSignal signal.
    void textLogActivatedSlot(bool activated);

private:

    ///Event filter function.
    bool eventFilter(QObject *target, QEvent *event);

    ///Creates a log file name.
    QString createLogFileName(QString logFileName);

    ///Creates a new config and loads it.
    bool createConfig(bool isCallFromButton);

    ///Restores the size of the second element of the splitter (after the main window has been resized).
    void restoreSizeSplitterSecondElement(QSplitter* splitter, qint32 oldSize);

    ///Appends a console string to a console.
    void appendConsoleStringToConsole(QString* consoleString, QTextEdit* textEdit);

    ///This function is called if the main window is closed.
    void closeEvent(QCloseEvent * event);

    ///Initializes all actions from the main window gui.
    void initActionsConnections();

    ///Saves the main configuration.
    void saveSettings();

    ///Loads the main configuration.
    bool loadSettings();

    ///Write an XML element and his attributes to the XML stream.
    void writeXmlElement(QXmlStreamWriter& xmlWriter, QString elementName, std::map<QString, QString> &attributes);

    ///Initializes the tab in the main window.
    void inititializeTab(void);

    ///Sets the background color of a widget.
    void setWidgetBackgroundColorFromString(QString colorString, QWidget *widget);

    ///Sets the text color of a widget.
    void setWidgetTextColorFromString(QString colorString, QWidget *widget);

    ///Sets the font of a console.
    void setConsoleFont(QString fontFamily, QString fontSize, QTextEdit* textEdit);

    ///Shows the number of received and sent bytes.
    void showNumberOfReceivedAndSentBytes(void);

private:
    ///Pointer to the user interface.
    Ui::MainWindow *m_userInterface;

    ///Pointer to the settings dialog.
    SettingsDialog *m_settingsDialog;

    ///Pointer to the send window.
    SendWindow* m_sendWindow;

    ///Pointer at the main interface thread.
    MainInterfaceThread* m_mainInterface;

    ///True if the main interface (main interface thread) is connected.
    bool m_isConnected;

    ///The position and size of the send window.
    bool m_sendWindowPositionAndSizeloaded;

    ///Pointer to the script window.
    ScriptWindow* m_scriptWindow;

    ///The position and size of the script window.
    bool m_scriptWindowPositionAndSizeloaded;

    ///The add message dialog;
    AddMessageDialog* m_addMessageDialog;

    ///The label in the status bar.
    QLabel m_statusBarLabel;

    ///The main config file Path;
    QString m_mainConfigFile;

    ///The path to the file which contains all known config files.
    QString m_mainConfigFileList;

    ///This timer is started in resizeEvent;
    QTimer m_resizeTimer;

    ///True if the current connected interface is a can interface;
    bool m_isConnectedWithCan;

    ///True if the current connected interface is an I2C master interface;
    bool m_isConnectedWithI2cMaster;

    ///The can tab.
    CanTab* m_canTab;

    ///Command line scripts.
    QStringList m_commandLineScripts;

    ///True if the ScriptCommunicator starts the first time.
    bool m_isFirstProgramStart;

    ///Dummy widget for enabling/disabling all mouse events.
    QWidget* m_mouseGrabWidget;

    ///Contains all buttons in the script page.
    QVector<QToolButton*> m_scriptButtons;

    ///The search console dialog.
    SearchConsole* m_searchConsole;

    ///The current send data rate.
    quint32 m_dataRateSend;

    ///The current receive data rate.
    quint32 m_dataRateReceive;

    ///Timer for minimizing the script window (command line mode).
    QTimer m_commandLineModeMinimizeTimer;

    ///Object which the functions for handling the sent and recieved data.
    MainWindowHandleData* m_handleData;

    ///The old value for the send format combobox.
    QString m_oldSendFormat;

    ///The size of the second element in the tool box splitter.
    qint32 m_toolBoxSplitterSizeSecond;

    ///The size of the second element in send area splitter.
    qint32 m_sendAreaSplitterSizeSecond;

    ///The size of the second element in send area inputs splitter.
    qint32 m_sendAreaInputsSplitterSizeSecond;

    ///The size of the second element in the tool box splitter (for every tool box page).
    QList<qint32> m_toolBoxSplitterSizesSecond;

    ///The page index of th tool box.
    qint32 m_currentToolBoxIndex;

    ///The lock file for the main config file.
    QFile m_mainConfigLockFile;

    ///The timer for the config lock file.
    QTimer m_configLockFileTimer;

    ///Contains the exra plugin paths (command-line argument -P).
    QStringList m_extraPluginPaths;

    ///Contains the script arguments (command-line argument -A).
    QStringList m_scriptArguments;

    ///Checks for ScriptCommunicator updates.
    QNetworkAccessManager *updatesManager;

    ///Map which contains all script tabs (the second argument is a pointer to the script thread).
    QMap<QWidget*, QObject*> m_scriptTabs;

    ///Map which contains all script tabs titles (the first argument is a pointer to the tab).
    QMap<QWidget*, QString> m_scriptTabsTitles;

    ///Map which contains all script tool box page (the second argument is a pointer to the script thread).
    QMap<QWidget*, QObject*> m_scriptToolBoxPage;

    ///True if the main window was closed by a script (exitScriptCommunicator).
    bool m_closedByScript;

    ///The exit code which was passed in exitScriptCommunicator.
    qint32 m_exitCode;
};


#endif // MAINWINDOW_H

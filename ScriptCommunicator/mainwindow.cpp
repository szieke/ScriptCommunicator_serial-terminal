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

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "ui_settingsdialog.h"
#include "ui_createSceFile.h"
#include "settingsdialog.h"
#include "sendwindow.h"
#include <QTimer>
#include <QScrollBar>
#include <QMutex>
#include <QTime>

#ifdef Q_OS_WIN32
#include <Windows.h>
#include <QtWinExtras/QtWin>
#endif

#include <QMessageBox>
#include <QtSerialPort/QSerialPort>
#include <QFileDialog>
#include <QDomDocument>
#include "scriptwindow.h"
#include <QStandardPaths>
#include "addmessagedialog.h"
#include "canTab.h"
#include <QPrintDialog>
#include "searchconsole.h"
#include "customConsoleLogObject.h"
#include "scriptTcpClient.h"
#include "version.h"

///The current version of ScriptCommunicator.
const QString MainWindow::VERSION = SCRIPT_COMMUNICATOR_VERSION;

#ifdef Q_OS_WIN32
const QString MainWindow::INIT_MAIN_CONFIG_FILE = "initialSettingsWin.config";
#else

#ifdef Q_OS_MAC
const QString MainWindow::INIT_MAIN_CONFIG_FILE = "initialSettingsMac.config";
#else
const QString MainWindow::INIT_MAIN_CONFIG_FILE = "initialSettings.config";
#endif//#ifdef Q_OS_MAC

#endif


/**
 * Drag enter event.
 * @param event
 *      The drag enter event.
 */
void DragDropLineEdit::dragEnterEvent(QDragEnterEvent *event)
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
void DragDropLineEdit::dropEvent(QDropEvent *event)
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
            setText(list[0]);
        }
        event->acceptProposedAction();
    }
}


/**
 * Is called if the document's content changes.
 *
 * @param from
 *      The position of the character in the document where the change occurred.
 * @param charsRemoved
 *      The number of chars removed.
 * @param charsAdded
 *      The number of chars added.
 */
void SendConsole::contentsChangeSlot(int from, int charsRemoved, int charsAdded)
{
    (void)charsRemoved;

    if(m_mainWindow->m_userInterface->interactiveConsoleCheckBox->isChecked() && (charsAdded != 0))
    {
        QString added = toPlainText().mid(from,charsAdded);

        if(!added.isEmpty())
        {
            QTextCursor cursor = textCursor();
            cursor.clearSelection();
            cursor.setPosition(from);
            cursor.setPosition(from + (charsAdded - charsRemoved), QTextCursor::KeepAnchor);
            cursor.removeSelectedText();


            const Settings* settings = m_mainWindow->m_settingsDialog->settings();
            m_mainWindow->m_sendWindow->sendDataWithTheMainInterface(added.toLocal8Bit().replace("\n", settings->consoleSendOnEnter.toLocal8Bit()), this);
        }
    }
}


/**
 * Use Ctrl + mouse wheel to increase/decrease font size in consoles.
 *
 * @param event
 *      The wheel event.
 */
void SendConsole::wheelEvent(QWheelEvent *event)
{
    if (event->modifiers() == Qt::ControlModifier)
    {
        SettingsDialog* settingsDialog = m_mainWindow->getSettingsDialog();
        Settings currentSettings = *settingsDialog->settings();
        auto fontSize = currentSettings.stringConsoleFontSize.toInt();

        QPoint numDegrees = event->angleDelta();
        if (!numDegrees.isNull())
        {
            if (numDegrees.y() > 0)
            {
                //Scroll up zooms in.
                if (++fontSize > Settings::MAX_FONT_SIZE)
                    fontSize = Settings::MAX_FONT_SIZE;
            } else
            {
                //Scroll down zooms out.
                if (--fontSize < Settings::MIN_FONT_SIZE)
                    fontSize = Settings::MIN_FONT_SIZE;
            }
        }

        currentSettings.stringConsoleFontSize = QString::number(fontSize);
        settingsDialog->setAllSettingsSlot(currentSettings, false);
    }

    //Forward event to parent for normal scrolling.
    QTextEdit::wheelEvent(event);
}

/**
 * Constructor.
 *
 * @param scripts
 *      The command-line scripts.
 * @param withScriptWindow
 *      True if the script window shall be shown (command-line mode).
 * @param scriptWindowIsMinimized
 *      True if the script window shall be minimized (command-line mode).
 * @param extraPluginPaths
 *      Extra plug-in search paths.
 * @param configFile
 *      The command line config file.
 * @param iconFile
 *      The icon file.
 */
MainWindow::MainWindow(QStringList scripts, bool withScriptWindow, bool scriptWindowIsMinimized, QStringList extraPluginPaths, QStringList scriptArguments,
                       QString configFile, QString iconFile) :
    QMainWindow(0),
    m_userInterface(new Ui::MainWindow), m_isConnected(false),
    m_sendWindowPositionAndSizeloaded(false), m_scriptWindowPositionAndSizeloaded(false),
    m_isConnectedWithCan(false), m_commandLineScripts(scripts),
    m_isFirstProgramStart(false), m_mouseGrabWidget(0), m_searchConsole(0),
    m_dataRateSend(0), m_dataRateReceive(0), m_handleData(0), m_ignoreNextResizeEventTime(QDateTime::currentDateTime()), m_toolBoxSplitterSizeSecond(0),
    m_sendAreaSplitterSizeSecond(0), m_sendAreaInputsSplitterSizeSecond(0), m_toolBoxSplitterSizesSecond(), m_currentToolBoxIndex(0), m_mainConfigLockFile(),
    m_configLockFileTimer(), m_extraPluginPaths(extraPluginPaths), m_scriptArguments(scriptArguments), updatesManager(0), m_scriptTabs(), m_scriptTabsTitles(),
    m_scriptToolBoxPage()
{

    m_userInterface->setupUi(this);

    m_userInterface->sequenceListWidget->setIconSize(QSize(0,0));
    m_userInterface->workerScriptListWidget->setIconSize(QSize(0,0));

    QSize tmpSize = m_userInterface->startWorkerScriptsPushButton->size();
    m_userInterface->startWorkerScriptsPushButton->setMinimumSize(tmpSize);
    m_userInterface->pauseWorkerScriptPushButton->setMinimumSize(tmpSize);
    m_userInterface->pauseWorkerScriptPushButton->setText("debug");
    m_userInterface->startWorkerScriptsPushButton->setText("start");



    QStringList availTargets;
    availTargets << "ascii" << "hex" << "bin" << "uint8" << "uint16" << "uint32" << "int8" << "int16" << "int32";
    m_userInterface->SendFormatComboBox->addItems(availTargets);
    m_userInterface->SendFormatComboBox->setCurrentText("ascii");
    m_oldSendFormat = "ascii";

    m_userInterface->historyFormatComboBox->addItems(availTargets);
    m_userInterface->historyFormatComboBox->setCurrentText("hex");

    m_canTab = new CanTab(this);

    m_userInterface->ReceiveTextEditAscii->setMainWindow(this);
    m_userInterface->ReceiveTextEditBinary->setMainWindow(this);
    m_userInterface->ReceiveTextEditDecimal->setMainWindow(this);
    m_userInterface->ReceiveTextEditHex->setMainWindow(this);
    m_userInterface->ReceiveTextEditMixed->setMainWindow(this);
    m_userInterface->ReceiveTextEditCustom->setMainWindow(this);


    m_settingsDialog = new SettingsDialog(m_userInterface->actionLockScrolling);
    m_sendWindow = new SendWindow(m_settingsDialog, this);
    m_handleData = new MainWindowHandleData(this, m_settingsDialog, m_userInterface);

    m_userInterface->SendTextEdit->setIsMainWindowSendArea(true);
    m_userInterface->SendTextEdit->setMainWindowPointer(this);

    m_mainInterface = new MainInterfaceThread(this);
    m_mainInterface->moveToThread(m_mainInterface);
    m_mainInterface->start(QThread::TimeCriticalPriority);
    QThread::msleep(10);//Let the interface thread run.

    connect(m_sendWindow, SIGNAL(sequenceTableHasChangedSignal()),this, SLOT(setUpSequencesPageSlot()));

    m_addMessageDialog = new AddMessageDialog(this, m_settingsDialog);

    m_userInterface->statusBar->addPermanentWidget(&m_statusBarLabel, 1);


    m_userInterface->actionQuit->setEnabled(true);
    m_userInterface->actionConfigure->setEnabled(true);

    initActionsConnections();


    m_userInterface->ReceiveLable->setText("0 bytes received");

    m_userInterface->ReceiveTextEditAscii->setFocus();


    qRegisterMetaType< QVector<QByteArray>>("QVector<QByteArray>");

    connect(m_addMessageDialog, SIGNAL(messageEnteredSignal(QString, bool)),this, SLOT(messageEnteredSlot(QString, bool)));

    connect(&m_resizeTimer, SIGNAL(timeout()),m_handleData, SLOT(reInsertDataInMixecConsoleSlot()));

    connect(m_sendWindow, SIGNAL(sendDataWithTheMainInterfaceSignal(QByteArray,uint)), m_mainInterface, SLOT(sendDataSlot(QByteArray, uint)), Qt::QueuedConnection);
    connect(m_handleData, SIGNAL(sendDataWithTheMainInterfaceSignal(QByteArray,uint)), m_mainInterface, SLOT(sendDataSlot(QByteArray, uint)), Qt::QueuedConnection);


    connect(m_sendWindow, SIGNAL(configHasToBeSavedSignal()),this, SLOT(configHasToBeSavedSlot()));

    connect(m_mainInterface, SIGNAL(showAdditionalConnectionInformationSignal(QString)),this, SLOT(showAdditionalConnectionInformationSlot(QString)), Qt::QueuedConnection);

    connect(m_mainInterface, SIGNAL(disableMouseEventsSignal()),
            this, SLOT(disableMouseEventsSlot()), Qt::BlockingQueuedConnection);

    connect(m_mainInterface, SIGNAL(enableMouseEventsSignal()),
            this, SLOT(enableMouseEventsSlot()), Qt::BlockingQueuedConnection);

    connect(m_mainInterface, SIGNAL(setConnectionButtonsSignal(bool)), this, SLOT(setConnectionButtonsSlot(bool)), Qt::QueuedConnection);

    connect(m_settingsDialog, SIGNAL(deleteLogFileSignal(QString)),this, SLOT(deleteLogFileSlot(QString)));
    connect(m_settingsDialog, SIGNAL(configHasToBeSavedSignal()),this, SLOT(configHasToBeSavedSlot()));
    connect(m_settingsDialog, SIGNAL(conectionTypeChangesSignal()),this, SLOT(conectionTypeChangesSlot()));
    connect(m_settingsDialog, SIGNAL(appendTimestampAtLogsChangedSignal()),this, SLOT(reLogsSlot()));

    qRegisterMetaType<Settings>("Settings");
    connect(this, SIGNAL(connectDataConnectionSignal(Settings, bool)),m_mainInterface,
            SLOT(connectDataConnectionSlot(Settings, bool)), Qt::QueuedConnection);
    connect(m_mainInterface, SIGNAL(dataConnectionStatusSignal(bool, QString, bool)),this, SLOT(dataConnectionStatusSlot(bool, QString, bool)), Qt::QueuedConnection);

    connect(this, SIGNAL(exitThreadSignal()),m_mainInterface, SLOT(exitThreadSlot()), Qt::BlockingQueuedConnection);

    connect(m_mainInterface, SIGNAL(showMessageBoxSignal(QMessageBox::Icon, QString, QString, QMessageBox::StandardButtons )),this, SLOT(showMessageBoxSlot(QMessageBox::Icon, QString, QString, QMessageBox::StandardButtons )), Qt::BlockingQueuedConnection);

    connect(this, SIGNAL(globalSettingsChangedSignal(Settings)),m_mainInterface, SLOT(globalSettingsChangedSlot(Settings)), Qt::QueuedConnection);

    connect(m_settingsDialog, SIGNAL(htmlLogActivatedSignal(bool)),
            this, SLOT(htmLogActivatedSlot(bool)));

    connect(m_settingsDialog, SIGNAL(textLogActivatedSignal(bool)),
            this, SLOT(textLogActivatedSlot(bool)));
    connect(m_settingsDialog, SIGNAL(customLogActivatedSignal(bool)),
            this, SLOT(customLogActivatedSlot(bool)));

    connect(m_settingsDialog, SIGNAL(customConsoleSettingsChangedSignal()),
            this, SLOT(customConsoleSettingsChangedSlot()));
    connect(m_settingsDialog, SIGNAL(customLogSettingsChangedSignal()),
            this, SLOT(customLogSettingsChangedSlot()));

    connect(m_userInterface->tabWidget, SIGNAL(currentChanged(int)),
            this, SLOT(tabIndexChangedSlot(int)));

    connect(m_userInterface->ToolboxSplitter, SIGNAL(splitterMoved(int,int)),
            this, SLOT(toolBoxSplitterMoved(int,int)));

    connect(m_userInterface->SendAreaSplitter, SIGNAL(splitterMoved(int,int)),
            this, SLOT(sendAreaSplitterMoved(int,int)));

    connect(m_userInterface->SendAreaInputsSplitter, SIGNAL(splitterMoved(int,int)),
            this, SLOT(sendAreaInputsSplitterMoved(int,int)));

    connect(m_userInterface->toolBox, SIGNAL(currentChanged(int)),
            this, SLOT(currentToolBoxPageChangedSlot(int)));

    connect(m_userInterface->SendTextEdit, SIGNAL(textChanged()), this, SLOT(sendInputTextChangedSlot()));
    connect(m_userInterface->SendTextEdit, SIGNAL(focusOutSignal()), this, SLOT(checkSendInputSlot()));
    connect(m_userInterface->SendFormatComboBox, SIGNAL(currentTextChanged(QString)), this, SLOT(currentSendFormatChangedSlot(QString)));
    connect(m_userInterface->SendPushButton, SIGNAL(clicked()), this, SLOT(sendButtonPressedSlot()));
    connect(m_userInterface->ScriptTextEdit, SIGNAL(textChanged()), this, SLOT(scriptTextEditSlot()));
    connect(m_userInterface->ScriptTextEdit, SIGNAL(doubleClickSignal()), this, SLOT(scriptTextEditDoubleClickedSlot()));


    connect(m_userInterface->rtsCheckBox, SIGNAL(clicked()),this, SLOT(serialPortPinsChangedSlot()), Qt::QueuedConnection);
    connect(m_userInterface->dtrCheckBox, SIGNAL(clicked()),this, SLOT(serialPortPinsChangedSlot()), Qt::QueuedConnection);


    connect(m_userInterface->startIndexSpinBox, SIGNAL(valueChanged(int)),this, SLOT(historyStartIndexChangedSlot(int)), Qt::QueuedConnection);
    connect(m_userInterface->endIndexSpinBox, SIGNAL(valueChanged(int)),this, SLOT(historyEndIndexChangedSlot(int)), Qt::QueuedConnection);
    connect(m_userInterface->historyFormatComboBox, SIGNAL(currentTextChanged(QString)),this, SLOT(historyFormatChanged(QString)), Qt::QueuedConnection);
    connect(m_userInterface->sendHistoryPushButton, SIGNAL(pressed()),this, SLOT(sendHistoryButtonSlot()), Qt::QueuedConnection);
    connect(m_userInterface->clearHistoryPushButton, SIGNAL(pressed()),this, SLOT(clearHistoryButtonSlot()), Qt::QueuedConnection);
    connect(m_userInterface->createScriptPushButton, SIGNAL(pressed()),this, SLOT(createScriptButtonSlot()), Qt::QueuedConnection);

    m_scriptWindow = new ScriptWindow(this, m_mainInterface, m_commandLineScripts);
    connect(m_scriptWindow, SIGNAL(configHasToBeSavedSignal()),this, SLOT(configHasToBeSavedSlot()));
    connect(m_scriptWindow->getCreateSceFileDialog(), SIGNAL(configHasToBeSavedSignal()),this, SLOT(configHasToBeSavedSlot()));
    connect(m_scriptWindow, SIGNAL(scriptTableHasChangedSignal()),this, SLOT(setUpScriptPageSlot()));

    connect(m_userInterface->sequenceListWidget, SIGNAL(itemDoubleClicked(QListWidgetItem*)),this, SLOT(sequenceListItemDoubleClickedSlot(QListWidgetItem*)), Qt::QueuedConnection);
    connect(m_userInterface->workerScriptListWidget, SIGNAL(itemDoubleClicked(QListWidgetItem*)),this, SLOT(workerScriptListItemDoubleClickedSlot(QListWidgetItem*)), Qt::QueuedConnection);
    connect(m_userInterface->workerScriptListWidget, SIGNAL(currentRowChanged(int)),this, SLOT(workerScriptsCurrentRowChangedSlot(int)), Qt::QueuedConnection);
    connect(m_userInterface->startWorkerScriptsPushButton, SIGNAL(clicked()),this, SLOT(startScriptButtonPressedSlot()));
    connect(m_userInterface->pauseWorkerScriptPushButton, SIGNAL(clicked()),this, SLOT(pauseScriptButtonPressedSlot()));


    m_searchConsole = new SearchConsole(this);

    connect(&m_handleData->m_historyConsoleTimer, SIGNAL(timeout()),m_handleData, SLOT(historyConsoleTimerSlot()), Qt::QueuedConnection);
    connect(&m_handleData->m_sendHistoryTimer, SIGNAL(timeout()),m_handleData, SLOT(sendHistoryTimerSlot()), Qt::QueuedConnection);
    connect(&m_handleData->m_checkDebugWindowsIsClosed, SIGNAL(timeout()),m_handleData, SLOT(checkDebugWindowsIsClosedSlot()), Qt::QueuedConnection);

    if(m_commandLineScripts.isEmpty())
    {
        connect(m_mainInterface, SIGNAL(dataReceivedSignal(QByteArray)),m_handleData, SLOT(dataReceivedSlot(QByteArray)), Qt::QueuedConnection);
        connect(m_mainInterface, SIGNAL(canMessagesReceivedSignal(QVector<QByteArray>)),m_handleData, SLOT(canMessagesReceivedSlot(QVector<QByteArray>)), Qt::QueuedConnection);
        connect(m_mainInterface, SIGNAL(sendingFinishedSignal(QByteArray, bool, uint)),m_handleData, SLOT(dataHasBeenSendSlot(QByteArray, bool, uint)), Qt::QueuedConnection);

        connect(m_mainInterface, SIGNAL(sendingFinishedSignal(bool, uint)),m_sendWindow, SLOT(dataHasBeenSendSlot(bool, uint)), Qt::QueuedConnection);
        connect(m_mainInterface, SIGNAL(dataRateUpdateSignal(quint32,quint32)),this, SLOT(dataRateUpdateSlot(quint32,quint32)), Qt::QueuedConnection);

        m_mainConfigFileList = getAndCreateProgramUserFolder() + "/mainConfigFileList.txt";

        QString defaultConfig;
        QStringList list = readMainConfigFileList(false);
        if(configFile.isEmpty())
        {
            for(auto el : list)
            {
                if(el.indexOf("<DEFAULT_CONFIG_FILE>:") != -1)
                {
                    defaultConfig = el;
                    defaultConfig.remove("<DEFAULT_CONFIG_FILE>:");
                    break;
                }
            }
        }
        else
        {
            defaultConfig = configFile;

            QStringList list = readMainConfigFileList(true);
            int index = list.indexOf("<DEFAULT_CONFIG_FILE>:" + configFile);
            if(index == -1)
            {//configFile is not the default config.

                index = list.indexOf(configFile);
                if(index != -1)
                {
                    list.removeAt(index);
                }
                list.push_front(configFile);
            }
            saveMainConfigFileList(list);
        }

        if(defaultConfig.isEmpty())
        {
            if(list.length() == 0)
            {
                m_isFirstProgramStart = true;
                m_mainConfigFile = getAndCreateProgramUserFolder() + "/" + INIT_MAIN_CONFIG_FILE;

                QString templateFile = getScriptCommunicatorFilesFolder() + "/templates/" + INIT_MAIN_CONFIG_FILE;
                QFile::copy(templateFile, m_mainConfigFile);

                QFile::copy(getScriptCommunicatorFilesFolder() + "/templates/initalSequences.seq",
                            getAndCreateProgramUserFolder() + "/initalSequences.seq");

                QFile::copy(getScriptCommunicatorFilesFolder() + "/templates/initalScripts.scripts",
                            getAndCreateProgramUserFolder() + "/initalScripts.scripts");

                list.push_back(m_mainConfigFile);
                saveMainConfigFileList(list);
            }
            else if(list.length() == 1)
            {
                m_mainConfigFile = list.at(0);
            }
            else
            {
                bool okPressed;
                QString result = QInputDialog::getItem(this, "select ScriptCommunicator config file","Note: The selected config can be made to default (config menu)",
                                                       list, 0, false, &okPressed, Qt::WindowStaysOnTopHint);

                if(okPressed)
                {
                    int index = list.indexOf(result);
                    if(index != -1)
                    {
                        list.removeAt(index);
                    }
                    list.push_front(result);
                    saveMainConfigFileList(list);

                    m_mainConfigFile = result;
                }
                else
                {
                    m_mainConfigFile = getAndCreateProgramUserFolder() + "/" + INIT_MAIN_CONFIG_FILE;
                }

            }
        }
        else
        {
            m_mainConfigFile = defaultConfig;
        }

        if(m_mainConfigFile.isEmpty())
        {
            m_mainConfigFile = getAndCreateProgramUserFolder() + "/" + INIT_MAIN_CONFIG_FILE;
        }

        m_userInterface->ReceiveTextEditAscii->setWordWrapMode (QTextOption::WrapAnywhere);

        QFileInfo fi(m_mainConfigFile + ".lock");
        bool lockFileExists = fi.exists();

        m_mainConfigLockFile.setFileName(m_mainConfigFile + ".lock");
        m_mainConfigLockFile.open(QIODevice::WriteOnly);
        connect(&m_configLockFileTimer, SIGNAL(timeout()),this, SLOT(configLockFileTimerSlot()), Qt::QueuedConnection);
        m_configLockFileTimer.start(2000);

        bool loadConfig = true;

        if(lockFileExists)
        {//The main config file is locked

            bool yesButtonPressed = false;
            showYesNoDialogSlot(QMessageBox::Warning, "config file is locked", m_mainConfigFile + " is already in use.\nCreate a new config?",
                                0, &yesButtonPressed);
            if(yesButtonPressed)
            {
                if(createConfig(false))
                {
                    loadConfig = false;
                }
            }

        }//if(fi.exists())

        if(loadConfig)
        {
            loadSettings();
            if(m_isFirstProgramStart)
            {
                saveSettings();
            }
            inititializeTab();
            conectionTypeChangesSlot();
        }



    }//if(m_commandLineScripts.isEmpty())
    else
    {//Command line mode.
        m_settingsDialog->updateSettings();
        Settings settings = *m_settingsDialog->settings();
        settings.showReceivedDataInConsole = false;
        settings.showSendDataInConsole = false;
        settings.generateTimeStampsInConsole = false;
        settings.htmlLogFile = false;
        settings.textLogFile = false;

        m_settingsDialog->setAllSettingsSlot(settings, true);

        m_scriptWindow->startCommandLineScripts();

        if(withScriptWindow)
        {
            m_scriptWindow->show();
            QApplication::setActiveWindow(m_scriptWindow);

            if(scriptWindowIsMinimized)
            {
                connect(&m_commandLineModeMinimizeTimer, SIGNAL(timeout()),this, SLOT(commandLineModeMinimizeTimerSlot()), Qt::QueuedConnection);
                m_commandLineModeMinimizeTimer.start(500);
            }
        }

    }

#ifdef Q_OS_WIN32
        QString name = m_mainConfigFile.isEmpty() ? QString("%1").arg(QDateTime::currentMSecsSinceEpoch()) : m_mainConfigFile;
        //Specifiy a unique application-defined Application User Model ID (AppUserModelID) that identifies the current process to the taskbar.
        //Note: Because of this id every instance of ScriptCommunicator has its own icon group in the task bar.
        QtWin::setCurrentProcessExplicitAppUserModelID("ScriptCommunicator_" + name);
#endif

    if(!iconFile.isEmpty())
    {
        setMainWindowAndTaskBarIconSlot(iconFile);
    }

}


/**
 * Destructor
 */
MainWindow::~MainWindow()
{
    m_mainConfigLockFile.close();
    QFile::remove(m_mainConfigLockFile.fileName());

    delete m_scriptWindow;
    delete m_handleData;
    delete m_settingsDialog;
    delete m_userInterface;
    delete m_sendWindow;
    delete m_mainInterface;
    delete m_addMessageDialog;
    delete m_searchConsole;
}

/**
 * Sets the main window and the ScriptCommunicator task bar icon.
 * Supported formats: .ico, .gif, .png, .jpeg, .tiff, .bmp, .icns.
 * @param iconFile
 *      The file name of the icon.
 */
void MainWindow::setMainWindowAndTaskBarIconSlot(QString iconFile)
{
    setWindowIcon(QIcon(iconFile));
    qApp->setWindowIcon(QIcon(iconFile));
}

/**
 * Slot for the config file lock file timer.
 */
void MainWindow::configLockFileTimerSlot()
{
    m_mainConfigLockFile.close();
    m_mainConfigLockFile.open(QIODevice::WriteOnly);
}

/**
  * This slot function is called if the user changes the text of the send text edit box.
  * @param text
  *         The new text.
  */
void MainWindow::sendInputTextChangedSlot(void)
{
    m_sendWindow->textEditChanged(m_userInterface->SendTextEdit, m_userInterface->SendFormatComboBox->currentText(),
                                  m_sendWindow->formatToDecimalType(m_userInterface->SendFormatComboBox->currentText()));

    if(!m_userInterface->ScriptTextEdit->toPlainText().isEmpty() &&
            !m_userInterface->SendTextEdit->toPlainText().isEmpty())
    {
        m_userInterface->actionDebugSequenceScript->setEnabled(true);
    }
    else
    {
        m_userInterface->actionDebugSequenceScript->setEnabled(false);
    }
}

/**
 * Is called if the history start index has been changed.
 * @param value
 *      The new value
 */
void MainWindow::historyStartIndexChangedSlot(int value)
{
    (void)value;
    m_handleData->historyConsoleTimerSlot();
    saveSettings();
}

/**
 * Is called if the history end index has been changed.
 * @param value
 *      The new value
 */
void MainWindow::historyEndIndexChangedSlot(int value)
{
    (void)value;
    m_handleData->historyConsoleTimerSlot();
    saveSettings();
}

/**
 * Is called if the history format has been changed.
 * @param value
 *      The new value
 */
void MainWindow::historyFormatChanged(QString value)
{
    (void)value;
    m_handleData->historyConsoleTimerSlot();
    saveSettings();
}

/**
 * Slot function for the send button.
 * @param debug
 *      True if the script shall be executed in the script debugger.
 */
void MainWindow::sendButtonPressedSlot(bool debug)
{
    checkSendInputSlot();

    const Settings* settings = m_settingsDialog->settings();
    QByteArray sendData = m_sendWindow->textToByteArray(m_userInterface->SendFormatComboBox->currentText(), m_userInterface->SendTextEdit->toPlainText(),
                                                        m_sendWindow->formatToDecimalType(m_userInterface->SendFormatComboBox->currentText()), settings->targetEndianess);
    if(m_userInterface->SendFormatComboBox->currentText() == "ascii")
    {
        const Settings* settings = m_settingsDialog->settings();
        sendData.replace("\n", settings->consoleSendOnEnter.toLocal8Bit());
    }
    if(!sendData.isEmpty())
    {
        m_sendWindow->sendDataWithTheMainInterface(sendData, this, 0, 0, false, m_userInterface->ScriptTextEdit->toPlainText(), debug);
    }
}

/**
 * Checks if the text in the send text edit box has the correct format.
 */
void MainWindow::checkSendInputSlot(void)
{
    m_sendWindow->checkTextEditContent(m_userInterface->SendTextEdit, m_userInterface->SendFormatComboBox->currentText());
}

/**
 * Is called if the user presses the clear history button.
 */
void MainWindow::clearHistoryButtonSlot()
{
    m_userInterface->clearHistoryPushButton->setEnabled(false);
    m_userInterface->sendHistoryPushButton->setEnabled(false);
    m_userInterface->createScriptPushButton->setEnabled(false);

    m_userInterface->startIndexSpinBox->blockSignals(true);
    m_userInterface->endIndexSpinBox->blockSignals(true);

    m_userInterface->startIndexSpinBox->setMaximum(0);
    m_userInterface->endIndexSpinBox->setMaximum(0);

    m_userInterface->startIndexSpinBox->setValue(0);
    m_userInterface->endIndexSpinBox->setValue(0);

    m_handleData->m_sendHistory.clear();
    m_userInterface->historyTextEdit->clear();

    m_userInterface->startIndexSpinBox->blockSignals(false);
    m_userInterface->endIndexSpinBox->blockSignals(false);

    saveSettings();
}

/**
 * Is called if the user presses the send history button.
 */
void MainWindow::sendHistoryButtonSlot()
{
    if(m_handleData->m_historySendIsInProgress)
    {
        m_handleData->cancelSendHistory();
    }
    else
    {
        if(m_isConnected)
        {
            m_handleData->sendHistory();
        }
        else
        {
            QMessageBox::critical(this, "error", "sending failed: no connection");
        }

    }

}

/**
 * Is called if the user presses the create script button (send history).
 */
void MainWindow::createScriptButtonSlot()
{
    QString createdScriptFileName = QFileDialog::getSaveFileName(this, tr("select a filename"),
                                                                 "", tr("worker script files (*.js);;Files (*)"));
    if(!createdScriptFileName.isEmpty())
    {

        QString templateFile = getScriptCommunicatorFilesFolder() + "/templates/workerScriptTemplates/worker_script_template_send_history.js";
        QFile scriptFile(templateFile);
        if (scriptFile.open(QIODevice::ReadOnly))
        {
            QString content = scriptFile.readAll();
            scriptFile.close();

            //Set the new filename.
            scriptFile.setFileName(createdScriptFileName);

            content.replace("@SendPause@", QString("%1").arg(m_userInterface->sendPauseSpinBox->value()));

            QFileInfo fileInfo(scriptFile.fileName());
            content.replace("@ScriptName@", fileInfo.fileName());

            QString elementsString;
            qint32 endIndex = m_userInterface->endIndexSpinBox->value();
            qint32 startIndex = m_userInterface->startIndexSpinBox->value();
            quint32 arrayIndex = 0;

            //Creates on entry in the script sendElements array.
            auto createElementString = [](quint32 arrayIndex, QByteArray* data)
            {
                QString elementString = QString("sendElements[%1] = Array(").arg(arrayIndex);
                QString currentElement = byteArrayToNumberString(*data, false , false, false, false, true,
                                                                 DECIMAL_TYPE_UINT8, LITTLE_ENDIAN_TARGET);
                currentElement.replace(" ", ",");
                elementString += currentElement + ");\n";
                return elementString;
            };

            //Create all sendElements array entries.
            if(startIndex < endIndex)
            {
                for(qint32 i = startIndex; (i <= endIndex) && (i < m_handleData->m_sendHistory.length()); i++)
                {
                    elementsString +=createElementString(arrayIndex, &m_handleData->m_sendHistory[i]);
                    arrayIndex++;
                }
            }
            else
            {
                for(qint32 i = startIndex; (i >= endIndex) && (i < m_handleData->m_sendHistory.length()); i--)
                {
                    elementsString +=createElementString(arrayIndex, &m_handleData->m_sendHistory[i]);
                    arrayIndex++;
                }
            }
            content.replace("@SendElementsInit@", elementsString);

            (void)scriptFile.remove();
            if(scriptFile.open(QIODevice::WriteOnly))
            {
                if(scriptFile.write(content.toLocal8Bit()))
                {
                    m_scriptWindow->addScript(scriptFile.fileName());
                    QMessageBox::information(this, "information", QString("%1 has been created and added to the scripts config (script window and script tab in the main window)").arg(createdScriptFileName));
                }
                else
                {
                    QMessageBox::critical(this, "error", QString("could not write to %1").arg(createdScriptFileName));
                }
                scriptFile.close();
            }
            else
            {
                QMessageBox::critical(this, "error", QString("could not open %1").arg(createdScriptFileName));
            }

        }//if (scriptFile.open(QIODevice::ReadWrite))

    }//if(!createdScriptFileName.isEmpty())
}

/**
 * This slot is called if the value of the send format combobox has been changed.
 * @param format
 *      The new format.
 */
void MainWindow::currentSendFormatChangedSlot(QString format)
{

    if(!m_userInterface->SendTextEdit->toPlainText().isEmpty())
    {
        QString newText = m_sendWindow->formatComboBoxChanged(m_userInterface->SendTextEdit, format, m_oldSendFormat);
        m_userInterface->SendTextEdit->setPlainText(newText);
    }

    m_oldSendFormat = format;
}
/**
 * Timer slot function for minimizing the script window (command line mode).
 */
void MainWindow::commandLineModeMinimizeTimerSlot()
{
    m_scriptWindow->setWindowState(Qt::WindowMinimized);
    m_commandLineModeMinimizeTimer.stop();
}

/**
 * Is called if an item in the sequence list has been double clicked.
 * @param item
 *      The clicked item.
 */
void MainWindow::sequenceListItemDoubleClickedSlot(QListWidgetItem *item)
{
    bool isOK = false;
    quint32 row = item->data(1).toUInt(&isOK);
    if(row != 0xffffffff)
    {
        m_sendWindow->sendSequence(row, false, this);
    }
}

/**
 * Is called if an item in the worker script list has been double clicked.
 * @param item
 *      The clicked item.
 */
void MainWindow::workerScriptListItemDoubleClickedSlot(QListWidgetItem *item)
{
    bool isOK = false;
    quint32 row = item->data(1).toUInt(&isOK);
    if(row != 0xffffffff)
    {
        if(m_scriptWindow->getScriptState(row) == SUSPENDED_BY_DEBUGGER)
        {
            //Do nothing.
        }
        else if(m_scriptWindow->getScriptState(row) == RUNNING)
        {
            m_scriptWindow->stopScriptThread(row);
        }
        else
        {
            m_scriptWindow->startScriptThread(row);
        }
    }

}

/**
 * Create all buttons in the sequence page.
 */
void MainWindow::setUpSequencesPageSlot(void)
{
    QStringList sequences = m_sendWindow->getAllSequences();

    m_userInterface->sequenceListWidget->clear();

    if(!sequences.isEmpty())
    {
        for(auto el : sequences)
        {
            m_userInterface->sequenceListWidget->addItem(" " + el);
            quint32 row = m_userInterface->sequenceListWidget->count() - 1;
            QListWidgetItem *item = m_userInterface->sequenceListWidget->item(row);
            item->setData(1, row);
            item->setToolTip("sequences can be added/modified in the send window,\nto send a sequence double click on it");
        }

    }
    else
    {
        m_userInterface->sequenceListWidget->addItem(" no sequences");
        QListWidgetItem *item = m_userInterface->sequenceListWidget->item(0);
        item->setData(1, 0xffffffff);
        item->setToolTip("sequences can be added/modified in the send window,\nto send a sequence double click on it");
    }
}

/**
 * Create all buttons in the script page.
 */
void MainWindow::setUpScriptPageSlot(void)
{
    QStringList scriptNamesAndState = m_scriptWindow->getAllScriptNamesAndStates();


    m_userInterface->startWorkerScriptsPushButton->setEnabled(false);
    m_userInterface->pauseWorkerScriptPushButton->setEnabled(false);

    QList<QListWidgetItem*> items = m_userInterface->workerScriptListWidget->selectedItems();
    quint32 selectedRow = 0xffffffff;

    if(!items.isEmpty())
    {
        bool isOK = false;
        selectedRow = items[0]->data(1).toUInt(&isOK);
    }

    m_userInterface->workerScriptListWidget->clear();


    if(!scriptNamesAndState.isEmpty())
    {
        for(qint32 i = 0; (i + 1) < scriptNamesAndState.size(); i +=2)
        {
            m_userInterface->workerScriptListWidget->addItem(" " + scriptNamesAndState[i]);
            quint32 row = m_userInterface->workerScriptListWidget->count() - 1;
            QListWidgetItem *item = m_userInterface->workerScriptListWidget->item(row);
            item->setToolTip("scripts can be added/modified in the script window");
            item->setData(1, row);

            QFont font = item->font();
            if(scriptNamesAndState[i + 1] == "running")
            {
                font.setBold(true);
                font.setItalic(false);
            }
            else  if(scriptNamesAndState[i + 1] == "paused")
            {
                font.setBold(true);
                font.setItalic(true);
            }
            else  if(scriptNamesAndState[i + 1] == "suspended")
            {
                font.setBold(true);
                font.setItalic(true);
            }
            else
            {
                font.setBold(false);
                font.setItalic(false);
            }

            item->setFont(font);
        }

        if(selectedRow != 0xffffffff)
        {
            QListWidgetItem *item = m_userInterface->workerScriptListWidget->item(selectedRow);
            if(item)
            {
                item->setSelected(true);
                workerScriptsCurrentRowChangedSlot(selectedRow);
            }
        }
    }
    else
    {
        m_userInterface->workerScriptListWidget->addItem(" no scripts");
        QListWidgetItem *item = m_userInterface->workerScriptListWidget->item(0);
        item->setData(1, 0xffffffff);
        item->setToolTip("scripts can be added/modified in the script window");
        workerScriptsCurrentRowChangedSlot(0);
        m_userInterface->workerScriptListWidget->clearSelection();
    }
}

///Is called if the user presses the pause script button.
void MainWindow::pauseScriptButtonPressedSlot(void)
{
    QList<QListWidgetItem*> items = m_userInterface->workerScriptListWidget->selectedItems();
    if(!items.isEmpty())
    {
        QFont font = items[0]->font();
        bool isOK = false;
        quint32 index = items[0]->data(1).toUInt(&isOK);

        if(index != 0xffffffff)
        {
            if(font.bold() && !font.italic())
            {//Script is running
                m_scriptWindow->pauseScriptThread(index);
            }
            else if(!font.bold() && !font.italic())
            {//Script is stopped.

                m_scriptWindow->startScriptThread(index, true);
            }
        }

    }
}

/**
 * Is called if the the selected row in the worker script list has been changed.
 * @param currentRow
 *      The current row.
 */
void MainWindow::workerScriptsCurrentRowChangedSlot(int currentRow)
{
    QListWidgetItem* item = m_userInterface->workerScriptListWidget->item(currentRow);
    if(item)
    {
        bool isOK = false;
        quint32 index = item->data(1).toUInt(&isOK);
        if(index != 0xffffffff)
        {
            m_userInterface->startWorkerScriptsPushButton->setEnabled(true);


            if(m_scriptWindow->getScriptState(index) == PAUSED)
            {//Script has been paused.

                m_userInterface->startWorkerScriptsPushButton->setText("resume");
                m_userInterface->pauseWorkerScriptPushButton->setEnabled(false);
                m_userInterface->pauseWorkerScriptPushButton->setText("pause");

            }
            else if(m_scriptWindow->getScriptState(index) == RUNNING)
            {//Script is running

                m_userInterface->startWorkerScriptsPushButton->setText("stop");
                m_userInterface->pauseWorkerScriptPushButton->setEnabled(true);
                m_userInterface->pauseWorkerScriptPushButton->setText("pause");
            }
            else if(m_scriptWindow->getScriptState(index) == SUSPENDED_BY_DEBUGGER)
            {//Script is suspended

                m_userInterface->pauseWorkerScriptPushButton->setEnabled(false);
                m_userInterface->startWorkerScriptsPushButton->setEnabled(false);
            }
            else
            {//Scripts is stopped.

                m_userInterface->startWorkerScriptsPushButton->setText("start");
                m_userInterface->pauseWorkerScriptPushButton->setEnabled(true);
                m_userInterface->pauseWorkerScriptPushButton->setText("debug");

            }
        }
    }
}

/**
 * Is called if the user presses a script button.
 */
void MainWindow::startScriptButtonPressedSlot(void)
{
    QList<QListWidgetItem*> items = m_userInterface->workerScriptListWidget->selectedItems();
    if(!items.isEmpty())
    {
        QFont font = items[0]->font();
        bool isOK = false;
        quint32 index = items[0]->data(1).toUInt(&isOK);
        if(index != 0xffffffff)
        {
            if(font.bold() && font.italic())
            {//Script has been paused.

                m_scriptWindow->startScriptThread(index);
            }
            else if(font.bold())
            {//Script is running
                m_scriptWindow->stopScriptThread(index);
            }
            else
            {//Scripts is stopped.
                m_scriptWindow->startScriptThread(index);
            }
        }
    }
}

/**
 * Disables all mouse events for all windows.
 */
void MainWindow::disableMouseEventsSlot()
{
    if(m_mouseGrabWidget == 0)
    {
        m_mouseGrabWidget = new QWidget(this);
    }
    m_mouseGrabWidget->grabMouse();

}

/**
 * Enables all mouse events for all windows.
 */
void MainWindow::enableMouseEventsSlot()
{
    m_mouseGrabWidget->releaseMouse();
}


/**
 * Write an XML element and his attributes to the XML stream.
 * @param xmlWriter
 *      The XML stream.
 * @param elementName
 *      The element name.
 * @param attributes
 *      The attributes.
 */
void MainWindow::writeXmlElement(QXmlStreamWriter& xmlWriter, QString elementName, std::map<QString, QString>& attributes)
{
    xmlWriter.writeStartElement(elementName);

    for(auto iter : attributes)
    {
        xmlWriter.writeAttribute(iter.first, iter.second);
    }
    xmlWriter.writeEndElement();

}


/**
 * Shows the send window.
 */
void MainWindow::show(void)
{
    QWidget::show();

    if(m_sendWindow->isVisible())
    {
        //Bring the window to foreground.
        m_sendWindow->raise();
    }
    if(m_scriptWindow->isVisible())
    {
        //Bring the window to foreground.
        m_scriptWindow->raise();
    }

    if(m_settingsDialog->isVisible())
    {
        //Bring the window to foreground.
        m_settingsDialog->raise();
    }
    if(m_scriptWindow->getCreateSceFileDialog()->isVisible())
    {
        //Bring the window to foreground.
        m_scriptWindow->getCreateSceFileDialog()->raise();
    }

    QSize tmpSize = m_userInterface->historyFormatComboBox->frameSize();
    m_userInterface->createScriptPushButton->setMinimumSize(tmpSize);
    m_userInterface->sendHistoryPushButton->setMinimumSize(tmpSize);
    m_userInterface->clearHistoryPushButton->setMinimumSize(tmpSize);

    m_handleData->reInsertDataInMixecConsoleSlot();
}

/**
 * Loads the main configuration.
 *
 * return
 *      True on success.
 */
bool MainWindow::loadSettings()
{
    bool result = true;

    if(m_commandLineScripts.isEmpty())
    {
        m_scriptWindow->stopAllScripts();

        while(!m_scriptWindow->allScriptsHaveBeenStopped())
        {
            QThread::msleep(10);
            QCoreApplication::processEvents();
        }

        setWindowTitle("ScriptCommunicator " + MainWindow::VERSION + "   " + m_mainConfigFile);

        QFile settingsFile(m_mainConfigFile);
        m_settingsDialog->updateSettings();
        Settings currentSettings = *m_settingsDialog->settings();
        currentSettings.serialPort.setDTR = true;
        currentSettings.serialPort.setRTS = false;
        bool showSendWindow = false;
        bool showSettingWindow = false;

        QDomDocument doc("settings");
        bool isConnected = false;
        bool scriptWindowIsVisible = false;
        bool createSceWindowIsVisible = false;

        QList<int> splitterSizes = m_userInterface->ToolboxSplitter->sizes();
        m_toolBoxSplitterSizeSecond = splitterSizes[1];
        m_toolBoxSplitterSizesSecond.clear();
        m_toolBoxSplitterSizesSecond.append(m_toolBoxSplitterSizeSecond);
        m_toolBoxSplitterSizesSecond.append(m_toolBoxSplitterSizeSecond);
        m_toolBoxSplitterSizesSecond.append(m_toolBoxSplitterSizeSecond);
        m_toolBoxSplitterSizesSecond.append(m_toolBoxSplitterSizeSecond);
        m_currentToolBoxIndex = m_userInterface->toolBox->currentIndex();

        if (!settingsFile.open(QIODevice::ReadOnly))
        {
            QMessageBox::critical(this, "open error", "could not open " + m_mainConfigFile);
            result = false;
        }
        else
        {
            QByteArray content = settingsFile.readAll();
            settingsFile.close();
            if (!doc.setContent(content))
            {
                QMessageBox::critical(this, "parse error", "could not parse " + m_mainConfigFile);
                result = false;
            }
            else
            {
                QDomElement docElem = doc.documentElement();

                {
                    QDomNodeList nodeList = docElem.elementsByTagName("consoleSettings");
                    if(!nodeList.isEmpty())
                    {
                        QDomNode node = nodeList.at(0);
                        currentSettings.maxCharsInConsole = node.attributes().namedItem("maxCharsInConsole").nodeValue().toUInt();
                        currentSettings.generateTimeStampsInConsole = node.attributes().namedItem("generateTimeStampsInConsole").nodeValue().toUInt();
                        currentSettings.showSendDataInConsole = node.attributes().namedItem("showSendDataInConsole").nodeValue().toUInt();
                        currentSettings.showReceivedDataInConsole = node.attributes().namedItem("showReceivedDataInConsole").nodeValue().toUInt();
                        currentSettings.lockScrollingInConsole = node.attributes().namedItem("lockScrollingInConsole").nodeValue().toUInt();
                        currentSettings.stringConsoleFont  = node.attributes().namedItem("stringConsoleFont").nodeValue();
                        currentSettings.stringConsoleFontSize  = node.attributes().namedItem("stringConsoleFontSize").nodeValue();
                        currentSettings.showDecimalInConsole = node.attributes().namedItem("showDecimalInConsole").nodeValue().toUInt();
                        currentSettings.showHexInConsole = node.attributes().namedItem("showHexInConsole").nodeValue().toUInt();
                        currentSettings.timeStampIntervalConsole = node.attributes().namedItem("timeStampIntervalConsole").nodeValue().toUInt();
                        currentSettings.updateIntervalConsole = node.attributes().namedItem("updateIntervalConsole").nodeValue().toUInt();
                        currentSettings.showMixedConsole = node.attributes().namedItem("showMixedConsole").nodeValue().toUInt();
                        currentSettings.showBinaryConsole = node.attributes().namedItem("showBinaryConsole").nodeValue().toUInt();
                        currentSettings.showCanMetaInformationInConsole = node.attributes().namedItem("showCanMetaInformationInConsole").nodeValue().toUInt();
                        currentSettings.showCanTab = node.attributes().namedItem("showCanTab").nodeValue().toUInt();
                        currentSettings.consoleReceiveColor = node.attributes().namedItem("consoleReceiveColor").nodeValue();
                        currentSettings.consoleSendColor = node.attributes().namedItem("consoleSendColor").nodeValue();
                        currentSettings.consoleBackgroundColor = node.attributes().namedItem("consoleBackgroundColor").nodeValue();
                        currentSettings.consoleMessageAndTimestampColor = node.attributes().namedItem("consoleMessageAndTimestampColor").nodeValue();
                        currentSettings.consoleMixedAsciiColor = node.attributes().namedItem("consoleMixedAsciiColor").nodeValue();
                        currentSettings.consoleMixedDecimalColor = node.attributes().namedItem("consoleMixedDecimalColor").nodeValue();
                        currentSettings.consoleMixedHexadecimalColor = node.attributes().namedItem("consoleMixedHexadecimalColor").nodeValue();
                        currentSettings.consoleMixedBinaryColor = node.attributes().namedItem("consoleMixedBinaryColor").nodeValue();
                        currentSettings.consoleNewLineAfterBytes = node.attributes().namedItem("consoleNewLineAfterBytes").nodeValue().toUInt();
                        currentSettings.consoleNewLineAfterPause = node.attributes().namedItem("consoleNewLineAfterPause").nodeValue().toUInt();
                        currentSettings.consoleSendOnEnter = node.attributes().namedItem("consoleSendOnEnter").nodeValue();
                        currentSettings.consoleTimestampFormat = node.attributes().namedItem("consoleTimestampFormat").nodeValue();
                        currentSettings.consoleTimestampFormat = currentSettings.consoleTimestampFormat.isEmpty() ? " \\nyyyy-MM-dd hh:mm:ss.zzz\\n" : currentSettings.consoleTimestampFormat;
                        currentSettings.consoleShowCustomConsole = node.attributes().namedItem("consoleShowCustomConsole").nodeValue().toUInt();
                        currentSettings.consoleScript = node.attributes().namedItem("consoleScript").nodeValue();
                        currentSettings.consoleCreateTimestampAt = node.attributes().namedItem("consoleCreateTimestampAt").nodeValue().toUInt();
                        currentSettings.consoleDecimalsType = (DecimalType)node.attributes().namedItem("consoleDecimalsType").nodeValue().toUInt();

                        if(m_isFirstProgramStart || currentSettings.consoleScript.isEmpty())
                        {
                            currentSettings.consoleScript = getScriptCommunicatorFilesFolder() +
                                    "/exampleScripts/CustomLogConsoleScripts/CustomConsole_Table/CustomConsole_Table.js";
                        }
                        else
                        {
                           currentSettings.consoleScript = convertToAbsolutePath(m_mainConfigFile, currentSettings.consoleScript);
                        }

                        if(node.attributes().namedItem("consoleTimestampAt").nodeValue() != "")
                        {
                            currentSettings.consoleTimestampAt = (quint16)node.attributes().namedItem("consoleTimestampAt").nodeValue().toUInt();
                        }
                        else
                        {
                            //Time stamp at LF.
                            currentSettings.consoleTimestampAt = 10;
                        }


                        if(node.attributes().namedItem("consoleNewLineAt").nodeValue() != "")
                        {
                            currentSettings.consoleNewLineAt = (quint16)node.attributes().namedItem("consoleNewLineAt").nodeValue().toUInt();
                        }
                        else
                        {
                            //New line at LF.
                            currentSettings.consoleNewLineAt = 10;
                        }

                        if(node.attributes().namedItem("logNewLineAt").nodeValue() != "")
                        {
                            currentSettings.logNewLineAt = (quint16)node.attributes().namedItem("logNewLineAt").nodeValue().toUInt();
                        }
                        else
                        {
                            //New line at LF.
                            currentSettings.logNewLineAt = 10;
                        }

                        if(node.attributes().namedItem("showAsciiInConsole").nodeValue() != "")
                        {
                            currentSettings.showAsciiInConsole = node.attributes().namedItem("showAsciiInConsole").nodeValue().toUInt();
                        }
                        else
                        {
                            currentSettings.showAsciiInConsole = true;
                        }
                    }
                }
                {//log settings

                    QDomNodeList nodeList = docElem.elementsByTagName("logSettings");
                    if(!nodeList.isEmpty())
                    {
                        QDomNode node = nodeList.at(0);
                        currentSettings.htmlLogFile = node.attributes().namedItem("htmlLogFile").nodeValue().toUInt();
                        currentSettings.htmlLogfileName = node.attributes().namedItem("htmlLogFileName").nodeValue();
                        currentSettings.textLogFile = node.attributes().namedItem("textLogFile").nodeValue().toUInt();
                        currentSettings.textLogfileName = node.attributes().namedItem("textLogFileName").nodeValue();
                        currentSettings.customLogfileName = node.attributes().namedItem("customLogFileName").nodeValue();
                        currentSettings.writeSendDataInToLog = node.attributes().namedItem("writeSendDataInToLog").nodeValue().toUInt();
                        currentSettings.writeReceivedDataInToLog = node.attributes().namedItem("writeReceivedDataInToLog").nodeValue().toUInt();
                        currentSettings.generateTimeStampsInLog = node.attributes().namedItem("generateTimeStampsInLog").nodeValue().toUInt();
                        currentSettings.appendTimestampAtLogFileName = node.attributes().namedItem("appendTimestampAtLogFileName").nodeValue().toUInt();
                        currentSettings.stringHtmlLogFont  = node.attributes().namedItem("stringHtmlLogFont").nodeValue();
                        currentSettings.stringHtmlLogFontSize  = node.attributes().namedItem("stringHtmlLogFontSize").nodeValue();
                        currentSettings.writeDecimalInToLog = node.attributes().namedItem("writeDecimalInToLog").nodeValue().toUInt();
                        currentSettings.writeHexInToLog = node.attributes().namedItem("writeHexInToLog").nodeValue().toUInt();
                        currentSettings.writeBinaryInToLog = node.attributes().namedItem("writeBinaryInToLog").nodeValue().toUInt();
                        currentSettings.timeStampIntervalLog = node.attributes().namedItem("timeStampIntervalLog").nodeValue().toUInt();
                        currentSettings.writeCanMetaInformationInToLog = node.attributes().namedItem("writeCanMetaInformationInToLog").nodeValue().toUInt();
                        currentSettings.logNewLineAfterBytes = node.attributes().namedItem("logNewLineAfterBytes").nodeValue().toUInt();
                        currentSettings.logNewLineAfterPause = node.attributes().namedItem("logNewLineAfterPause").nodeValue().toUInt();
                        currentSettings.logTimestampFormat = node.attributes().namedItem("logTimestampFormat").nodeValue();
                        currentSettings.logTimestampFormat = currentSettings.consoleTimestampFormat.isEmpty() ? " \\nyyyy-MM-dd hh:mm:ss.zzz\\n" : currentSettings.logTimestampFormat;
                        currentSettings.logGenerateCustomLog = node.attributes().namedItem("logGenerateCustomLog").nodeValue().toUInt();
                        currentSettings.logScript = node.attributes().namedItem("logScript").nodeValue();
                        currentSettings.logCreateTimestampAt = node.attributes().namedItem("logCreateTimestampAt").nodeValue().toUInt();
                        currentSettings.logDecimalsType = (DecimalType)node.attributes().namedItem("logDecimalsType").nodeValue().toUInt();

                        if(node.attributes().namedItem("logTimestampAt").nodeValue() != "")
                        {
                            currentSettings.logTimestampAt = (quint16)node.attributes().namedItem("logTimestampAt").nodeValue().toUInt();
                        }
                        else
                        {
                            //Time stamp at LF.
                            currentSettings.logTimestampAt = 10;
                        }

                        if(m_isFirstProgramStart || currentSettings.logScript.isEmpty())
                        {
                            currentSettings.logScript = getScriptCommunicatorFilesFolder() +
                                    "/exampleScripts/CustomLogConsoleScripts/CustomConsole_Table/CustomConsole_Table.js";
                        }
                        else
                        {
                            currentSettings.logScript = convertToAbsolutePath(m_mainConfigFile, currentSettings.logScript);
                        }

                        if(node.attributes().namedItem("writeAsciiInToLog").nodeValue() != "")
                        {
                            currentSettings.writeAsciiInToLog = node.attributes().namedItem("writeAsciiInToLog").nodeValue().toUInt();
                        }
                        else
                        {
                            currentSettings.writeAsciiInToLog = true;
                        }

                        m_handleData->m_htmlLogFile.setFileName(currentSettings.htmlLogfileName);
                        m_handleData->m_textLogFile.setFileName(currentSettings.textLogfileName);
                        m_handleData->m_customLogFile.setFileName(currentSettings.customLogfileName);

                        currentSettings.htmlLogfileName = convertToAbsolutePath(m_mainConfigFile, currentSettings.htmlLogfileName);
                        currentSettings.textLogfileName = convertToAbsolutePath(m_mainConfigFile, currentSettings.textLogfileName);
                        currentSettings.customLogfileName = convertToAbsolutePath(m_mainConfigFile, currentSettings.customLogfileName);
                    }
                }
                {//serial port

                    QDomNodeList nodeList = docElem.elementsByTagName("serialPortSetting");
                    if(!nodeList.isEmpty())
                    {
                        QDomNode node = nodeList.at(0);

                        currentSettings.serialPort.baudRate = node.attributes().namedItem("baudRate").nodeValue().toUInt();
                        currentSettings.serialPort.dataBits = static_cast<QSerialPort::DataBits>(node.attributes().namedItem("dataBits").nodeValue().toUInt());
                        currentSettings.serialPort.stringFlowControl = node.attributes().namedItem("flowControl").nodeValue();
                        currentSettings.serialPort.name = node.attributes().namedItem("name").nodeValue();
                        currentSettings.serialPort.stringParity = node.attributes().namedItem("parity").nodeValue();
                        currentSettings.serialPort.stopBits = static_cast<QSerialPort::StopBits>(node.attributes().namedItem("stopBits").nodeValue().toUInt());
                        currentSettings.serialPort.setDTR = static_cast<QSerialPort::StopBits>(node.attributes().namedItem("setDTR").nodeValue().toUInt() == 1) ? true : false;
                        currentSettings.serialPort.setRTS = static_cast<QSerialPort::StopBits>(node.attributes().namedItem("setRTS").nodeValue().toUInt() == 1) ? true : false;
                    }

                }
                {//socket

                    QDomNodeList nodeList = docElem.elementsByTagName("socketSetting");
                    if(!nodeList.isEmpty())
                    {
                        QDomNode node = nodeList.at(0);

                        currentSettings.connectionType = static_cast<ConnectionType>(node.attributes().namedItem("connectionType").nodeValue().toUInt());
                        currentSettings.socketSettings.destinationPort = node.attributes().namedItem("destinationPort").nodeValue().toUInt();
                        currentSettings.socketSettings.destinationIpAddress = node.attributes().namedItem("destinationIpAddress").nodeValue();
                        currentSettings.socketSettings.ownPort = node.attributes().namedItem("ownPort").nodeValue().toUInt();
                        currentSettings.socketSettings.socketType = node.attributes().namedItem("socketType").nodeValue();

                        currentSettings.socketSettings.proxySettings = node.attributes().namedItem("proxySettings").nodeValue().toUInt();
                        currentSettings.socketSettings.proxyIpAddress = node.attributes().namedItem("proxyIpAddress").nodeValue();
                        currentSettings.socketSettings.proxyPort = node.attributes().namedItem("proxyPort").nodeValue().toUInt();
                        currentSettings.socketSettings.proxyUserName = node.attributes().namedItem("proxyUserName").nodeValue();
                        currentSettings.socketSettings.proxyPassword = node.attributes().namedItem("proxyPassword").nodeValue();
                    }

                }
                {//cheetah spi

                    QDomNodeList nodeList = docElem.elementsByTagName("cheetahSpiSetting");
                    if(!nodeList.isEmpty())
                    {
                        QDomNode node = nodeList.at(0);

                        currentSettings.cheetahSpi.port = node.attributes().namedItem("port").nodeValue().toUInt();
                        currentSettings.cheetahSpi.mode = node.attributes().namedItem("mode").nodeValue().toUInt();
                        currentSettings.cheetahSpi.baudRate = node.attributes().namedItem("baudRate").nodeValue().toUInt();
                        currentSettings.cheetahSpi.chipSelect = node.attributes().namedItem("chipSelect").nodeValue().toUInt();
                    }
                }
                {//pcan

                    QDomNodeList nodeList = docElem.elementsByTagName("pcanSetting");
                    if(!nodeList.isEmpty())
                    {
                        QDomNode node = nodeList.at(0);

                        currentSettings.pcanInterface.baudRate = node.attributes().namedItem("baudRate").nodeValue().toUInt();
                        currentSettings.pcanInterface.busOffAutoReset= (node.attributes().namedItem("busOffAutoReset").nodeValue().toUInt() == 1) ? true : false;
                        currentSettings.pcanInterface.powerSupply= (node.attributes().namedItem("powerSupply").nodeValue().toUInt() == 1) ? true : false;
                        currentSettings.pcanInterface.channel = node.attributes().namedItem("channel").nodeValue().toUInt();
                        currentSettings.pcanInterface.filterExtended= (node.attributes().namedItem("filterExtended").nodeValue().toUInt() == 1) ? true : false;
                        currentSettings.pcanInterface.filterFrom = node.attributes().namedItem("filterFrom").nodeValue();
                        currentSettings.pcanInterface.filterTo = node.attributes().namedItem("filterTo").nodeValue();
                    }
                }
                {//send window

                    QDomNodeList nodeList = docElem.elementsByTagName("sendWindow");
                    if(!nodeList.isEmpty())
                    {
                        QDomNode node = nodeList.at(0);

                        if(m_isFirstProgramStart)
                        {
                            m_sendWindow->setCurrentSequenceFileName (getAndCreateProgramUserFolder() + "/initalSequences.seq");
                        }
                        else
                        {
                            QString fileName = node.attributes().namedItem("sequenceFileName").nodeValue();
                            fileName = convertToAbsolutePath(m_mainConfigFile, fileName);
                            m_sendWindow->setCurrentSequenceFileName(fileName);
                        }

                        m_sendWindow->setCurrentSendString(node.attributes().namedItem("sendString").nodeValue());
                        m_sendWindow->setCurrentCyclicScript(node.attributes().namedItem("cyclicScript").nodeValue());
                        m_sendWindow->setCurrentSendStringFormat(node.attributes().namedItem("sendStringFormat").nodeValue());
                        m_sendWindow->setCurrentSendRepetition(node.attributes().namedItem("sendStringRepetition").nodeValue());
                        m_sendWindow->setCurrentSendPause(node.attributes().namedItem("sendStringPause").nodeValue());
                        isConnected = (node.attributes().namedItem("isConnected").nodeValue() == "1") ? true : false;
                        m_userInterface->interactiveConsoleCheckBox->setChecked((node.attributes().namedItem("interactiveConsoleCheckBox").nodeValue() == "1") ? true : false);
                        currentSettings.targetEndianess = (Endianess)node.attributes().namedItem("targetEndianess").nodeValue().toUInt();
                        m_sendWindow->setAddToHistoryCheckBox((node.attributes().namedItem("addToHistoryCheckBox").nodeValue() == "1") ? true : false);

                        //Read the 2 splitter sizes.
                        for(quint32 i = 0; i < 2; i++)
                        {
                            QString splitterSizes = (i == 0) ? node.attributes().namedItem("windowSplitter").nodeValue() :
                                                               node.attributes().namedItem("cyclicAreaSplitter").nodeValue();;
                            if(splitterSizes.size() > 0)
                            {
                                QStringList list = splitterSizes.split(":");
                                if(list.size()== 2)
                                {
                                    QList<int> readSizes;
                                    readSizes.append(list[0].toInt());
                                    readSizes.append(list[1].toInt());
                                    if(i == 0)
                                    {
                                        m_sendWindow->getWindowSplitter()->setSizes(readSizes);
                                    }
                                    else
                                    {
                                        m_sendWindow->getCyclicAreaSplitter()->setSizes(readSizes);
                                    }
                                }

                            }
                        }



                    }
                }
                {//main window position and size

                    QDomNodeList nodeList = docElem.elementsByTagName("mainWindowPositionAndSize");
                    if(!nodeList.isEmpty())
                    {
                        QDomNode node = nodeList.at(0);

                        QRect rect;
                        rect.setLeft(node.attributes().namedItem("left").nodeValue().toInt());
                        rect.setTop(node.attributes().namedItem("top").nodeValue().toInt());
                        rect.setWidth(node.attributes().namedItem("width").nodeValue().toInt());
                        rect.setHeight(node.attributes().namedItem("height").nodeValue().toInt());
                        m_ignoreNextResizeEventTime = QDateTime::currentDateTime();
                        setWindowPositionAndSize(this, rect);


                        QString splitterSizes = node.attributes().namedItem("splitterSizes").nodeValue();
                        if(splitterSizes.size() > 0)
                        {
                            QStringList list = splitterSizes.split(":");
                            if(list.size()== 2)
                            {
                                QList<int> readSizes;
                                readSizes.append(list[0].toInt());
                                readSizes.append(list[1].toInt());
                                m_userInterface->ToolboxSplitter->blockSignals(true);
                                m_userInterface->ToolboxSplitter->setSizes(readSizes);
                                m_userInterface->ToolboxSplitter->blockSignals(false);
                                m_toolBoxSplitterSizeSecond = list[1].toInt();
                            }
                        }

                        QString sizes = node.attributes().namedItem("toolBoxSplitterSizes").nodeValue();
                        if(!sizes.isEmpty() && sizes.split(":").size() == 4)
                        {
                            m_toolBoxSplitterSizesSecond[0] = sizes.split(":")[0].toInt();
                            m_toolBoxSplitterSizesSecond[1] = sizes.split(":")[1].toInt();
                            m_toolBoxSplitterSizesSecond[2] = sizes.split(":")[2].toInt();
                            m_toolBoxSplitterSizesSecond[3] = sizes.split(":")[3].toInt();
                        }
                        else
                        {
                            m_toolBoxSplitterSizesSecond[0] = m_toolBoxSplitterSizeSecond;
                            m_toolBoxSplitterSizesSecond[1] = m_toolBoxSplitterSizeSecond;
                            m_toolBoxSplitterSizesSecond[2] = m_toolBoxSplitterSizeSecond;
                            m_toolBoxSplitterSizesSecond[3] = m_toolBoxSplitterSizeSecond;
                        }


                        m_currentToolBoxIndex = node.attributes().namedItem("toolBoxIndex").nodeValue().toInt();

                        m_userInterface->toolBox->blockSignals(true);
                        m_userInterface->toolBox->setCurrentIndex(m_currentToolBoxIndex);
                        m_userInterface->toolBox->blockSignals(false);

                        m_userInterface->SendTextEdit->blockSignals(true);
                        m_userInterface->SendFormatComboBox->blockSignals(true);
                        m_userInterface->SendTextEdit->setPlainText(node.attributes().namedItem("sendTextEdit").nodeValue());
                        m_userInterface->SendFormatComboBox->setCurrentText(node.attributes().namedItem("sendFormatComboBox").nodeValue());
                        m_oldSendFormat = m_userInterface->SendFormatComboBox->currentText();
                        m_userInterface->SendTextEdit->blockSignals(false);
                        m_userInterface->SendFormatComboBox->blockSignals(false);

                        QString script = node.attributes().namedItem("scriptTextEdit").nodeValue();
                        if(script.startsWith("./")){script.replace("./", getScriptCommunicatorFilesFolder() + "/");}
                        m_userInterface->ScriptTextEdit->setPlainText(script);

                        splitterSizes = node.attributes().namedItem("sendAreaSplitterSizes").nodeValue();
                        if(splitterSizes.size() > 0)
                        {
                            QStringList list = splitterSizes.split(":");
                            if(list.size()== 2)
                            {
                                QList<int> readSizes;
                                readSizes.append(list[0].toInt());
                                readSizes.append(list[1].toInt());
                                m_userInterface->SendAreaSplitter->setSizes(readSizes);
                                m_sendAreaSplitterSizeSecond = list[1].toInt();
                            }

                        }

                        QString sendAreaInputsSplitterSizes = node.attributes().namedItem("sendAreaInputsSplitterSizes").nodeValue();
                        if(sendAreaInputsSplitterSizes.size() > 0)
                        {
                            QStringList list = sendAreaInputsSplitterSizes.split(":");
                            if(list.size()== 2)
                            {
                                QList<int> readSizes;
                                readSizes.append(list[0].toInt());
                                readSizes.append(list[1].toInt());
                                m_userInterface->SendAreaInputsSplitter->setSizes(readSizes);
                                m_sendAreaInputsSplitterSizeSecond = list[1].toInt();
                            }
                        }
                    }
                }
                {//send window position and size

                    QDomNodeList nodeList = docElem.elementsByTagName("sendWindowPositionAndSize");
                    if(!nodeList.isEmpty())
                    {
                        QDomNode node = nodeList.at(0);
                        QRect rect;
                        rect.setLeft(node.attributes().namedItem("left").nodeValue().toInt());
                        rect.setTop(node.attributes().namedItem("top").nodeValue().toInt());
                        rect.setWidth(node.attributes().namedItem("width").nodeValue().toInt());
                        rect.setHeight(node.attributes().namedItem("height").nodeValue().toInt());

                        setWindowPositionAndSize(m_sendWindow, rect);
                        m_sendWindowPositionAndSizeloaded = true;

                        if(node.attributes().namedItem("visible").nodeValue() == "1")
                        {
                            showSendWindow = true;
                        }
                    }
                }
                {//settings dialog position and size

                    QDomNodeList nodeList = docElem.elementsByTagName("settingsDialogPositionAndSize");
                    if(!nodeList.isEmpty())
                    {
                        QDomNode node = nodeList.at(0);
                        QRect rect;
                        rect.setLeft(node.attributes().namedItem("left").nodeValue().toInt());
                        rect.setTop(node.attributes().namedItem("top").nodeValue().toInt());
                        rect.setWidth(node.attributes().namedItem("width").nodeValue().toInt());
                        rect.setHeight(node.attributes().namedItem("height").nodeValue().toInt());
                        currentSettings.settingsDialogTabIndex = node.attributes().namedItem("settingsDialogTabIndex").nodeValue().toUInt();

                        setWindowPositionAndSize(m_settingsDialog, rect);

                        if(node.attributes().namedItem("visible").nodeValue() == "1")
                        {
                            showSettingWindow = true;
                        }

                    }
                }
                {//script window position and size

                    QDomNodeList nodeList = docElem.elementsByTagName("scriptWindowPositionAndSize");
                    if(!nodeList.isEmpty())
                    {
                        QDomNode node = nodeList.at(0);
                        QRect rect;
                        rect.setLeft(node.attributes().namedItem("left").nodeValue().toInt());
                        rect.setTop(node.attributes().namedItem("top").nodeValue().toInt());
                        rect.setWidth(node.attributes().namedItem("width").nodeValue().toInt());
                        rect.setHeight(node.attributes().namedItem("height").nodeValue().toInt());

                        setWindowPositionAndSize(m_scriptWindow, rect);
                        m_scriptWindowPositionAndSizeloaded = true;


                        if(node.attributes().namedItem("visible").nodeValue() == "1")
                        {
                            scriptWindowIsVisible = true;
                        }
                    }
                }
                {//script window

                    QDomNodeList nodeList = docElem.elementsByTagName("scriptWindow");
                    if(!nodeList.isEmpty())
                    {
                        QDomNode node = nodeList.at(0);

                        if(m_isFirstProgramStart)
                        {
                            m_scriptWindow->setCurrentScriptConfigFileName(getAndCreateProgramUserFolder() + "/initalScripts.scripts");
                        }
                        else
                        {
                            QString fileName = node.attributes().namedItem("scriptConfigFileName").nodeValue();
                            fileName = convertToAbsolutePath(m_mainConfigFile, fileName);
                            m_scriptWindow->setCurrentScriptConfigFileName(fileName);
                        }

                        currentSettings.scriptEditorPath = node.attributes().namedItem("scriptEditorPath").nodeValue();
                        currentSettings.useExternalScriptEditor = (node.attributes().namedItem("useExternalScriptEditor").nodeValue() == "1") ? true : false;


                        QList<int> sizes;
                        sizes.push_back(node.attributes().namedItem("splitterSize1").nodeValue().toInt());
                        sizes.push_back(node.attributes().namedItem("splitterSize2").nodeValue().toInt());

                        if(sizes[0] != 0 && sizes[1] != 0)
                        {
                            m_scriptWindow->setSplitterSizes(sizes);
                        }

                    }
                }
                {//search console

                    QDomNodeList nodeList = docElem.elementsByTagName("searchConsole");
                    if(!nodeList.isEmpty())
                    {
                        QDomNode node = nodeList.at(0);
                        m_userInterface->directionDownRadioButton->setChecked(node.attributes().namedItem("directionDownRadioButton").nodeValue().toUInt());
                        m_userInterface->matchCaseCheckBox->setChecked(node.attributes().namedItem("matchCaseCheckBox").nodeValue().toUInt());
                        m_userInterface->matchWholeWordCheckBox->setChecked(node.attributes().namedItem("matchWholeWordCheckBox").nodeValue().toUInt());
                        m_searchConsole->setLastSearchStrings(node.attributes().namedItem("lastSearchStrings").nodeValue());
                    }
                }
                {//send history

                    QDomNodeList nodeList = docElem.elementsByTagName("historyItem");
                    for (int x = nodeList.size() - 1; x >= 0; x--)
                    {
                        QDomNode node = nodeList.at(x);
                        QString value = node.attributes().namedItem("data").nodeValue();
                        QByteArray array =  SendWindow::textToByteArray("hex", value, DECIMAL_TYPE_UINT8, LITTLE_ENDIAN_TARGET);
                        m_handleData->addDataToSendHistory(&array);
                        m_handleData->m_historyConsoleTimer.stop();
                    }

                    nodeList = docElem.elementsByTagName("sendHistory");
                    if(!nodeList.isEmpty())
                    {
                        QDomNode node = nodeList.at(0);
                        m_userInterface->startIndexSpinBox->blockSignals(true);
                        m_userInterface->historyFormatComboBox->blockSignals(true);
                        m_userInterface->endIndexSpinBox->blockSignals(true);

                        m_userInterface->startIndexSpinBox->setValue(node.attributes().namedItem("startIndexSpinBox").nodeValue().toUInt());
                        m_userInterface->endIndexSpinBox->setValue(node.attributes().namedItem("endIndexSpinBox").nodeValue().toUInt());
                        m_userInterface->sendPauseSpinBox->setValue(node.attributes().namedItem("sendPauseSpinBox").nodeValue().toUInt());
                        m_userInterface->sendRepetitionCountSpinBox->setValue(node.attributes().namedItem("sendRepetitionCountSpinBox").nodeValue().toUInt());
                        m_userInterface->historyFormatComboBox->setCurrentText(node.attributes().namedItem("historyFormatComboBox").nodeValue());

                        m_userInterface->startIndexSpinBox->blockSignals(false);
                        m_userInterface->historyFormatComboBox->blockSignals(false);
                        m_userInterface->endIndexSpinBox->blockSignals(false);

                        if(m_handleData->m_sendHistory.size() != 0)
                        {
                            m_handleData->m_historyConsoleTimer.start();
                        }
                    }
                }
                {//create sce file window settings

                    QDomNodeList nodeList = docElem.elementsByTagName("createSceWindow");
                    if(!nodeList.isEmpty())
                    {
                        QDomNode node = nodeList.at(0);

                        Ui::CreateSceFile* windowUi = m_scriptWindow->getCreateSceFileDialog()->getUI();
                        QRect rect;
                        rect.setLeft(node.attributes().namedItem("left").nodeValue().toInt());
                        rect.setTop(node.attributes().namedItem("top").nodeValue().toInt());
                        rect.setWidth(node.attributes().namedItem("width").nodeValue().toInt());
                        rect.setHeight(node.attributes().namedItem("height").nodeValue().toInt());
                        setWindowPositionAndSize(m_scriptWindow->getCreateSceFileDialog(), rect);
                        m_scriptWindow->getCreateSceFileDialog()->setConfigFileName(node.attributes().namedItem("configFileName").nodeValue());

                        if(node.attributes().namedItem("visible").nodeValue() == "1")
                        {
                            createSceWindowIsVisible = true;
                        }



                        QString splitterSizes = node.attributes().namedItem("splitterSizes").nodeValue();
                        if(splitterSizes.size() > 0)
                        {
                            QStringList list = splitterSizes.split(":");
                            if(list.size()== 2)
                            {
                                QList<int> readSizes;
                                readSizes.append(list[0].toInt());
                                readSizes.append(list[1].toInt());
                                windowUi->splitter->setSizes(readSizes);
                            }
                        }
                    }
                }

                {//update setting

                    QDomNodeList nodeList = docElem.elementsByTagName("updateSetting");
                    if(!nodeList.isEmpty())
                    {
                        QDomNode node = nodeList.at(0);

                        currentSettings.updateSettings.proxySettings = node.attributes().namedItem("proxySettings").nodeValue().toUInt();
                        currentSettings.updateSettings.proxyIpAddress = node.attributes().namedItem("proxyIpAddress").nodeValue();
                        currentSettings.updateSettings.proxyPort = node.attributes().namedItem("proxyPort").nodeValue().toUInt();
                        currentSettings.updateSettings.proxyUserName = node.attributes().namedItem("proxyUserName").nodeValue();
                        currentSettings.updateSettings.proxyPassword = node.attributes().namedItem("proxyPassword").nodeValue();
                    }
                    else
                    {
                        currentSettings.updateSettings.proxySettings = 1;//use system settings.
                    }

                }

                m_settingsDialog->blockSignals(true);
                m_settingsDialog->setAllSettingsSlot(currentSettings, true);
                m_settingsDialog->blockSignals(false);

                m_userInterface->rtsCheckBox->blockSignals(true);
                m_userInterface->rtsCheckBox->setChecked(currentSettings.serialPort.setRTS);
                m_userInterface->rtsCheckBox->blockSignals(false);

                m_userInterface->dtrCheckBox->blockSignals(true);
                m_userInterface->dtrCheckBox->setChecked(currentSettings.serialPort.setDTR);
                m_userInterface->dtrCheckBox->blockSignals(false);

                m_scriptWindow->loadTableData();
                m_sendWindow->loadTableData();
                m_scriptWindow->getCreateSceFileDialog()->loadConfigFile();

                customLogSettingsChangedSlot();
                customConsoleSettingsChangedSlot();
            }
        }

        setUpSequencesPageSlot();
        setUpScriptPageSlot();

        if(scriptWindowIsVisible)
        {
            m_scriptWindow->show();
        }

        if(showSendWindow)
        {
            m_sendWindow->show();
        }
        if(showSettingWindow)
        {
            m_settingsDialog->show();
        }
        if(createSceWindowIsVisible)
        {
            m_scriptWindow->getCreateSceFileDialog()->show();
        }
        if(isConnected)
        {
            toggleConnectionSlot(isConnected);
        }

        textLogActivatedSlot(currentSettings.textLogFile);
        htmLogActivatedSlot(currentSettings.htmlLogFile);
        customLogActivatedSlot(currentSettings.logGenerateCustomLog);
        m_userInterface->actionReopenAllLogs->setVisible(currentSettings.appendTimestampAtLogFileName);

    }

    return result;
}

/**
 * This slot is called if the connection type has been changed.
 */
void MainWindow::conectionTypeChangesSlot()
{

    const Settings* settings = m_settingsDialog->settings();
    if(settings->connectionType != CONNECTION_TYPE_SERIAL_PORT)
    {
        m_userInterface->additionalInfolabel->setText("");
        m_userInterface->rtsCheckBox->setVisible(false);
        m_userInterface->dtrCheckBox->setVisible(false);
    }
    else
    {
        m_userInterface->additionalInfolabel->setText("CTS=0, DSR=0, DCD=0, RI=0");
        m_userInterface->rtsCheckBox->setVisible(true);
        m_userInterface->dtrCheckBox->setVisible(true);
    }
    update();

}

/**
 * Sets the background color of a widget.
 * @param colorString
 *      The text color.
 * @param widget
 *      The widget.
 */
void MainWindow::setWidgetBackgroundColorFromString(QString colorString, QWidget* widget)
{
    bool isOk;

    int r = colorString.mid(0, 2).toUInt(&isOk, 16);
    int g = colorString.mid(2, 2).toUInt(&isOk, 16);
    int b = colorString.mid(4, 2).toUInt(&isOk, 16);

    QColor color(r,g,b);

    QPalette palette = widget->palette();
    palette.setColor(QPalette::Base, color);
    widget->setPalette(palette);

}

/**
 * Sets the font of a console.
 * @param fontFamily
 *      The font family.
 * @param fontSize
 *      The font size.
 * @param textEdit
 *      The console.
 */
void MainWindow::setConsoleFont(QString fontFamily, QString fontSize, QTextEdit* textEdit)
{
    QFont font = textEdit->font();
    font.setFamily(fontFamily);
    bool success;
    qint32 size = fontSize.toInt(&success);
    font.setPointSize(size);
    textEdit->setFont(font);
}

/**
 * Sets the text color of a widget.
 * @param colorString
 *      The text color.
 * @param widget
 *      The widget.
 */
void MainWindow::setWidgetTextColorFromString(QString colorString, QWidget* widget)
{
    bool isOk;

    int r = colorString.mid(0, 2).toUInt(&isOk, 16);
    int g = colorString.mid(2, 2).toUInt(&isOk, 16);
    int b = colorString.mid(4, 2).toUInt(&isOk, 16);

    QColor color(r,g,b);

    QPalette palette = widget->palette();
    palette.setColor(QPalette::Text, color);
    widget->setPalette(palette);

}
/**
 * Initializes the tab in the main window.
 */
void MainWindow::inititializeTab(void)
{
    const Settings* currentSettings = m_settingsDialog->settings();
    static bool showAsciiInConsole = false;
    static bool showHexInConsole = false;
    static bool showDecimalInConsole = false;
    static bool showMixedConsole = false;
    static bool showBinaryConsole = false;
    static bool showCustomConsole = false;
    static bool showCanTab = false;
    static QString receiveColor = "";
    static QString sendColor = "";
    static QString backgroundColor = "";
    static QString messageAndTimestampColor = "";
    static QString mixedAsciiColor = "";
    static QString mixedDecimalColor = "";
    static QString mixedHexadecimalColor = "";
    static QString mixedBinaryColor = "";
    static QString consoleFont = "";
    static QString stringConsoleFontSize = "";
    static DecimalType  consoleDecimalsType = DECIMAL_TYPE_UINT8;
    static Endianess  targetEndianess = LITTLE_ENDIAN_TARGET;

    bool tabsChanged = false;

    if(targetEndianess != currentSettings->targetEndianess)
    {
        tabsChanged = true;
        targetEndianess = currentSettings->targetEndianess;
    }

    if(consoleDecimalsType != currentSettings->consoleDecimalsType)
    {
        tabsChanged = true;
        consoleDecimalsType = currentSettings->consoleDecimalsType;
    }

    if(consoleFont != currentSettings->stringConsoleFont)
    {
        tabsChanged = true;
        consoleFont = currentSettings->stringConsoleFont;
    }

    if(stringConsoleFontSize != currentSettings->stringConsoleFontSize)
    {
        tabsChanged = true;
        stringConsoleFontSize = currentSettings->stringConsoleFontSize;
    }


    if(receiveColor != currentSettings->consoleReceiveColor)
    {
        tabsChanged = true;
        receiveColor = currentSettings->consoleReceiveColor;
    }

    if(sendColor != currentSettings->consoleSendColor)
    {
        tabsChanged = true;
        sendColor = currentSettings->consoleSendColor;
    }

    if(backgroundColor != currentSettings->consoleBackgroundColor)
    {
        tabsChanged = true;
        backgroundColor = currentSettings->consoleBackgroundColor;
    }

    if(messageAndTimestampColor != currentSettings->consoleMessageAndTimestampColor)
    {
        tabsChanged = true;
        messageAndTimestampColor = currentSettings->consoleMessageAndTimestampColor;
    }

    if(mixedAsciiColor != currentSettings->consoleMixedAsciiColor)
    {
        tabsChanged = true;
        mixedAsciiColor = currentSettings->consoleMixedAsciiColor;
    }

    if(mixedDecimalColor != currentSettings->consoleMixedDecimalColor)
    {
        tabsChanged = true;
        mixedDecimalColor = currentSettings->consoleMixedDecimalColor;
    }

    if(mixedHexadecimalColor != currentSettings->consoleMixedHexadecimalColor)
    {
        tabsChanged = true;
        mixedHexadecimalColor = currentSettings->consoleMixedHexadecimalColor;
    }

    if(mixedBinaryColor != currentSettings->consoleMixedBinaryColor)
    {
        tabsChanged = true;
        mixedBinaryColor = currentSettings->consoleMixedBinaryColor;
    }

    if(showAsciiInConsole != currentSettings->showAsciiInConsole)
    {
        tabsChanged = true;
        showAsciiInConsole = currentSettings->showAsciiInConsole;
    }
    if(showHexInConsole != currentSettings->showHexInConsole)
    {
        tabsChanged = true;
        showHexInConsole = currentSettings->showHexInConsole;
    }
    if(showDecimalInConsole!= currentSettings->showDecimalInConsole)
    {
        tabsChanged = true;
        showDecimalInConsole = currentSettings->showDecimalInConsole;
    }
    if(showMixedConsole != currentSettings->showMixedConsole)
    {
        tabsChanged = true;
        showMixedConsole = currentSettings->showMixedConsole;
    }
    if(showBinaryConsole != currentSettings->showBinaryConsole)
    {
        tabsChanged = true;
        showBinaryConsole = currentSettings->showBinaryConsole;
    }
    if(showCustomConsole != currentSettings->consoleShowCustomConsole)
    {
        tabsChanged = true;
        showCustomConsole = currentSettings->consoleShowCustomConsole;
    }
    if(showCanTab != currentSettings->showCanTab)
    {
        tabsChanged = true;
        showCanTab = currentSettings->showCanTab;
    }

    if(tabsChanged)
    {
        m_settingsDialog->setEnabled(false);
        QCoreApplication::processEvents();

        setWidgetBackgroundColorFromString(currentSettings->consoleBackgroundColor, m_userInterface->historyTextEdit);
        setWidgetTextColorFromString(currentSettings->consoleReceiveColor, m_userInterface->historyTextEdit);
        setConsoleFont(currentSettings->stringConsoleFont, currentSettings->stringConsoleFontSize, m_userInterface->historyTextEdit);


        int index = m_userInterface->tabWidget->currentIndex();
        m_userInterface->tabWidget->blockSignals(true);
        //remove all tabs
        for(int i = m_userInterface->tabWidget->count(); i > 0; i--)
        {
            m_userInterface->tabWidget->removeTab(i - 1);
        }


        if(currentSettings->showMixedConsole)
        {
            m_userInterface->tabWidget->addTab( m_userInterface->tabMixed, "Mixed");
            setWidgetBackgroundColorFromString(currentSettings->consoleBackgroundColor, m_userInterface->ReceiveTextEditMixed);
            setWidgetTextColorFromString(currentSettings->consoleReceiveColor, m_userInterface->ReceiveTextEditMixed);
            setConsoleFont("Courier new", currentSettings->stringConsoleFontSize, m_userInterface->ReceiveTextEditMixed);
        }


        if(currentSettings->showAsciiInConsole)
        {
            m_userInterface->tabWidget->addTab( m_userInterface->tabAscii, "Ascii");
            setWidgetBackgroundColorFromString(currentSettings->consoleBackgroundColor, m_userInterface->ReceiveTextEditAscii);
            setWidgetTextColorFromString(currentSettings->consoleReceiveColor, m_userInterface->ReceiveTextEditAscii);
            setConsoleFont(currentSettings->stringConsoleFont, currentSettings->stringConsoleFontSize, m_userInterface->ReceiveTextEditAscii);
        }


        if(currentSettings->showHexInConsole)
        {
            m_userInterface->tabWidget->addTab( m_userInterface->tabHex, "Hex");
            setWidgetBackgroundColorFromString(currentSettings->consoleBackgroundColor, m_userInterface->ReceiveTextEditHex);
            setWidgetTextColorFromString(currentSettings->consoleReceiveColor, m_userInterface->ReceiveTextEditHex);
            setConsoleFont(currentSettings->stringConsoleFont, currentSettings->stringConsoleFontSize, m_userInterface->ReceiveTextEditHex);
        }

        if(currentSettings->showDecimalInConsole)
        {
            m_userInterface->tabWidget->addTab( m_userInterface->tabDecimal, "Dec");
            setWidgetBackgroundColorFromString(currentSettings->consoleBackgroundColor, m_userInterface->ReceiveTextEditDecimal);
            setWidgetTextColorFromString(currentSettings->consoleReceiveColor, m_userInterface->ReceiveTextEditDecimal);
            setConsoleFont(currentSettings->stringConsoleFont, currentSettings->stringConsoleFontSize, m_userInterface->ReceiveTextEditDecimal);
        }

        if(currentSettings->showBinaryConsole)
        {
            m_userInterface->tabWidget->addTab( m_userInterface->tabBinary, "Binary");
            setWidgetBackgroundColorFromString(currentSettings->consoleBackgroundColor, m_userInterface->ReceiveTextEditBinary);
            setWidgetTextColorFromString(currentSettings->consoleReceiveColor, m_userInterface->ReceiveTextEditBinary);
            setConsoleFont(currentSettings->stringConsoleFont, currentSettings->stringConsoleFontSize, m_userInterface->ReceiveTextEditBinary);
        }

        if(currentSettings->consoleShowCustomConsole)
        {
            m_userInterface->tabWidget->addTab( m_userInterface->tabCustom, "Custom");
            setWidgetBackgroundColorFromString(currentSettings->consoleBackgroundColor, m_userInterface->ReceiveTextEditCustom);
            setWidgetTextColorFromString(currentSettings->consoleReceiveColor, m_userInterface->ReceiveTextEditCustom);
            setConsoleFont(currentSettings->stringConsoleFont, currentSettings->stringConsoleFontSize, m_userInterface->ReceiveTextEditCustom);
        }

        if(currentSettings->showCanTab)
        {
            m_userInterface->tabWidget->addTab(m_userInterface->tabCan, "Can");
        }

        //Add all script tabs.
        QMap<QWidget*, QObject*>::iterator i;
        for (i = m_scriptTabs.begin(); i != m_scriptTabs.end(); ++i)
        {
            m_userInterface->tabWidget->addTab(i.key(), m_scriptTabsTitles[i.key()]);
        }

        if(index < m_userInterface->tabWidget->count())
        {
            m_userInterface->tabWidget->setCurrentIndex(index);
        }

        m_handleData->reInsertDataInConsole();

        m_userInterface->tabWidget->blockSignals(false);

        tabIndexChangedSlot(m_userInterface->tabWidget->currentIndex());


        //Prevent the click events in the settings dialog.
        for(quint32 i = 0; i < 10; i++)
        {
            QThread::msleep(1);
            QCoreApplication::processEvents();
        }

        m_settingsDialog->setEnabled(true);
    }

}

/**
 * This slot is called if the main configuration has to be saved.
 */
void MainWindow::configHasToBeSavedSlot(void)
{
    const Settings* currentSettings = m_settingsDialog->settings();
    m_userInterface->actionReopenAllLogs->setVisible(currentSettings->appendTimestampAtLogFileName);


    inititializeTab();
    saveSettings();
}


/**
 * Returns the ScriptCommunicator plugin folder
 * @return
 *      The folder.
 */
QString MainWindow::getPluginsFolder(void)
{
    QString path;
#ifdef Q_OS_LINUX
    path = QCoreApplication::applicationDirPath() + "/bin/plugins";
#elif defined Q_OS_MAC
    path =  getScriptCommunicatorFilesFolder () + "/plugins";
#else
    path =  QCoreApplication::applicationDirPath() + "/plugins";
#endif

    return path;
}

/**
 * Returns the folder in which the ScriptCommunicator files
 * are locared (templates, example scripts and the manual)
 * @return
 *      The folder.
 */
QString MainWindow::getScriptCommunicatorFilesFolder(void)
{
#ifdef Q_OS_MAC
    return QCoreApplication::applicationDirPath() + "/../../..";
#else
    return QCoreApplication::applicationDirPath();
#endif
}

/**
 * Returns the program folder (in the user documents folder). If the folder doesn't exists it will be created.
 * @return
 *      The folder.
 */
QString MainWindow::getAndCreateProgramUserFolder(void)
{
#ifdef Q_OS_LINUX
    QString folder = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/ScriptCommunicator";
#else
    QString folder = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/ScriptCommunicator";
#endif

    if(!QDir(folder).exists())
    {
        QDir().mkdir(folder);
    }
    return folder;
}

/**
 * Saves the main configuration.
 */
void MainWindow::saveSettings()
{

    if(m_commandLineScripts.isEmpty())
    {
        QFile settingsFile(m_mainConfigFile);
        m_settingsDialog->updateSettings();
        const Settings* currentSettings = m_settingsDialog->settings();

        emit globalSettingsChangedSignal(*currentSettings);

        if (!settingsFile.open(QIODevice::Text | QIODevice::WriteOnly))
        {
            QMessageBox::critical(this, "error", "could not open " + m_mainConfigFile);

        }
        else
        {
            QXmlStreamWriter xmlWriter;
            xmlWriter.setAutoFormatting(true);
            xmlWriter.setAutoFormattingIndent(2);
            /* set device (here file)to streamwriter */
            xmlWriter.setDevice(&settingsFile);
            /* Writes a document start with the XML version number version. */
            xmlWriter.writeStartDocument();

            xmlWriter.writeStartElement("MainConfig");
            xmlWriter.writeAttribute("version", VERSION);

            {//console settings
                std::map<QString, QString> consoleSetting =
                {std::make_pair(QString("maxCharsInConsole"), QString("%1").arg(currentSettings->maxCharsInConsole)),
                 std::make_pair(QString("generateTimeStampsInConsole"), QString("%1").arg(currentSettings->generateTimeStampsInConsole)),
                 std::make_pair(QString("showSendDataInConsole"), QString("%1").arg(currentSettings->showSendDataInConsole)),
                 std::make_pair(QString("showReceivedDataInConsole"), QString("%1").arg(currentSettings->showReceivedDataInConsole)),
                 std::make_pair(QString("lockScrollingInConsole"), QString("%1").arg(currentSettings->lockScrollingInConsole)),
                 std::make_pair(QString("stringConsoleFont"), QString("%1").arg(currentSettings->stringConsoleFont)),
                 std::make_pair(QString("stringConsoleFontSize"), QString("%1").arg(currentSettings->stringConsoleFontSize)),
                 std::make_pair(QString("showDecimalInConsole"), QString("%1").arg(currentSettings->showDecimalInConsole)),
                 std::make_pair(QString("showHexInConsole"), QString("%1").arg(currentSettings->showHexInConsole)),
                 std::make_pair(QString("showAsciiInConsole"), QString("%1").arg(currentSettings->showAsciiInConsole)),
                 std::make_pair(QString("timeStampIntervalConsole"), QString("%1").arg(currentSettings->timeStampIntervalConsole)),
                 std::make_pair(QString("updateIntervalConsole"), QString("%1").arg(currentSettings->updateIntervalConsole)),
                 std::make_pair(QString("showMixedConsole"), QString("%1").arg(currentSettings->showMixedConsole)),
                 std::make_pair(QString("showBinaryConsole"), QString("%1").arg(currentSettings->showBinaryConsole)),
                 std::make_pair(QString("showCanMetaInformationInConsole"), QString("%1").arg(currentSettings->showCanMetaInformationInConsole)),
                 std::make_pair(QString("showCanTab"), QString("%1").arg(currentSettings->showCanTab)),
                 std::make_pair(QString("consoleReceiveColor"), QString("%1").arg(currentSettings->consoleReceiveColor)),
                 std::make_pair(QString("consoleSendColor"), QString("%1").arg(currentSettings->consoleSendColor)),
                 std::make_pair(QString("consoleBackgroundColor"), QString("%1").arg(currentSettings->consoleBackgroundColor)),
                 std::make_pair(QString("consoleMessageAndTimestampColor"), QString("%1").arg(currentSettings->consoleMessageAndTimestampColor)),
                 std::make_pair(QString("consoleMixedAsciiColor"), QString("%1").arg(currentSettings->consoleMixedAsciiColor)),
                 std::make_pair(QString("consoleMixedDecimalColor"), QString("%1").arg(currentSettings->consoleMixedDecimalColor)),
                 std::make_pair(QString("consoleMixedHexadecimalColor"), QString("%1").arg(currentSettings->consoleMixedHexadecimalColor)),
                 std::make_pair(QString("consoleMixedBinaryColor"), QString("%1").arg(currentSettings->consoleMixedBinaryColor)),
                 std::make_pair(QString("consoleNewLineAfterBytes"), QString("%1").arg(currentSettings->consoleNewLineAfterBytes)),
                 std::make_pair(QString("consoleNewLineAfterPause"), QString("%1").arg(currentSettings->consoleNewLineAfterPause)),
                 std::make_pair(QString("consoleNewLineAt"), QString("%1").arg(currentSettings->consoleNewLineAt)),
                 std::make_pair(QString("logNewLineAt"), QString("%1").arg(currentSettings->logNewLineAt)),
                 std::make_pair(QString("consoleSendOnEnter"), currentSettings->consoleSendOnEnter),
                 std::make_pair(QString("consoleTimestampFormat"), currentSettings->consoleTimestampFormat),
                 std::make_pair(QString("consoleShowCustomConsole"), QString("%1").arg(currentSettings->consoleShowCustomConsole)),
                 std::make_pair(QString("consoleScript"), convertToRelativePath(m_mainConfigFile, currentSettings->consoleScript)),
                 std::make_pair(QString("consoleCreateTimestampAt"), QString("%1").arg(currentSettings->consoleCreateTimestampAt)),
                 std::make_pair(QString("consoleTimestampAt"), QString("%1").arg(currentSettings->consoleTimestampAt)),
                 std::make_pair(QString("consoleDecimalsType"), QString("%1").arg(currentSettings->consoleDecimalsType)),
                };

                writeXmlElement(xmlWriter, "consoleSettings", consoleSetting);
            }
            {//log settings
                std::map<QString, QString> consoleSetting =
                {std::make_pair(QString("htmlLogFile"), QString("%1").arg(currentSettings->htmlLogFile)),
                 std::make_pair(QString("htmlLogFileName"), convertToRelativePath(m_mainConfigFile, currentSettings->htmlLogfileName)),
                 std::make_pair(QString("textLogFile"), QString("%1").arg(currentSettings->textLogFile)),
                 std::make_pair(QString("textLogFileName"), convertToRelativePath(m_mainConfigFile, currentSettings->textLogfileName)),
                 std::make_pair(QString("customLogFileName"), convertToRelativePath(m_mainConfigFile, currentSettings->customLogfileName)),
                 std::make_pair(QString("writeSendDataInToLog"), QString("%1").arg(currentSettings->writeSendDataInToLog)),
                 std::make_pair(QString("writeReceivedDataInToLog"), QString("%1").arg(currentSettings->writeReceivedDataInToLog)),
                 std::make_pair(QString("generateTimeStampsInLog"), QString("%1").arg(currentSettings->generateTimeStampsInLog)),
                 std::make_pair(QString("appendTimestampAtLogFileName"), QString("%1").arg(currentSettings->appendTimestampAtLogFileName)),
                 std::make_pair(QString("stringHtmlLogFont"), QString("%1").arg(currentSettings->stringHtmlLogFont)),
                 std::make_pair(QString("stringHtmlLogFontSize"), QString("%1").arg(currentSettings->stringHtmlLogFontSize)),
                 std::make_pair(QString("writeDecimalInToLog"), QString("%1").arg(currentSettings->writeDecimalInToLog)),
                 std::make_pair(QString("writeHexInToLog"), QString("%1").arg(currentSettings->writeHexInToLog)),
                 std::make_pair(QString("writeAsciiInToLog"), QString("%1").arg(currentSettings->writeAsciiInToLog)),
                 std::make_pair(QString("writeBinaryInToLog"), QString("%1").arg(currentSettings->writeBinaryInToLog)),
                 std::make_pair(QString("timeStampIntervalLog"), QString("%1").arg(currentSettings->timeStampIntervalLog)),
                 std::make_pair(QString("writeCanMetaInformationInToLog"), QString("%1").arg(currentSettings->writeCanMetaInformationInToLog)),
                 std::make_pair(QString("logNewLineAfterBytes"), QString("%1").arg(currentSettings->logNewLineAfterBytes)),
                 std::make_pair(QString("logNewLineAfterPause"), QString("%1").arg(currentSettings->logNewLineAfterPause)),
                 std::make_pair(QString("logTimestampFormat"), currentSettings->logTimestampFormat),
                 std::make_pair(QString("logGenerateCustomLog"), QString("%1").arg(currentSettings->logGenerateCustomLog)),
                 std::make_pair(QString("logScript"), convertToRelativePath(m_mainConfigFile, currentSettings->logScript)),
                 std::make_pair(QString("logCreateTimestampAt"), QString("%1").arg(currentSettings->logCreateTimestampAt)),
                 std::make_pair(QString("logTimestampAt"), QString("%1").arg(currentSettings->logTimestampAt)),
                 std::make_pair(QString("logDecimalsType"), QString("%1").arg(currentSettings->logDecimalsType)),
                };

                writeXmlElement(xmlWriter, "logSettings", consoleSetting);
            }
            {//serial port
                std::map<QString, QString> consoleSetting =
                {std::make_pair(QString("baudRate"), QString("%1").arg(currentSettings->serialPort.baudRate)),
                 std::make_pair(QString("dataBits"), QString("%1").arg(currentSettings->serialPort.dataBits)),
                 std::make_pair(QString("flowControl"), currentSettings->serialPort.stringFlowControl),
                 std::make_pair(QString("name"), currentSettings->serialPort.name),
                 std::make_pair(QString("parity"), currentSettings->serialPort.stringParity),
                 std::make_pair(QString("stopBits"), QString("%1").arg(currentSettings->serialPort.stopBits)),
                 std::make_pair(QString("setDTR"), QString("%1").arg(currentSettings->serialPort.setDTR)),
                 std::make_pair(QString("setRTS"), QString("%1").arg(currentSettings->serialPort.setRTS))

                };

                writeXmlElement(xmlWriter, "serialPortSetting", consoleSetting);
            }
            {//socket
                std::map<QString, QString> consoleSetting =
                {std::make_pair(QString("connectionType"), QString("%1").arg(currentSettings->connectionType)),
                 std::make_pair(QString("destinationPort"), QString("%1").arg(currentSettings->socketSettings.destinationPort)),
                 std::make_pair(QString("destinationIpAddress"), currentSettings->socketSettings.destinationIpAddress),
                 std::make_pair(QString("ownPort"), QString("%1").arg(currentSettings->socketSettings.ownPort)),
                 std::make_pair(QString("socketType"), QString("%1").arg(currentSettings->socketSettings.socketType)),

                 std::make_pair(QString("proxySettings"), QString("%1").arg(currentSettings->socketSettings.proxySettings)),
                 std::make_pair(QString("proxyIpAddress"), currentSettings->socketSettings.proxyIpAddress),
                 std::make_pair(QString("proxyPort"), QString("%1").arg(currentSettings->socketSettings.proxyPort)),
                 std::make_pair(QString("proxyUserName"), currentSettings->socketSettings.proxyUserName),
                 std::make_pair(QString("proxyPassword"), currentSettings->socketSettings.proxyPassword),
                };

                writeXmlElement(xmlWriter, "socketSetting", consoleSetting);
            }
            {//cheetah spi
                std::map<QString, QString> consoleSetting =
                {std::make_pair(QString("port"), QString("%1").arg(currentSettings->cheetahSpi.port)),
                 std::make_pair(QString("mode"), QString("%1").arg(currentSettings->cheetahSpi.mode)),
                 std::make_pair(QString("baudRate"), QString("%1").arg(currentSettings->cheetahSpi.baudRate)),
                 std::make_pair(QString("chipSelect"), QString("%1").arg(currentSettings->cheetahSpi.chipSelect)),
                };

                writeXmlElement(xmlWriter, "cheetahSpiSetting", consoleSetting);
            }
            {//cheetah pcan
                std::map<QString, QString> consoleSetting =
                {std::make_pair(QString("baudRate"), QString("%1").arg(currentSettings->pcanInterface.baudRate)),
                 std::make_pair(QString("busOffAutoReset"), QString("%1").arg(currentSettings->pcanInterface.busOffAutoReset)),
                 std::make_pair(QString("channel"), QString("%1").arg(currentSettings->pcanInterface.channel)),
                 std::make_pair(QString("powerSupply"), QString("%1").arg(currentSettings->pcanInterface.powerSupply)),
                 std::make_pair(QString("filterExtended"), QString("%1").arg(currentSettings->pcanInterface.filterExtended)),
                 std::make_pair(QString("filterFrom"), QString("%1").arg(currentSettings->pcanInterface.filterFrom)),
                 std::make_pair(QString("filterTo"), QString("%1").arg(currentSettings->pcanInterface.filterTo)),
                };

                writeXmlElement(xmlWriter, "pcanSetting", consoleSetting);
            }
            {//send window
                QList<int> windowSplitterSizes = m_sendWindow->getWindowSplitter()->sizes();
                QList<int> cyclicAreSizes = m_sendWindow->getCyclicAreaSplitter()->sizes();

                std::map<QString, QString> consoleSetting =
                {std::make_pair(QString("sequenceFileName"), convertToRelativePath(m_mainConfigFile, m_sendWindow->getCurrentSequenceFileName())),
                 std::make_pair(QString("sendString"), m_sendWindow->getCurrentSendString()),
                 std::make_pair(QString("cyclicScript"), m_sendWindow->getCurrentCyclicScript()),
                 std::make_pair(QString("sendStringFormat"), m_sendWindow->getCurrentSendStringFormat()),
                 std::make_pair(QString("sendStringRepetition"), m_sendWindow->getCurrentSendRepetition()),
                 std::make_pair(QString("sendStringPause"), m_sendWindow->getCurrentSendPause()),
                 std::make_pair(QString("isConnected"),QString("%1").arg(m_isConnected)),
                 std::make_pair(QString("interactiveConsoleCheckBox"),QString("%1").arg(m_userInterface->interactiveConsoleCheckBox->isChecked())),
                 std::make_pair(QString("windowSplitter"), QString("%1:%2").arg(windowSplitterSizes[0]).arg(windowSplitterSizes[1])),
                 std::make_pair(QString("cyclicAreaSplitter"), QString("%1:%2").arg(cyclicAreSizes[0]).arg(cyclicAreSizes[1])),
                 std::make_pair(QString("targetEndianess"), QString("%1").arg(currentSettings->targetEndianess)),
                 std::make_pair(QString("addToHistoryCheckBox"),QString("%1").arg(m_sendWindow->getAddToHistoryCheckBox())),

                };

                writeXmlElement(xmlWriter, "sendWindow", consoleSetting);
            }
            {//main window position and size

                QRect rect = windowPositionAndSize(this);
                QList<int> splitterSizes = m_userInterface->ToolboxSplitter->sizes();
                QList<int> sendAreaSplitterSizes = m_userInterface->SendAreaSplitter->sizes();
                QList<int> sendAreaInputsSplitterSizes = m_userInterface->SendAreaInputsSplitter->sizes();

                std::map<QString, QString> consoleSetting =
                {std::make_pair(QString("left"), QString("%1").arg(rect.left())),
                 std::make_pair(QString("top"), QString("%1").arg(rect.top())),
                 std::make_pair(QString("width"), QString("%1").arg(rect.width())),
                 std::make_pair(QString("height"), QString("%1").arg(rect.height())),
                 std::make_pair(QString("splitterSizes"), QString("%1:%2").arg(splitterSizes[0]).arg(splitterSizes[1])),
                 std::make_pair(QString("sendAreaSplitterSizes"), QString("%1:%2").arg(sendAreaSplitterSizes[0]).arg(sendAreaSplitterSizes[1])),
                 std::make_pair(QString("sendAreaInputsSplitterSizes"), QString("%1:%2").arg(sendAreaInputsSplitterSizes[0]).arg(sendAreaInputsSplitterSizes[1])),
                 std::make_pair(QString("toolBoxIndex"), QString("%1").arg(m_currentToolBoxIndex)),
                 std::make_pair(QString("sendTextEdit"), m_userInterface->SendTextEdit->toPlainText()),
                 std::make_pair(QString("scriptTextEdit"), m_userInterface->ScriptTextEdit->toPlainText()),
                 std::make_pair(QString("sendFormatComboBox"), m_userInterface->SendFormatComboBox->currentText()),
                 std::make_pair(QString("toolBoxSplitterSizes"),
                 QString("%1:%2:%3:%4").arg(m_toolBoxSplitterSizesSecond[0]).arg(m_toolBoxSplitterSizesSecond[1]).arg(m_toolBoxSplitterSizesSecond[2]).arg(m_toolBoxSplitterSizesSecond[3]))


                };
                writeXmlElement(xmlWriter, "mainWindowPositionAndSize", consoleSetting);
            }
            {//send window position and size

                QRect rect = windowPositionAndSize(m_sendWindow);
                std::map<QString, QString> consoleSetting =
                {std::make_pair(QString("left"), QString("%1").arg(rect.left())),
                 std::make_pair(QString("top"), QString("%1").arg(rect.top())),
                 std::make_pair(QString("width"), QString("%1").arg(rect.width())),
                 std::make_pair(QString("height"), QString("%1").arg(rect.height())),
                 std::make_pair(QString("visible"), QString("%1").arg(m_sendWindow->isVisible()))
                };
                writeXmlElement(xmlWriter, "sendWindowPositionAndSize", consoleSetting);


            }
            {//settings dialog position and size

                QRect rect = windowPositionAndSize(m_settingsDialog);
                std::map<QString, QString> consoleSetting =
                {std::make_pair(QString("left"), QString("%1").arg(rect.left())),
                 std::make_pair(QString("top"), QString("%1").arg(rect.top())),
                 std::make_pair(QString("width"), QString("%1").arg(rect.width())),
                 std::make_pair(QString("height"), QString("%1").arg(rect.height())),
                 std::make_pair(QString("visible"), QString("%1").arg(m_settingsDialog->isVisible())),
                 std::make_pair(QString("settingsDialogTabIndex"), QString("%1").arg(currentSettings->settingsDialogTabIndex))
                };
                writeXmlElement(xmlWriter, "settingsDialogPositionAndSize", consoleSetting);
            }
            {//script window position and size

                QRect rect = windowPositionAndSize(m_scriptWindow);
                std::map<QString, QString> consoleSetting =
                {std::make_pair(QString("left"), QString("%1").arg(rect.left())),
                 std::make_pair(QString("top"), QString("%1").arg(rect.top())),
                 std::make_pair(QString("width"), QString("%1").arg(rect.width())),
                 std::make_pair(QString("height"), QString("%1").arg(rect.height())),
                 std::make_pair(QString("visible"), QString("%1").arg(m_scriptWindow->isVisible()))
                };
                writeXmlElement(xmlWriter, "scriptWindowPositionAndSize", consoleSetting);
            }
            {//script window
                QList<int> sizes = m_scriptWindow->getSplitterSizes();
                std::map<QString, QString> consoleSetting =
                {std::make_pair(QString("scriptConfigFileName"), convertToRelativePath(m_mainConfigFile, m_scriptWindow->getCurrentScriptConfigFileName())),
                 std::make_pair(QString("scriptEditorPath"), currentSettings->scriptEditorPath),
                 std::make_pair(QString("useExternalScriptEditor"),QString("%1").arg(currentSettings->useExternalScriptEditor)),
                 std::make_pair(QString("splitterSize1"), QString("%1").arg(sizes[0])),
                 std::make_pair(QString("splitterSize2"), QString("%1").arg(sizes[1])),
                };


                writeXmlElement(xmlWriter, "scriptWindow", consoleSetting);
            }
            {//search console

                std::map<QString, QString> consoleSetting =
                {
                    std::make_pair(QString("lastSearchStrings"), m_searchConsole->getLastSearchStrings()),
                    std::make_pair(QString("directionDownRadioButton"), QString("%1").arg(m_userInterface->directionDownRadioButton->isChecked())),
                    std::make_pair(QString("matchCaseCheckBox"), QString("%1").arg(m_userInterface->matchCaseCheckBox->isChecked())),
                    std::make_pair(QString("matchWholeWordCheckBox"), QString("%1").arg(m_userInterface->matchWholeWordCheckBox->isChecked()))
                };

                writeXmlElement(xmlWriter, "searchConsole", consoleSetting);
            }
            {//send history

                std::map<QString, QString> consoleSetting =
                {
                    std::make_pair(QString("startIndexSpinBox"), QString("%1").arg(m_userInterface->startIndexSpinBox->value())),
                    std::make_pair(QString("endIndexSpinBox"), QString("%1").arg(m_userInterface->endIndexSpinBox->value())),
                    std::make_pair(QString("sendPauseSpinBox"), QString("%1").arg(m_userInterface->sendPauseSpinBox->value())),
                    std::make_pair(QString("sendRepetitionCountSpinBox"), QString("%1").arg(m_userInterface->sendRepetitionCountSpinBox->value())),
                    std::make_pair(QString("historyFormatComboBox"), m_userInterface->historyFormatComboBox->currentText())
                };

                writeXmlElement(xmlWriter, "sendHistory", consoleSetting);

                xmlWriter.writeStartElement("historyData");
                for(qint32 i = 0; i < m_handleData->m_sendHistory.size(); i++)
                {
                    xmlWriter.writeStartElement("historyItem");

                    xmlWriter.writeAttribute("index", QString("%1").arg(i));

                    QString text = MainWindow::byteArrayToNumberString(m_handleData->m_sendHistory[i],
                                                                       false , true, false, true, true, DECIMAL_TYPE_UINT8, LITTLE_ENDIAN_TARGET);
                    xmlWriter.writeAttribute("data", text);

                    xmlWriter.writeEndElement();//"historyItem"
                }
                xmlWriter.writeEndElement();//historyData
            }


            {//create sce file window settings.

                Ui::CreateSceFile* windowUi = m_scriptWindow->getCreateSceFileDialog()->getUI();
                QRect rect = windowPositionAndSize(m_scriptWindow->getCreateSceFileDialog());
                QList<int> splitterSizes = windowUi->splitter->sizes();

                std::map<QString, QString> createSceSettings =
                {std::make_pair(QString("left"), QString("%1").arg(rect.left())),
                 std::make_pair(QString("top"), QString("%1").arg(rect.top())),
                 std::make_pair(QString("width"), QString("%1").arg(rect.width())),
                 std::make_pair(QString("height"), QString("%1").arg(rect.height())),
                 std::make_pair(QString("splitterSizes"), QString("%1:%2").arg(splitterSizes[0]).arg(splitterSizes[1])),
                 std::make_pair(QString("configFileName"), m_scriptWindow->getCreateSceFileDialog()->getConfigFileName()),
                 std::make_pair(QString("visible"), QString("%1").arg(m_scriptWindow->getCreateSceFileDialog()->isVisible())),
                };
                writeXmlElement(xmlWriter, "createSceWindow", createSceSettings);
            }

            {//update settings
                std::map<QString, QString> updateSetting =
                {std::make_pair(QString("proxyIpAddress"), QString("%1").arg(currentSettings->updateSettings.proxyIpAddress)),
                 std::make_pair(QString("proxyPassword"), QString("%1").arg(currentSettings->updateSettings.proxyPassword)),
                 std::make_pair(QString("proxyPort"), QString("%1").arg(currentSettings->updateSettings.proxyPort)),
                 std::make_pair(QString("proxySettings"), QString("%1").arg(currentSettings->updateSettings.proxySettings)),
                 std::make_pair(QString("proxyUserName"), QString("%1").arg(currentSettings->updateSettings.proxyUserName)),
                };

                writeXmlElement(xmlWriter, "updateSetting", updateSetting);
            }

            xmlWriter.writeEndElement();//"settings"
            xmlWriter.writeEndDocument();

        }
    }


}


/**
 * This slot function shows a QMessageBox dialog.
 * @param icon
 *      The icon if the message box.
 * @param title
 *      The title of the message box.
 * @param text
 *      The text of the message box.
 * @param buttons
 *      The buttons of the message box.
 * @param parent
 *      The parent of the message box
 */
void MainWindow::showMessageBoxSlot(QMessageBox::Icon icon, QString title, QString text, QMessageBox::StandardButtons buttons, QWidget *parent)
{
    QMessageBox message(icon, title, text, buttons, parent ? parent : this);
    message.exec();
}

/**
 * This slot function shows a yes/no dialog.
 * @param icon
 *      The icon if the message box.
 * @param title
 *      The title of the message box.
 * @param text
 *      The text of the message box.
 * @param parent
 *      The parent of the message box
 * @param yesButtonPressed
 *      True if the yes button has been pressed.
 */
void MainWindow::showYesNoDialogSlot(QMessageBox::Icon icon, QString title, QString text,
                                     QWidget* parent, bool* yesButtonPressed)
{
    QMessageBox message(icon, title, text, QMessageBox::Yes | QMessageBox::No, parent);
    if(message.exec() == QMessageBox::Yes)
    {
        *yesButtonPressed = true;
    }
    else
    {
        *yesButtonPressed = false;
    }
}

/**
 * This slot function is called if the html log file has to be activated.
 * It is connected to the SettingsDialog::htmlLogActivatedSignal signal.
 * @param activated
 *      True for activate.
 */
void MainWindow::htmLogActivatedSlot(bool activated)
{
    Settings currentSettings = *m_settingsDialog->settings();

    if(activated)
    {
        if(currentSettings.htmlLogfileName.isEmpty())
        {
            QString fileName = QFileDialog::getSaveFileName(m_settingsDialog, tr("Save html log file"),
                                                            "",tr("Files (*.html)"));
            if(!fileName.isEmpty())
            {
                currentSettings.htmlLogfileName = fileName;
            }
            else
            {
                currentSettings.htmlLogFile = false;
            }
            m_settingsDialog->setAllSettingsSlot(currentSettings, false);
        }

        if(!currentSettings.htmlLogfileName.isEmpty())
        {
            QString fileName = currentSettings.htmlLogfileName;
            if(currentSettings.appendTimestampAtLogFileName)
            {
                if(currentSettings.appendTimestampAtLogFileName)
                {
                    fileName = createLogFileName(fileName);
                }
            }

            m_handleData->m_htmlLogFile.close();
            m_handleData->m_htmlLogFile.setFileName(fileName);
            if(!m_handleData->m_htmlLogFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append))
            {
                QMessageBox::critical(this, "could not open file", fileName);
                currentSettings.htmlLogFile = false;
                m_settingsDialog->setAllSettingsSlot(currentSettings, false);
            }
            else
            {
                if(m_handleData->m_htmlLogFile.size() == 0)
                {
                    m_handleData->m_HtmlLogFileStream << "<style>body {background-color:" +
                                                         QString(MainWindowHandleData::LOG_BACKGROUND_COLOR)  + ";}</style>";
                }
            }
        }
    }
    else
    {
        m_handleData->m_htmlLogFile.close();

        if(!currentSettings.textLogFile)
        {//The text and the html log are disabled now.

            m_handleData->m_decimalLogByteBuffer.clear();
        }
    }
}

/**
 * Creates a log file name.
 * @param fileName
 *      The original file name.
 * @return
 *      The create file name.
 */
QString MainWindow::createLogFileName(QString fileName)
{
    QString result;

    QString time = QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss").toLocal8Bit();

    QFileInfo fileInfo(fileName);

    result = fileInfo.absolutePath();
    result += "/" + fileInfo.baseName();
    result += "_" + time;
    if(!fileInfo.completeSuffix().isEmpty())
    {
        result += "." + fileInfo.completeSuffix();
    }

    return result;
}

/**
 * This slot function is called if the text log file has to be activated.
 * It is connected to the SettingsDialog::textLogActivatedSignal signal.
 * @param activated
 *      True for activate.
 */
void MainWindow::textLogActivatedSlot(bool activated)
{
    Settings currentSettings = *m_settingsDialog->settings();

    if(activated)
    {
        if(currentSettings.textLogfileName.isEmpty())
        {
            QString fileName = QFileDialog::getSaveFileName(m_settingsDialog, tr("Save text log file"),
                                                            "",tr("Files (*.txt)"));
            if(!fileName.isEmpty())
            {
                currentSettings.textLogfileName = fileName;
            }
            else
            {
                currentSettings.textLogFile = false;
            }

            m_settingsDialog->setAllSettingsSlot(currentSettings, false);
        }

        if(!currentSettings.textLogfileName.isEmpty())
        {
            QString fileName = currentSettings.textLogfileName;
            if(currentSettings.appendTimestampAtLogFileName)
            {
                fileName = createLogFileName(fileName);
            }

            m_handleData->m_textLogFile.close();
            m_handleData->m_textLogFile.setFileName(fileName);
            if(!m_handleData->m_textLogFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append))
            {
                QMessageBox::critical(this, "could not open file", fileName);
                currentSettings.textLogFile = false;
                m_settingsDialog->setAllSettingsSlot(currentSettings, false);
            }
        }
    }
    else
    {
        m_handleData->m_textLogFile.close();

        if(!currentSettings.htmlLogFile)
        {//The text and the html log are disabled now.

            m_handleData->m_decimalLogByteBuffer.clear();
        }
    }
}

/**
 * Is called if the custom console settings have been changed.
 */
void MainWindow::customConsoleSettingsChangedSlot()
{
    Settings settings = *m_settingsDialog->settings();

    m_handleData->m_checkDebugWindowsIsClosed.stop();
    m_handleData->m_customConsoleObject->unloadCustomScript();

    if(settings.consoleShowCustomConsole)
    {

        if(!m_handleData->m_customConsoleObject->loadCustomScript(settings.consoleScript, settings.consoleDebugCustomConsole))
        {
            settings.consoleShowCustomConsole = false;
            settings.consoleDebugCustomConsole= false;
            m_settingsDialog->setAllSettingsSlot(settings, false);
        }


    }

    if(settings.logDebugCustomLog || settings.consoleDebugCustomConsole)
    {
        m_handleData->m_checkDebugWindowsIsClosed.start(200);
    }

}

/**
 * Is called if the custom log settings have been changed.
 */
void MainWindow::customLogSettingsChangedSlot()
{
    Settings settings = *m_settingsDialog->settings();


    m_handleData->m_checkDebugWindowsIsClosed.stop();
    m_handleData->m_customLogObject->unloadCustomScript();

    if(settings.logGenerateCustomLog && !settings.customLogfileName.isEmpty())
    {

        if(!m_handleData->m_customLogObject->loadCustomScript(settings.logScript, settings.logDebugCustomLog))
        {
            settings.logGenerateCustomLog = false;
            settings.logDebugCustomLog = false;
            m_settingsDialog->setAllSettingsSlot(settings, false);
        }

        //Must be here, because in this call the corresponding check boxes are disabled if the script runs in the debugger.
        m_settingsDialog->setAllSettingsSlot(settings, false);
    }

    if(settings.logDebugCustomLog || settings.consoleDebugCustomConsole)
    {
        m_handleData->m_checkDebugWindowsIsClosed.start(200);
    }
}

/**
 * This slot function is called if the custom log file has to be activated.
 * It is connected to the SettingsDialog::textLogActivatedSignal signal.
 * @param activated
 *      True for activate.
 */
void MainWindow::customLogActivatedSlot(bool activated)
{
    Settings currentSettings = *m_settingsDialog->settings();

    if(activated)
    {
        if(currentSettings.customLogfileName.isEmpty())
        {
            QString fileName = QFileDialog::getSaveFileName(m_settingsDialog, tr("Save custom log file"),
                                                            "",tr("Files (*)"));
            if(!fileName.isEmpty())
            {
                currentSettings.customLogfileName = fileName;
            }
            else
            {
                currentSettings.logGenerateCustomLog = false;
            }
            m_settingsDialog->setAllSettingsSlot(currentSettings, false);
        }

        if(!currentSettings.customLogfileName.isEmpty())
        {

            QString fileName = currentSettings.customLogfileName;
            if(currentSettings.appendTimestampAtLogFileName)
            {
                fileName = createLogFileName(fileName);
            }


            m_handleData->m_customLogFile.close();
            m_handleData->m_customLogFile.setFileName(fileName);

            if(!m_handleData->m_customLogFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append))
            {
                QMessageBox::critical(this, "could not open file", fileName);
                currentSettings.logGenerateCustomLog = false;
                m_settingsDialog->setAllSettingsSlot(currentSettings, false);
            }
        }
    }
    else
    {
        m_handleData->m_customLogFile.close();
    }

}

/**
 * The main interface thread can activate/deactivate the connect button with this slot.
 * This slot is connected to the MainInterfaceThread::setConnectionButtonsSignal signal.
 * @param canConnect
 *      True for enable.
 */
void MainWindow::setConnectionButtonsSlot(bool enable)
{
    m_userInterface->actionConnect->setEnabled(enable);
    m_settingsDialog->getUserInterface()->connectButton->setEnabled(enable);
    m_settingsDialog->setInterfaceSettingsCanBeChanged(enable);
}

/**
 * Shows additional Information about the connection in the mainwindow.
 * @param text
 *      The text to show.
 */
void MainWindow::showAdditionalConnectionInformationSlot(QString text)
{
    m_userInterface->additionalInfolabel->setText(text);
    update();
}

/**
 * Is called (by script) when the serial port pins shall be changed (DTR, RTS).
 * @param setRTS
 *      True if the RTS pin shall be set.
 * @param setDTS
 *      True if the DTS pin shall be set.
 */
void MainWindow::setSerialPortPinsSlot(bool setRTS, bool setDTR)
{
    m_userInterface->dtrCheckBox->setChecked(setDTR);
    m_userInterface->rtsCheckBox->setChecked(setRTS);
    serialPortPinsChangedSlot();
}

/**
 * Is called when a serial port check box has been changed (DTR, RTS).
 */
void MainWindow::serialPortPinsChangedSlot(void)
{
    m_settingsDialog->updateSettings();
    Settings settings = *m_settingsDialog->settings();
    settings.serialPort.setDTR = m_userInterface->dtrCheckBox->isChecked();
    settings.serialPort.setRTS = m_userInterface->rtsCheckBox->isChecked();
    m_settingsDialog->setAllSettingsSlot(settings, false);
    saveSettings();

}

/**
 * The main window receive the current connection status with this slot (from the main interface thread).
 * This slot is connected to the MainInterfaceThread::dataConnectionStatusSignal signal.
 * @param isConnected
 *      True for connected.
 * @param message
 *      Additional information about the connection status.
 * @param isWaiting
 *      True if the interface is waiting for a client/connection
 */
void MainWindow::dataConnectionStatusSlot(bool isConnected, QString message, bool isWaiting)
{
    bool showConnect = true;
    m_sendWindow->setIsConnected(isConnected);
    m_statusBarLabel.setText(message);

    if(isConnected)
    {
        const Settings* currentSettings = m_settingsDialog->settings();

        m_isConnected = true;
        m_isConnectedWithCan = (currentSettings->connectionType == CONNECTION_TYPE_PCAN) ? true : false;
        showConnect = false;
        m_userInterface->actionConnect->setText("Disconnect");
        m_settingsDialog->getUserInterface()->connectButton->setText("disconnect");
    }
    else
    {
        m_isConnected = false;
        m_isConnectedWithCan = false;
        showConnect = isWaiting ? false : true;
        m_userInterface->actionConnect->setText(isWaiting ? "Stop waiting" : "Connect");
        m_settingsDialog->getUserInterface()->connectButton->setText(isWaiting ? "stop waiting" : "connect");
    }

    m_userInterface->actionConnect->setIcon(showConnect ? QIcon(":/connect") : QIcon(":/disconnect"));
    m_settingsDialog->setInterfaceSettingsCanBeChanged(showConnect);
}

/**
 * Slot function for the connect/disconnect button.
 */
void MainWindow::toggleConnectionSlot(bool connectionStatus)
{
    m_settingsDialog->updateSettings();
    Settings settings = *m_settingsDialog->settings();

    if (connectionStatus)
    {
        //connect
        settings.serialPort.setDTR = m_userInterface->dtrCheckBox->isChecked();
        settings.serialPort.setRTS = m_userInterface->rtsCheckBox->isChecked();
        configHasToBeSavedSlot();
        emit connectDataConnectionSignal(settings, true);
    }
    else
    {
        //disconnect
        emit connectDataConnectionSignal(settings, false);
    }
}

/**
 * Parses a API file and returns a semicolon separated list with all public functions, signals and properties.
 * @param name
 *      The name of the API file.
 * @return
 *      The list.
 */
QString MainWindow::parseApiFile(QString name)
{
    QString result;
    QFile file(MainWindow::getScriptCommunicatorFilesFolder() + "/apiFiles/" + name);
    if (file.open(QFile::ReadOnly))
    {
        QTextStream in(&file);
        QString singleLine = in.readLine();
        while(!singleLine.isEmpty())
        {
            QString singleEntry;
            QStringList list = singleLine.split(":");

            if(list.length() >= 3)
            {
                if((list.length() > 3) && (list[2].indexOf("\\n") == -1) )
                {
                    singleEntry = list[2];
                    list = list[3].split("\\n");
                    singleEntry = list[0] + " " + singleEntry;
                }
                else
                {//Signals.
                    list = list[2].split("\\n");
                    singleEntry = list[0];
                }

                if(result.isEmpty())
                {//The current entry is the first entry.

                    result += singleEntry;
                }
                else
                {//The current entry is not the first entry.

                    result += ";" + singleEntry;
                }
            }

            singleLine = in.readLine();
        }
        file.close();
    }
    return result;
}

/**
 * Converts a byte array into his string representation (Byte 1 is converted into char '1'...).
 * @param data
 *      The data.
 * @param isBinary
 *      True if the format is binary.
 * @param isHex
 *      True if the format is hexadecimal.
 * @param withFormatBrackets
 *      True, if the numbers shall be inside a bracket (h[...] or d[...])
 * @param withLeadingZero
 *      True if leading zeros shall be added.
 * @param withSpaces
 *      True if spaces between the single numbers should be added.
 * @param decimalType
 *      The decimal type.
 * @param endianess
 *      The endianess of the data in data.
 * @return
 *      The created string.
 */
QString MainWindow::byteArrayToNumberString(const QByteArray &data, bool isBinary, bool isHex, bool withFormatBrackets, bool withLeadingZero,
                                            bool withSpaces, DecimalType decimalType, Endianess endianess)
{
    const char* dataArray = data.constData();
    QString dataString;
    int bytesPerNumber = 1;

    if(isHex)
    {
        if(withFormatBrackets){dataString.append(" h[");}
    }
    else if(isBinary)
    {
        if(withFormatBrackets){dataString.append(" b[");}
    }
    else
    {
        if(withFormatBrackets){dataString.append(" d[");}
    }

    bool isFirstElement = true;
    int prec =0;

    if(isBinary)
    {
        prec = 2;
    }
    else if(isHex)
    {
        prec = 16;
    }
    else
    {
        prec = 10;
        if((decimalType == DECIMAL_TYPE_UINT16) || (decimalType == DECIMAL_TYPE_INT16))
        {
            bytesPerNumber = 2;
        }
        else if((decimalType == DECIMAL_TYPE_UINT32) || (decimalType == DECIMAL_TYPE_INT32))
        {
            bytesPerNumber = 4;
        }
        else
        {
            bytesPerNumber = 1;
        }
    }

    for(int i = 0; (i + bytesPerNumber) <= data.length(); i+=bytesPerNumber)
    {
        if(!isFirstElement)
        {
            if(withSpaces)
            {
                dataString.append(" ");
            }
        }
        else
        {
            isFirstElement = false;
        }

        QString tmp;

        if(isHex)
        {
            tmp = QString::number(static_cast<uint>(static_cast<quint8>(dataArray[i])),prec);

            if(withLeadingZero && (tmp.size() == 1))
            {
                tmp = "0" + tmp;
            }
        }
        else if(!isHex && !isBinary)
        {//decimal

            quint32 number = 0;
            for(int k = 0; k < bytesPerNumber; k++)
            {
                if(endianess == LITTLE_ENDIAN_TARGET)
                {
                    number += (quint32)((quint8)dataArray[i + k]) << (8 * k);
                }
                else
                {
                    number += (quint32)((quint8)dataArray[i + k]) << (8 * (bytesPerNumber - (k + 1)));
                }
            }

            char buffer[32];
            if(decimalType == DECIMAL_TYPE_UINT8)
            {
                if(withLeadingZero)
                {
                    sprintf(buffer, "%03u", (quint8)number);
                }
                else
                {
                    sprintf(buffer, "%u", (quint8)number);
                }
            }
            else if(decimalType == DECIMAL_TYPE_INT8)
            {
                if(withLeadingZero)
                {
                    sprintf(buffer, "%04d", (qint8)number);
                }
                else
                {
                    sprintf(buffer, "%d", (qint8)number);
                }
            }
            else if(decimalType == DECIMAL_TYPE_UINT16)
            {
                if(withLeadingZero)
                {
                    sprintf(buffer, "%05u", (quint16)number);
                }
                else
                {
                    sprintf(buffer, "%u", (quint16)number);
                }
            }
            else if(decimalType == DECIMAL_TYPE_INT16)
            {
                if(withLeadingZero)
                {
                    sprintf(buffer, "%06d", (qint16)number);
                }
                else
                {
                    sprintf(buffer, "%d", (qint16)number);
                }
            }
            else if(decimalType == DECIMAL_TYPE_UINT32)
            {
                if(withLeadingZero)
                {
                    sprintf(buffer, "%010u", (quint32)number);
                }
                else
                {
                    sprintf(buffer, "%u", (quint32)number);
                }
            }
            else
            {//DECIMAL_TYPE_INT32
                if(withLeadingZero)
                {
                    sprintf(buffer, "%011d", (qint32)number);
                }
                else
                {
                    sprintf(buffer, "%d", (qint32)number);
                }
            }
            tmp = buffer;
        }
        else if(isBinary)
        {
            tmp = QString::number(static_cast<uint>(static_cast<quint8>(dataArray[i])),prec);

            if(withLeadingZero)
            {
                QString leadingZeros;
                for(int i = 0; i < (8 - tmp.size()); i++)
                {
                    leadingZeros += "0";
                }
                tmp = leadingZeros + tmp;
            }
        }
        dataString.append(tmp);

    }
    if(withFormatBrackets)
    {
        dataString.append("] ");
    }

    return dataString;
}

/**
 * Starts/executes the script editor.
 * @param scriptEditor
 *      The script editor path.
 * @param arguments
 *  The editor arguments.
 * @param parent
 *      Parent pointer for the message box.
 * @param isInternalEditor
 *      True if the internal script editor shall be started.
 * @return
 *      True on success.
 */
bool MainWindow::startScriptEditor(QString scriptEditor, QStringList arguments, QWidget* parent, bool isInternalEditor)
{

    QProcess *myProcess = new QProcess(parent);
    bool success = true;

#ifdef Q_OS_MAC
    if(isInternalEditor)
    {
        //Start the script editor.
        if(!myProcess->startDetached(scriptEditor, arguments))
        {
            QMessageBox::critical(parent, "error starting script editor", "could not start: " + scriptEditor);
            success = false;
        }
    }
    else
    {
        QStringList args;
        args << "-a" + scriptEditor;
        args << arguments;
        myProcess->start("open", args);

        myProcess->waitForFinished(10000);
        if(myProcess->exitCode() != 0)
        {
            if(scriptEditor.isEmpty())
            {
                QMessageBox::critical(parent, "error starting the external script editor", "the path to the external script editor is empty (settings dialog)");
            }
            else
            {
                QMessageBox::critical(parent, "error starting the external script editor", "could not start: " + scriptEditor);
            }
            success = false;
        }
    }

#else
    (void)isInternalEditor;

    //Start the script editor.
    if(!myProcess->startDetached(scriptEditor, arguments))
    {
        if(scriptEditor.isEmpty())
        {
            QMessageBox::critical(parent, "error starting script editor", "the path to the external script editor is empty (settings dialog)");
        }
        else
        {
            QMessageBox::critical(parent, "error starting script editor", "could not start: " + scriptEditor);
        }
        success = false;
    }
#endif
    return success;

}

/**
 * Opens the external script editor.
 * @param arguments
 *      The editor arguments.
 * @param currentSettings
 *      The current settings.
 * @param parent
 *      The parent window.
 */
void MainWindow::openScriptEditor(QStringList arguments, const Settings* currentSettings, QWidget* parent)
{
    QString internalScriptEditor;
#ifdef Q_OS_LINUX
    internalScriptEditor = QCoreApplication::applicationDirPath() + "/ScriptEditor";
#elif defined Q_OS_MAC
    internalScriptEditor = QCoreApplication::applicationDirPath() + "/ScriptEditor";
#else//Windows
    internalScriptEditor = QCoreApplication::applicationDirPath() + "/ScriptEditor.exe";
#endif

    if(currentSettings->useExternalScriptEditor)
    {//An external editor shall be used.

        if(!startScriptEditor(currentSettings->scriptEditorPath, arguments, parent, false))
        {//The external script editor could not be started.
            startScriptEditor(internalScriptEditor, arguments, parent, true);
        }
    }
    else
    {
        (void)startScriptEditor(internalScriptEditor, arguments, parent, true);
    }
}

/**
 * Limits the number of char in the given text edit to the value of maxChars.
 * @param textEdit
 *      The text edit.
 * @param maxChars
 *      The max. number of chars.
 */
void MainWindow::limtCharsInTextEdit(const QTextEdit* textEdit, const int maxChars)
{

    int currentCount = textEdit->document()->characterCount();

    if(currentCount > (maxChars + (maxChars / 5)))
    {
        QTextCursor tc = textEdit->textCursor();
        tc.setPosition(0);
        tc.setPosition(currentCount - (maxChars + 1), QTextCursor::KeepAnchor);
        tc.removeSelectedText();
        tc.movePosition( QTextCursor::End, QTextCursor::MoveAnchor );
    }
}

/**
 * Sets the position and the size of a window.
 * @param widget
 *      The widget.
 * @param positionAndSize
 *      The position and size of the window.
 */
void MainWindow::setWindowPositionAndSize(QWidget* widget, const QRect& positionAndSize)
{
    widget->resize(positionAndSize.size());
    widget->move(positionAndSize.topLeft());
}
/**
 * Shows the number of received and sent bytes.
 */
void MainWindow::showNumberOfReceivedAndSentBytes(void)
{
    m_userInterface->ReceiveLable->setText(QString("%1 bytes received (%2 b/s)").arg(m_handleData->m_receivedBytes).arg(m_dataRateReceive)
                                           + QString("  %1 bytes sent (%2 b/s)").arg(m_handleData->m_sentBytes).arg(m_dataRateSend));
}


/**
 * Appends a console string to a console and clears the console string.
 * @param consoleString
 *      The console string.
 * @param textEdit
 *      The text edit.
 */
void MainWindow::appendConsoleStringToConsole(QString* consoleString, QTextEdit* textEdit)
{
    const Settings* settings = m_settingsDialog->settings();

    if(consoleString->size() > 0)
    {
        textEdit->blockSignals(true);
        textEdit->document()->blockSignals(true);

        //Store the scroll bar position.
        int val = textEdit->verticalScrollBar()->value();

        textEdit->moveCursor(QTextCursor::End);
        textEdit->insertHtml(*consoleString);
        textEdit->moveCursor(QTextCursor::End);
        consoleString->clear();

        limtCharsInTextEdit(textEdit, settings->maxCharsInConsole);
        if(settings->lockScrollingInConsole)
        {
            //Restore the scroll bar position.
            textEdit->verticalScrollBar()->setValue(val);
        }
        else
        {   //Move the scroll bar to the end.
            textEdit->horizontalScrollBar()->setSliderPosition(0);
        }

        textEdit->blockSignals(false);
        textEdit->document()->blockSignals(false);
    }
}

/**
 * A user message has been entered (in the add message dialog or in a script).
 * @param message
 *      The message.
 * @param forceTimeStamp
 *      True if a time stamp shall be generated (independently from the time stamp settings)
 */
void MainWindow::messageEnteredSlot(QString message, bool forceTimeStamp)
{
    QByteArray array = message.toLocal8Bit();
    m_handleData->appendDataToStoredData(array, true, true, m_isConnectedWithCan, forceTimeStamp);
}

/**
 * Current data rates slot.
 * @param dataRateSend
 *      The current send data rate.
 * @param dataRateReceive
*      The current receive data rate.
 */
void MainWindow::dataRateUpdateSlot(quint32 dataRateSend, quint32 dataRateReceive)
{

    m_dataRateSend = dataRateSend;
    m_dataRateReceive = dataRateReceive;
    showNumberOfReceivedAndSentBytes();
}

/**
 * Slot function for the connect button.
 */
void MainWindow::connectButtonSlot(void)
{
    toggleConnectionSlot((m_userInterface->actionConnect->text() == "Connect") ? true : false);
}

/**
 * Initializes all actions from the main window gui.
 */
void MainWindow::initActionsConnections()
{
    connect(m_userInterface->actionConnect, SIGNAL(triggered()), this, SLOT(connectButtonSlot()));
    connect(m_settingsDialog->getUserInterface()->connectButton, SIGNAL(clicked(bool)), this, SLOT(connectButtonSlot()));

    connect(m_userInterface->actionQuit, SIGNAL(triggered()), this, SLOT(close()));
    connect(m_userInterface->actionConfigure, SIGNAL(triggered()), this, SLOT(showSettingsWindowSlot()));
    connect(m_userInterface->actionClear, SIGNAL(triggered()), this, SLOT(clearConsoleSlot()));
    connect(m_userInterface->actionSending, SIGNAL(triggered()), this, SLOT(showSendingWindowSlot()));
    connect(m_userInterface->actionScripts, SIGNAL(triggered()), this, SLOT(showScriptWindowSlot()));
    connect(m_userInterface->actionAbout, SIGNAL(triggered()), this, SLOT(showAboutWindowSlot()));
    connect(m_userInterface->actionManual, SIGNAL(triggered()), this, SLOT(openTheManualSlot()));
    connect(m_userInterface->actionAddMessage, SIGNAL(triggered()), this, SLOT(openAddMessageDialogSlot()));
    connect(m_userInterface->actionBringAllToForeground, SIGNAL(triggered()), this, SLOT(bringWindowsToFrontSlot()));


    connect(m_userInterface->actionCopyConfig, SIGNAL(triggered()), this, SLOT(copyConfigSlot()));
    connect(m_userInterface->actionLoadConfig, SIGNAL(triggered()), this, SLOT(loadConfigSlot()));
    connect(m_userInterface->actionLoadPreviosConfig, SIGNAL(triggered()), this, SLOT(loadPreviousConfigSlot()));
    connect(m_userInterface->actionDeletePreviousConfigList, SIGNAL(triggered()), this, SLOT(deletePreviousConfigListSlot()));
    connect(m_userInterface->actionSetCurrentConfigToDefault, SIGNAL(triggered()), this, SLOT(setCurrentConfigToDefaultSlot()));
    connect(m_userInterface->actionResetDefaultConfig, SIGNAL(triggered()), this, SLOT(resetDefaultConfigSlot()));
    connect(m_userInterface->actionCreateNewConfig, SIGNAL(triggered()), this, SLOT(createConfigSlot()));

    connect(m_userInterface->actionSaveConsole, SIGNAL(triggered()),this, SLOT(saveConsoleSlot()));
    connect(m_userInterface->actionPrintConsole, SIGNAL(triggered()),this, SLOT(printConsoleSlot()));
    connect(m_userInterface->actionSubmitBug, SIGNAL(triggered()),this, SLOT(submitBugSlot()));
    connect(m_userInterface->actionRequestFeature, SIGNAL(triggered()),this, SLOT(requestFeatureSlot()));
    connect(m_userInterface->actionVideo, SIGNAL(triggered()),this, SLOT(watchVideoSlot()));
    connect(m_userInterface->actionGetSupport, SIGNAL(triggered()),this, SLOT(getSupportSlot()));
    connect(m_userInterface->actionCheckForUpdates, SIGNAL(triggered()),this, SLOT(checkForUpdatesSlot()));
    connect(m_settingsDialog->getUserInterface()->checkForUpdates, SIGNAL(pressed()),this, SLOT(checkForUpdatesSlot()));

    connect(m_userInterface->actionAddScript, SIGNAL(triggered()),this, SLOT(addScriptSlot()));
    connect(m_userInterface->actionEditScript, SIGNAL(triggered()),this, SLOT(editScriptSlot()));
    connect(m_userInterface->actionDebugSequenceScript, SIGNAL(triggered()),this, SLOT(debugSequenceScript()));
    connect(m_userInterface->actionReopenAllLogs, SIGNAL(triggered()),this, SLOT(reopenLogsSlot()));


}

/**
 * Slot function for the clear console button.
 */
void MainWindow::clearConsoleSlot(void)
{
    m_userInterface->ReceiveTextEditAscii->clear();
    m_userInterface->ReceiveTextEditHex->clear();
    m_userInterface->ReceiveTextEditDecimal->clear();
    m_userInterface->ReceiveTextEditMixed->clear();
    m_userInterface->ReceiveTextEditBinary->clear();
    m_userInterface->ReceiveTextEditCustom->clear();
    m_canTab->clearTables();

    m_handleData->clear();
}

/**
 * Slot function for the show settings window button.
 */
void MainWindow::showSettingsWindowSlot(void)
{
    m_settingsDialog->isVisible() ? (void)m_settingsDialog->close() : m_settingsDialog->show();
}

/**
 * Slot function which shows/hide the add message dialog.
 */
void MainWindow::openAddMessageDialogSlot(void)
{
    if(!m_addMessageDialog->isVisible())
    {
        m_addMessageDialog->show();
        m_addMessageDialog->showNormal();
    }
    else
    {
        m_addMessageDialog->hide();
    }
}

/**
 * Slot function which opens the manual.
 */
void MainWindow::openTheManualSlot(void)
{
    QString name = "Manual_ScriptCommunicator.pdf";
#ifdef Q_OS_MAC
    QProcess process;
    QStringList args;
    args << (getScriptCommunicatorFilesFolder() + "/" + name);
    process.start("open", args);
    process.waitForFinished(10000);
    if(process.exitCode() != 0)
    {
        QMessageBox::critical(this, "error", "could not open " + name);
    }
#else
    name = getScriptCommunicatorFilesFolder() + "/" + name;
    if(!QDesktopServices::openUrl(QUrl::fromLocalFile(name)))
    {
        QMessageBox::critical(this, "error", "could not open " + name);
    }
#endif
}

/**
 * Slot function for the show about window button.
 */
void MainWindow::showAboutWindowSlot(void)
{

    QString text = "author: Stefan Zieker";
    text.append("<br>email: <a href=\"s.zieker@gmx.net\">s.zieker@gmx.net</a>");
    text.append("<br>web1: <a href=\"https://sourceforge.net/projects/scriptcommunicator/\">https://sourceforge.net/projects/scriptcommunicator/</a>");
    text.append("<br>web2: <a href=\"https://github.com/szieke/ScriptCommunicator_serial-terminal\">https://github.com/szieke/ScriptCommunicator_serial-terminal</a>");

    //Current version
    text.append("<br>version: " + VERSION);

    //Qt version
    text.append("<br>qt version: ");
    text.append(QT_VERSION_STR);

    QMessageBox msgBox(QMessageBox::Information, "about",text);
    msgBox.setTextFormat(Qt::RichText);
    msgBox.exec();
}

/**
 * Slot function for the show script window button.
 */
void MainWindow::showScriptWindowSlot(void)
{
    if(m_scriptWindow->isVisible())
    {
        m_scriptWindow->close();
    }
    else
    {
        if(!m_scriptWindowPositionAndSizeloaded)
        {
            //width of one frame
            QRect sendWindowRect = m_scriptWindow->geometry();
            QRect mainWindowRect = geometry();

            sendWindowRect.setHeight(mainWindowRect.height());
            m_scriptWindow->setGeometry(sendWindowRect);

            m_scriptWindow->move(x()+ size().width() / 2, y());
        }
        m_scriptWindow->show();
        m_scriptWindow->showNormal();
    }
}

/**
 * Slot function for the show sending window button.
 */
void MainWindow::showSendingWindowSlot(void)
{
    if(m_sendWindow->isVisible())
    {
        m_sendWindow->close();
    }
    else
    {
        //width of one frame
        int frameWidth = (frameGeometry().width() - geometry().width()) / 2;

        if(!m_sendWindowPositionAndSizeloaded)
        {
            QRect sendWindowRect = m_sendWindow->geometry();
            QRect mainWindowRect = geometry();

            sendWindowRect.setHeight(mainWindowRect.height());
            m_sendWindow->setGeometry(sendWindowRect);

            m_sendWindow->move(x()+ size().width() +  frameWidth * 2, y());
        }
        m_sendWindow->show();
        m_sendWindow->showNormal();
    }
}

/**
 * Adds tabs to the main window.
 * @param tabWidget
 *      The tab widget.
 */
void MainWindow::addTabsToMainWindowSlot(QTabWidget* tabWidget)
{
    QObject* scriptThread = sender();

    while(tabWidget->count() > 0)
    {
        QWidget* tab = tabWidget->widget(0);
        m_scriptTabs[tab] = scriptThread;
        m_scriptTabsTitles[tab] = tabWidget->tabText(0);
        tab->setAutoFillBackground(true);

        m_userInterface->tabWidget->addTab(tab, tabWidget->tabText(0));
    }

    m_userInterface->tabWidget->setCurrentIndex(m_userInterface->tabWidget->count() - 1);

}
/**
 * Adds script toolbox pages to the main window.
 * @param toolBox
 *      The tool box.
 */
void  MainWindow::addToolBoxPagesToMainWindowSlot(QToolBox* toolBox)
{
    QObject* scriptThread = sender();

    while(toolBox->count() > 0)
    {
        QWidget* page = toolBox->widget(0);
        m_scriptToolBoxPage[page] = scriptThread;
        QString text = toolBox->itemText(0);
        toolBox->removeItem(0);

        m_toolBoxSplitterSizesSecond.append(m_toolBoxSplitterSizeSecond);
        m_userInterface->toolBox->addItem(page, text);

    }

    if(!isHidden())
    {
        m_userInterface->toolBox->setCurrentIndex(m_userInterface->toolBox->count() - 1);
    }
    else
    {
        m_userInterface->toolBox->blockSignals(true);
        m_userInterface->toolBox->setCurrentIndex(m_currentToolBoxIndex);
        m_userInterface->toolBox->blockSignals(false);
    }
}

/**
 * Removes all script tabs and tool box pages for one script thread.
 * @param scriptThrad
 *      The script thread.
 */
void MainWindow::removeAllTabsAndToolBoxPages(QObject* scriptThread)
{
    //Add all script tabs.
    QMap<QWidget*, QObject*>::iterator i;
    for (i = m_scriptTabs.begin(); i != m_scriptTabs.end();)
    {
        if(i.value() == scriptThread)
        {
            int index = m_userInterface->tabWidget->indexOf(i.key());
            m_userInterface->tabWidget->removeTab(index);

            m_scriptTabsTitles.remove(i.key());

            i = m_scriptTabs.erase(i);
        }
        else
        {
            ++i;
        }
    }

    for (i = m_scriptToolBoxPage.begin(); i != m_scriptToolBoxPage.end();)
    {
        if(i.value() == scriptThread)
        {
            int index = m_userInterface->toolBox->indexOf(i.key());
            m_userInterface->toolBox->removeItem(index);
            m_toolBoxSplitterSizesSecond.removeAt(index);
            i = m_scriptToolBoxPage.erase(i);
        }
        else
        {
            ++i;
        }
    }
}

/**
 * Enables/Disables all script tabs for one script thread.
 * @param scriptThread
 * @param scriptThrad
 *      The script thread.
 * @param enable
 *      True of enabling and false for disabling.
 */
void MainWindow::enableAllTabsForOneScriptThreadSlot(QObject *scriptThread, bool enable)
{
    QMap<QWidget*, QObject*>::iterator i;
    for (i = m_scriptTabs.begin(); i != m_scriptTabs.end(); ++i)
    {
        if(i.value() == scriptThread)
        {
            i.key()->setEnabled(enable);
        }
    }
}

/**
 * Brings all windows to foreground.
 * @param callerWindow
 *      The caller window.
 */
void MainWindow::bringWindowsToFrontSlot(void)
{
    QList<QWidget*> widgets;
    widgets << m_settingsDialog << m_scriptWindow << m_sendWindow << m_addMessageDialog << m_scriptWindow->getCreateSceFileDialog() << this;

    emit bringWindowsToFrontSignal();
    for(auto el : widgets)
    {
        if(el->isVisible())
        {
            el->setWindowState( (el->windowState() & ~Qt::WindowMinimized) | Qt::WindowActive);
            el->raise();  // for MacOS
            el->activateWindow(); // for Windows
        }
    }
}


/**
 * This function is called if the main window is closed.
 * @param event
 *      The close event.
 */
void MainWindow::closeEvent(QCloseEvent * event)
{
    emit connectDataConnectionSignal(*m_settingsDialog->settings(), false);

    m_sendWindow->programIsClosing();

    saveSettings();

    m_scriptWindow->stopAllScripts();

    while(!m_scriptWindow->allScriptsHaveBeenStopped())
    {
        QThread::msleep(10);
        QCoreApplication::processEvents();
    }

    if(m_addMessageDialog->isVisible())
    {
        m_addMessageDialog->close();
    }

    m_scriptWindow->closeCreateSceFileDialog();
    if(m_commandLineScripts.isEmpty() && m_scriptWindow->getCreateSceFileDialog()->configMustBeSaved())
    {
        m_scriptWindow->getCreateSceFileDialog()->saveConfigSlot();
    }

    if(m_scriptWindow->isVisible())
    {
        m_scriptWindow->close();
    }
    if(m_settingsDialog->isVisible())
    {
        m_settingsDialog->close();
    }
    if(m_sendWindow->isVisible())
    {
        m_sendWindow->close();
    }

    emit exitThreadSignal();

    QThread::msleep(200);

    while(m_mainInterface->isRunning())
    {
        QThread::msleep(10);
        QCoreApplication::processEvents();
    }

    event->accept();
}

/**
 * This slot function is called if the log file has to be deleted.
 * It is connected to the SettingsDialog::deleteLogFileSignal signal.
 * @param logType
 *      The log type.
 */
void MainWindow::deleteLogFileSlot(QString logType)
{
    const Settings* currentSettings = m_settingsDialog->settings();

    if(logType == "html")
    {
        if(!currentSettings->htmlLogfileName.isEmpty())
        {
            m_handleData->m_htmlLogFile.remove();
            htmLogActivatedSlot(currentSettings->htmlLogFile);
        }
    }

    if(logType == "text")
    {
        if(!currentSettings->textLogfileName.isEmpty())
        {
            m_handleData->m_textLogFile.remove();
            textLogActivatedSlot(currentSettings->textLogFile);
        }
    }
    if(logType == "custom")
    {
        if(!currentSettings->customLogfileName.isEmpty())
        {
            m_handleData->m_customLogFile.remove();
            customLogActivatedSlot(currentSettings->logGenerateCustomLog);
        }
    }
}

/**
 * Reopens all open logs.
 */
void MainWindow::reopenLogsSlot(void)
{
    const Settings* currentSettings = m_settingsDialog->settings();

    if(currentSettings->htmlLogFile)
    {
        m_handleData->m_htmlLogFile.close();
        htmLogActivatedSlot(currentSettings->htmlLogFile);
    }

    if(currentSettings->textLogFile)
    {
        m_handleData->m_textLogFile.close();
        textLogActivatedSlot(currentSettings->textLogFile);
    }

    if(currentSettings->logGenerateCustomLog)
    {
        m_handleData->m_customLogFile.close();
        customLogActivatedSlot(currentSettings->logGenerateCustomLog);
    }
}

/**
 * Adds data to the main window send history.
 * @param data
 *      The data.
 */
void MainWindow::addDataToMainWindowSendHistorySlot(QByteArray data)
{
    m_handleData->addDataToSendHistory(&data);
}

/**
 * Converts a relative file path (relativ to rootFile) to an absolute file path.
 * @param rootFile
 *      The root file.
 * @param fileName
 *      The file name.
 * @return
 *      The absolute path.
 */
QString MainWindow::convertToAbsolutePath(QString rootFile, QString fileName)
{
    QString result;
    if(!fileName.isEmpty())
    {
        result = QDir(QFileInfo(rootFile).absolutePath()).absoluteFilePath(fileName);
        QString path = QDir(QFileInfo(result).absolutePath()).canonicalPath();

        if(!path.isEmpty())
        {
            QString fileName = QFileInfo(result).fileName();
            result = QDir(path).canonicalPath() + "/" + fileName;
        }
    }
    return result;
}
/**
 * Reads the main config fie list.
 *
 * @param removeDefaultMarker
 *      True if the default marker shall be removed.
 * @return
 *      The list.
 */
QStringList MainWindow::readMainConfigFileList(bool removeDefaultMarker)
{
    QStringList result;

    QFile inputFile(m_mainConfigFileList);
    if (inputFile.open(QIODevice::ReadOnly))
    {
        QTextStream in(&inputFile);
        while ( !in.atEnd() )
        {
            QString line = in.readLine();

            if(removeDefaultMarker)
            {
                line.remove("<DEFAULT_CONFIG_FILE>:");
            }

            if(!result.contains(line))
            {
                result.push_back(line);
            }
        }
        inputFile.close();
    }

    return result;
}

/**
 * This slot function is called if the send area splitter handle has been moved.
 * @param pos
 *      The new position of the splitter handle.
 * @param index
 *      The index of the splitter handle (always 0).
 */
void MainWindow::sendAreaSplitterMoved(int pos, int index)
{
    (void)pos;
    (void)index;
    QList<int> splitterSizes = m_userInterface->SendAreaSplitter->sizes();
    m_sendAreaSplitterSizeSecond = splitterSizes[1];

    //Restore the size of the second element in the send area inputs splitter.
    restoreSizeSplitterSecondElement(m_userInterface->SendAreaInputsSplitter, m_sendAreaInputsSplitterSizeSecond);
}

/**
 * Menu debug sequence script slot function.
 */
void MainWindow::debugSequenceScript(void)
{
    sendButtonPressedSlot(true);
}

/**
 * Is called if text of the cyclic script text edit has been changed.
 */
void MainWindow::scriptTextEditSlot(void)
{
    m_userInterface->actionDebugSequenceScript->setEnabled(false);

    if(m_userInterface->ScriptTextEdit->toPlainText().isEmpty())
    {
        m_userInterface->actionEditScript->setEnabled(false);
    }
    else
    {
        m_userInterface->actionEditScript->setEnabled(true);

        if(!m_userInterface->SendTextEdit->toPlainText().isEmpty())
        {
            m_userInterface->actionDebugSequenceScript->setEnabled(true);
        }
    }
}
/**
 * Is called when the user double clicks the script text edit.
 */
void MainWindow::scriptTextEditDoubleClickedSlot(void)
{
    addScriptSlot();
}

/**
 * This slot function is called if the send area inputs splitter handle has been moved.
 * @param pos
 *      The new position of the splitter handle.
 * @param index
 *      The index of the splitter handle (always 0).
 */
void MainWindow::sendAreaInputsSplitterMoved(int pos, int index)
{
    (void)pos;
    (void)index;
    QList<int> splitterSizes = m_userInterface->SendAreaInputsSplitter->sizes();
    m_sendAreaInputsSplitterSizeSecond = splitterSizes[1];
}

/**
 * This slot function is called if the tool box splitter handle has been moved.
 * @param pos
 *      The new position of the splitter handle.
 * @param index
 *      The index of the splitter handle (always 0).
 */
void MainWindow::toolBoxSplitterMoved(int pos, int index)
{
    (void)pos;
    (void)index;
    QList<int> splitterSizes = m_userInterface->ToolboxSplitter->sizes();
    m_toolBoxSplitterSizeSecond = splitterSizes[1];

    m_toolBoxSplitterSizesSecond[m_currentToolBoxIndex] = m_toolBoxSplitterSizeSecond;
}

/**
 * This slot function is called if the current index of the tool box has been changed.
 * @param index
 *      The new index.
 */
void MainWindow::currentToolBoxPageChangedSlot(int index)
{
    restoreSizeSplitterSecondElement(m_userInterface->ToolboxSplitter, m_toolBoxSplitterSizesSecond[index]);
    m_toolBoxSplitterSizeSecond = m_toolBoxSplitterSizesSecond[index];
    m_currentToolBoxIndex = index;
}

/**
 * Restores the size of the second element of the splitter (after the main window has been resized).
 * @param splitter
 *      The splitter.
 * @param oldSize
 *      The old size of the second element.
 */
void MainWindow::restoreSizeSplitterSecondElement(QSplitter* splitter, qint32 oldSize)
{
    if(oldSize > 0)
    {
        QList<int> splitterSizes = splitter->sizes();
        if(splitterSizes[1] > oldSize)
        {
            splitterSizes[0] += splitterSizes[1] - oldSize;
            splitterSizes[1] = oldSize;
        }
        else
        {
            splitterSizes[0] -= oldSize - splitterSizes[1];
            splitterSizes[1] = oldSize;
        }
        splitter->blockSignals(true);
        splitter->setSizes(splitterSizes);
        splitter->blockSignals(false);
    }
}

/**
 * Is called if the window is resized.
 * @param event
 *      The resize event.
 */
void MainWindow::resizeEvent(QResizeEvent* event)
{
    if(m_ignoreNextResizeEventTime.msecsTo(QDateTime::currentDateTime()) > 1000)
    {
        QMainWindow::resizeEvent(event);

        m_resizeTimer.stop();

        if(m_userInterface->ReceiveTextEditMixed->document()->characterCount() > 1)
        {
            m_resizeTimer.start(500);
        }
    }

    if(isVisible())
    {
        //Restore the size of the second element in the tool box splitter.
        restoreSizeSplitterSecondElement(m_userInterface->ToolboxSplitter, m_toolBoxSplitterSizeSecond);

        //Restore the size of the second element in the send area splitter
        restoreSizeSplitterSecondElement(m_userInterface->SendAreaSplitter, m_sendAreaSplitterSizeSecond);

        //Restore the size of the second element in the send area inputs splitter
        restoreSizeSplitterSecondElement(m_userInterface->SendAreaInputsSplitter, m_sendAreaInputsSplitterSizeSecond);

        m_handleData->calculateConsoleData();
    }
}

/**
 * Saves the main config fie list.
 * @param list
 *      The list.
 */
void MainWindow::saveMainConfigFileList(QStringList list)
{
    QFile file(m_mainConfigFileList);

    if(file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QTextStream out(&file);
        out.setCodec("UTF-8");

        for(auto el : list)
        {
            out << (el + "\n");
        }

        file.close();
    }
}

/**
 * Menu reset default config slot function.
 */
void MainWindow::resetDefaultConfigSlot(void)
{
    QStringList list = readMainConfigFileList();

    int index = list.indexOf(m_mainConfigFile);
    if(index != -1)
    {
        list.removeAt(index);
    }
    list.push_front(m_mainConfigFile);

    saveMainConfigFileList(list);
}

/**
 * Menu set current config to default slot function.
 */
void MainWindow::setCurrentConfigToDefaultSlot(void)
{
    QStringList list = readMainConfigFileList();

    int index = list.indexOf(m_mainConfigFile);
    if(index != -1)
    {
        list.removeAt(index);
    }
    list.push_front("<DEFAULT_CONFIG_FILE>:" + m_mainConfigFile);
    saveMainConfigFileList(list);
}
/**
 * Menu delete previous config list slot function.
 */
void MainWindow::deletePreviousConfigListSlot(void)
{
    QFile file(m_mainConfigFileList);
    file.remove();

    QStringList list;
    list.push_front(m_mainConfigFile);
    saveMainConfigFileList(list);
}

/**
 * This function exits ScriptCommunicator.
 */
void MainWindow::exitScriptCommunicator(void)
{
    close();
}

/**
 * Returns the console from the curretn tab.
 * Return NULL if the current tab has no console.
 */
QTextEdit* MainWindow::getConsoleFromCurrentTab(void)
{
    QTextEdit* textEdit = 0;

    if(m_userInterface->tabWidget->currentWidget() != 0)
    {
        QObjectList list = m_userInterface->tabWidget->currentWidget()->children();
        for(qint32 i = 0; i < list.size(); i++)
        {
            if(strcmp(list[i]->metaObject()->className(), "QTextEdit") == 0)
            {//The current tab has a console.
                textEdit = static_cast<QTextEdit*>(list[i]);
                break;
            }
        }
    }

    return textEdit;
}

/**
 * Is called if the tab index has been changed.
 *
 * @param index
 *      The new index.
 */
void MainWindow::tabIndexChangedSlot(int index)
{
    (void) index;
    m_searchConsole->activateDeactiveSearchButton();
}

/**
 * Menu save console slot function.
 */
void MainWindow::saveConsoleSlot()
{
    QTextEdit* textEdit = getConsoleFromCurrentTab();

    if(textEdit == 0)
    {
        QWidget* widget = QApplication::focusWidget();
        if(widget && (strcmp(widget->metaObject()->className(), "QTextEdit") == 0))
        {//The current focused widget is a text edit.
            textEdit = static_cast<QTextEdit*>(widget);
        }
    }

    if(textEdit)
    {//The current tab has a console.

        QString tmpFileName = QFileDialog::getSaveFileName(this, tr("Save Console"),
                                                           "",tr("TXT (*.txt);;HTML (*.html)"));
        if(!tmpFileName.isEmpty())
        {

            QStringList tmpList = tmpFileName.split(".");
            QString consoleContent;
            bool isHtml = false;

            if(tmpList.size() > 1)
            {
                if(tmpList[1] == "txt")
                {
                    consoleContent = textEdit->toPlainText();
                     isHtml = false;
                }
                else
                {
                    consoleContent = textEdit->toHtml();
                    isHtml = true;
                }
            }
            else
            {
                consoleContent = textEdit->toHtml();
                isHtml = true;
            }

            if(isHtml)
            {
                //QTextEdit returns Spaces as 160 (toHtml()).
                consoleContent.replace(160, ' ');


                //Write the background color, the font family and the font size into the HTML string.
                QPalette palette = textEdit->palette();
                QFont font = textEdit->font();
                int index = consoleContent.indexOf("<head>");

                //Write the style tag after the head tag.
                consoleContent.insert(index + 6, QString("<style>body {background-color:%1;color:%2;font-family:%3;font-size:%4;}</style>")
                                      .arg(palette.color(QPalette::Base).name(QColor::HexRgb))
                                      .arg(palette.color(QPalette::Text).name(QColor::HexRgb))
                                      .arg(font.family())
                                      .arg(font.pointSize()));
            }

            QFile file(tmpFileName);

            if(file.open(QIODevice::WriteOnly))
            {
                QByteArray data = consoleContent.toLocal8Bit();
                file.write(data);
                file.close();
            }
            else
            {
                QMessageBox::critical(this, "error", "could not open " + tmpFileName);
            }
        }
    }//if(textEdit)
    else
    {
        QMessageBox::information(this, "print console", "the current tab has no console which has the focus");
    }
}

/**
 * Menu submit bug slot function.
 */
void MainWindow::submitBugSlot()
{
    QString text = "To report a bug go to";
    text.append("<br><a href=\"http://sourceforge.net/p/scriptcommunicator/discussion/bugreports/\">http://sourceforge.net/p/scriptcommunicator/discussion/bugreports</a>");
    text.append("<br>and create a topic with your bug");

    QMessageBox msgBox(QMessageBox::Information, "report a bug",text);
    msgBox.setTextFormat(Qt::RichText);
    msgBox.exec();
}

/**
 * Menu request feature slot function.
 */
void MainWindow::requestFeatureSlot()
{
    QString text = "To request a new feature go to";
    text.append("<br><a href=\"http://sourceforge.net/p/scriptcommunicator/discussion/featurerequests//\">http://sourceforge.net/p/scriptcommunicator/discussion/featurerequests</a>");
    text.append("<br>and create a topic with the desired feature");

    QMessageBox msgBox(QMessageBox::Information, "request a feature",text);
    msgBox.setTextFormat(Qt::RichText);
    msgBox.exec();
}


/**
 * Menu edit script slot function.
 */
void MainWindow::editScriptSlot()
{
    QStringList arguments;
    arguments << m_userInterface->ScriptTextEdit->toPlainText();
    openScriptEditor(arguments, m_settingsDialog->settings(), this);
}

/**
 * Menu add script slot function.
 */
void MainWindow::addScriptSlot()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open sequence script file"),
                                                    "", tr("sequence script files (*.js);;Files (*)"));
    if(!fileName.isEmpty())
    {
        m_userInterface->ScriptTextEdit->setPlainText(fileName);
        saveSettings();
    }
}


/**
 * Menu check for updates slot function.
 */
void MainWindow::checkForUpdatesSlot()
{
    Settings settings = *m_settingsDialog->settings();
    QString type = "NO_PROXY";
    if(settings.updateSettings.proxySettings == 1)
    {
        type = "SYSTEM_PROXY";
    }
    else if(settings.updateSettings.proxySettings == 2)
    {
        type = "CUSTOM_PROXY";
    }
    else
    {
        type = "NO_PROXY";
    }

    QNetworkProxy proxy = ScriptTcpClient::createProxy(type, settings.updateSettings.proxyUserName,
                                                       settings.updateSettings.proxyPassword,
                                                       settings.updateSettings.proxyIpAddress,
                                                       settings.updateSettings.proxyPort);

    m_userInterface->actionCheckForUpdates->setEnabled(false);
    updatesManager = new QNetworkAccessManager(this);
    updatesManager->setProxy(proxy);
    connect(updatesManager, SIGNAL(finished(QNetworkReply*)),this, SLOT(updateManagerReplyFinished(QNetworkReply*)));

    updatesManager->get(QNetworkRequest(QUrl("https://github.com/szieke/ScriptCommunicator_serial-terminal/blob/master/README.md")));
}

/**
 * Slot function for the update manager replies.
 * @param reply
 *      The replies.
 */
void MainWindow::updateManagerReplyFinished(QNetworkReply* reply)
{
    QNetworkReply::NetworkError error = reply->error();

    if(error == QNetworkReply::NoError)
    {
        QString result = reply->readAll();
        QStringList list = result.split("Downloads (release ");

        bool pageIsInvalid = false;
        if(list.length() == 2)
        {
            QString version = list[1].left(5);

            list = version.split(".");
            if(list.length() == 2)
            {
                bool newVersionAvailable = false;
                QStringList tmpList = VERSION.split(".");
                quint32 currentMajor = tmpList[0].toUInt();
                quint32 currentMinor = tmpList[1].toUInt();

                quint32 serverMajor = list[0].toUInt();
                quint32 serverMinor = list[1].toUInt();

                if(serverMajor > currentMajor)
                {
                    newVersionAvailable = true;
                }
                else if(serverMajor == currentMajor)
                {
                    if(serverMinor > currentMinor)
                    {
                        newVersionAvailable = true;
                    }
                }

                if(newVersionAvailable)
                {
                    QString text = "A new version of ScriptCommunicator is available. To download this new version go to: ";
                    text.append("<a href=\"http://sourceforge.net/p/scriptcommunicator/\">http://sourceforge.net/p/scriptcommunicator/</a>");

                    QMessageBox msgBox(QMessageBox::Information, "update available",text);
                    msgBox.setTextFormat(Qt::RichText);
                    msgBox.exec();
                }
                else
                {
                    QMessageBox msgBox(QMessageBox::Information, "no update available", "ScriptCommunicator is up to date.");
                    msgBox.setTextFormat(Qt::RichText);
                    msgBox.exec();
                }
            }
            else
            {
                pageIsInvalid = true;
            }

        }//if(list.length() == 2)
        else
        {
            pageIsInvalid = true;
        }

        if(pageIsInvalid)
        {
            QMessageBox msgBox(QMessageBox::Information, "error while searching for an update", "Invalid data received: " + result);
            msgBox.setTextFormat(Qt::PlainText);
            msgBox.exec();
        }
    }
    else
    {
        QString text = "Connection error (eror code: %1). Are you connected with the internet and are your proxy settings (update tab in the setting dialog) correct?";
        text = text.arg(error);
        QMessageBox msgBox(QMessageBox::Information, "connection error",text);
        msgBox.setTextFormat(Qt::RichText);
        msgBox.exec();
    }

    m_userInterface->actionCheckForUpdates->setEnabled(true);
    reply->deleteLater();
}

/**
 * Menu get support slot function.
 */
void MainWindow::getSupportSlot()
{
    QString text = "To get support go to";
    text.append("<br><a href=\"http://sourceforge.net/p/scriptcommunicator/discussion/general//\">http://sourceforge.net/p/scriptcommunicator/discussion/general</a>");
    text.append("<br>and create a topic.");

    QMessageBox msgBox(QMessageBox::Information, "get support",text);
    msgBox.setTextFormat(Qt::RichText);
    msgBox.exec();
}

/**
 * Menu video slot function.
 */
void MainWindow::watchVideoSlot()
{
    QString text = "A video which demonstrates the basic features of ScriptCommunicator can be found here:";
    text.append("<br><a href=\"https://www.youtube.com/playlist?list=PLniMuy2Q_xGuFB_kl1nte2mDxfeeOu8ce\">https://www.youtube.com/playlist?list=PLniMuy2Q_xGuFB_kl1nte2mDxfeeOu8ce</a>");

    QMessageBox msgBox(QMessageBox::Information, "video",text);
    msgBox.setTextFormat(Qt::RichText);
    msgBox.exec();
}

/**
 * Menu print console slot function.
 */
void MainWindow::printConsoleSlot()
{
    QTextEdit* textEdit = getConsoleFromCurrentTab();

    if(textEdit == 0)
    {
        QWidget* widget = QApplication::focusWidget();
        if(widget && (strcmp(widget->metaObject()->className(), "QTextEdit") == 0))
        {//The current focused widget is a text edit.
            textEdit = static_cast<QTextEdit*>(widget);
        }
    }


    if(textEdit)
    {
        QPrinter printer(QPrinter::HighResolution);
        printer.setFullPage(true);

        QPrintDialog dialog(&printer, this);
        dialog.setWindowTitle(tr("Print Console"));

        if (dialog.exec() == QDialog::Accepted)
        {
            QTextDocument *doc =  textEdit->document();
            doc->print(&printer);
        }
    }
    else
    {
        QMessageBox::information(this, "print console", "the current tab has no console which has the focus");
    }
}

/**
 * Menu copy config slot function.
 */
void MainWindow::copyConfigSlot()
{
    QString tmpFileName = QFileDialog::getSaveFileName(this, tr("Copy main config file"),
                                                       getAndCreateProgramUserFolder(), tr("main config files (*.config);;Files (*)"));
    if(!tmpFileName.isEmpty())
    {
        saveSettings();

        m_configLockFileTimer.stop();
        m_mainConfigLockFile.close();
        QFile::remove(m_mainConfigLockFile.fileName());

        if(m_scriptWindow->isVisible())
        {
            m_scriptWindow->close();
        }
        if(m_settingsDialog->isVisible())
        {
            m_settingsDialog->close();
        }
        if(m_sendWindow->isVisible())
        {
            m_sendWindow->close();
        }
        if(m_scriptWindow->getCreateSceFileDialog()->isVisible())
        {
            m_scriptWindow->getCreateSceFileDialog()->close();
        }


        QStringList list = readMainConfigFileList(true);
        int index = list.indexOf("<DEFAULT_CONFIG_FILE>:" + tmpFileName);
        if(index == -1)
        {
            index = list.indexOf(tmpFileName);
            if(index != -1)
            {
                list.removeAt(index);
            }
            list.push_front(tmpFileName);
        }
        saveMainConfigFileList(list);

        m_mainConfigFile = tmpFileName;
        saveSettings();

        m_mainConfigLockFile.setFileName(m_mainConfigFile + ".lock");
        m_mainConfigLockFile.open(QIODevice::WriteOnly);
        m_configLockFileTimer.start(2000);

        loadSettings();
        inititializeTab();
    }
}

/**
 * Creates a new config and loads it.
 * @param isCallFromButton
 *      True if this function is called from a button slot function.
 */
bool MainWindow::createConfig(bool isCallFromButton)
{
    bool newConfigUsed = false;
    QString tmpFileName = QFileDialog::getSaveFileName(this, tr("Create main config file"),
                                                       getAndCreateProgramUserFolder(), tr("main config files (*.config);;Files (*)"));
    if(!tmpFileName.isEmpty())
    {
        if(isCallFromButton)
        {
            saveSettings();
        }

        QString templateFile = getScriptCommunicatorFilesFolder() + "/templates/" + INIT_MAIN_CONFIG_FILE;
        QFile(tmpFileName).remove();

        if(QFile::copy(templateFile, tmpFileName))
        {
            m_configLockFileTimer.stop();
            m_mainConfigLockFile.close();
            QFile::remove(m_mainConfigLockFile.fileName());

            if(m_scriptWindow->isVisible())
            {
                m_scriptWindow->close();
            }
            if(m_settingsDialog->isVisible())
            {
                m_settingsDialog->close();
            }
            if(m_sendWindow->isVisible())
            {
                m_sendWindow->close();
            }
            if(m_scriptWindow->getCreateSceFileDialog()->isVisible())
            {
                m_scriptWindow->getCreateSceFileDialog()->close();
            }

            QStringList list = readMainConfigFileList(true);
            int index = list.indexOf("<DEFAULT_CONFIG_FILE>:" + tmpFileName);
            if(index == -1)
            {
                index = list.indexOf(tmpFileName);
                if(index != -1)
                {
                    list.removeAt(index);
                }
                list.push_front(tmpFileName);
            }
            saveMainConfigFileList(list);


            saveSettings();

            disconnect(m_sendWindow, SIGNAL(configHasToBeSavedSignal()),this, SLOT(configHasToBeSavedSlot()));
            disconnect(m_scriptWindow, SIGNAL(configHasToBeSavedSignal()),this, SLOT(configHasToBeSavedSlot()));
            disconnect(m_scriptWindow->getCreateSceFileDialog(), SIGNAL(configHasToBeSavedSignal()),this, SLOT(configHasToBeSavedSlot()));

            m_handleData->m_sendHistory.clear();
            m_userInterface->historyTextEdit->clear();
            m_searchConsole->m_lastSearchString.clear();
            m_userInterface->findWhatComboBox->clear();
            m_sendWindow->unloadFileSlot();
            m_scriptWindow->unloadConfigSlot();
            m_scriptWindow->getCreateSceFileDialog()->unloadConfigSlot();

            m_mainConfigFile = tmpFileName;
            m_mainConfigLockFile.setFileName(m_mainConfigFile + ".lock");
            m_mainConfigLockFile.open(QIODevice::WriteOnly);
            m_configLockFileTimer.start(2000);

            m_isFirstProgramStart = false;
            loadSettings();
            inititializeTab();

            connect(m_sendWindow, SIGNAL(configHasToBeSavedSignal()),this, SLOT(configHasToBeSavedSlot()));
            connect(m_scriptWindow, SIGNAL(configHasToBeSavedSignal()),this, SLOT(configHasToBeSavedSlot()));
            connect(m_scriptWindow->getCreateSceFileDialog(), SIGNAL(configHasToBeSavedSignal()),this, SLOT(configHasToBeSavedSlot()));

            m_sendWindow->setCurrentCyclicScript("");
            m_sendWindow->setCurrentSendString("");

            Settings currentSettings = *m_settingsDialog->settings();
            currentSettings.consoleScript = "";
            currentSettings.logScript = "";
            m_settingsDialog->setAllSettingsSlot(currentSettings, true);


            newConfigUsed = true;
        }
        else
        {
            QMessageBox::critical(this, "error", QString("could not copy file %1 to %2").arg(templateFile, tmpFileName));
        }
    }

    return newConfigUsed;
}

/**
 * Menu create config slot function.
 */
void MainWindow::createConfigSlot()
{
    createConfig(true);
}

/**
 * Menu load previous config slot function.
 */
void MainWindow::loadPreviousConfigSlot()
{

    bool okPressed;
    QStringList list = readMainConfigFileList();
    QString result = QInputDialog::getItem(this, "select ScriptCommunicator config file","Note: The selected config can be made to default (config menu)",
                                           list, 0, false, &okPressed, Qt::WindowStaysOnTopHint);
    if(okPressed)
    {
        bool loadConfig = true;
        QFileInfo fi(result + ".lock");
        if(fi.exists())
        {//The main config file is locked
            showYesNoDialogSlot(QMessageBox::Warning, "config file is locked", result + " is already in use.\nUse it anyway?",
                                0, &loadConfig);
        }//if(fi.exists())

        if(loadConfig)
        {
            m_configLockFileTimer.stop();
            m_mainConfigLockFile.close();
            QFile::remove(m_mainConfigLockFile.fileName());

            saveSettings();

            if(m_scriptWindow->isVisible())
            {
                m_scriptWindow->close();
            }
            if(m_settingsDialog->isVisible())
            {
                m_settingsDialog->close();
            }
            if(m_sendWindow->isVisible())
            {
                m_sendWindow->close();
            }
            if(m_scriptWindow->getCreateSceFileDialog()->isVisible())
            {
                m_scriptWindow->getCreateSceFileDialog()->close();
            }

            QStringList list = readMainConfigFileList(true);
            int index = list.indexOf("<DEFAULT_CONFIG_FILE>:" + result);
            if(index == -1)
            {
                index = list.indexOf(result);
                if(index != -1)
                {
                    list.removeAt(index);
                }
                list.push_front(result);
            }
            saveMainConfigFileList(list);

            m_mainConfigFile = result;
            m_mainConfigLockFile.setFileName(m_mainConfigFile + ".lock");
            m_mainConfigLockFile.open(QIODevice::WriteOnly);
            m_configLockFileTimer.start(2000);

            loadSettings();
            inititializeTab();
        }
    }
}

/**
 * Menu load config slot function.
 */
void MainWindow::loadConfigSlot()
{
    QString tmpFileName = QFileDialog::getOpenFileName(this, tr("Open main config file"),
                                                       getAndCreateProgramUserFolder(),tr("main config files (*.config);;Files (*)"));

    if(!tmpFileName.isEmpty())
    {
        bool loadConfig = true;
        QFileInfo fi(tmpFileName + ".lock");
        if(fi.exists())
        {//The main config file is locked
            showYesNoDialogSlot(QMessageBox::Warning, "config file is locked", tmpFileName + " is already in use.\nUse it anyway?",
                                0, &loadConfig);
        }//if(fi.exists())

        if(loadConfig)
        {
            m_configLockFileTimer.stop();
            m_mainConfigLockFile.close();
            QFile::remove(m_mainConfigLockFile.fileName());

            saveSettings();

            if(m_scriptWindow->isVisible())
            {
                m_scriptWindow->close();
            }
            if(m_settingsDialog->isVisible())
            {
                m_settingsDialog->close();
            }
            if(m_sendWindow->isVisible())
            {
                m_sendWindow->close();
            }
            if(m_scriptWindow->getCreateSceFileDialog()->isVisible())
            {
                m_scriptWindow->getCreateSceFileDialog()->close();
            }

            QStringList list = readMainConfigFileList(true);
            int index = list.indexOf("<DEFAULT_CONFIG_FILE>:" + tmpFileName);
            if(index == -1)
            {
                index = list.indexOf(tmpFileName);
                if(index != -1)
                {
                    list.removeAt(index);
                }
                list.push_front(tmpFileName);
            }
            saveMainConfigFileList(list);

            m_mainConfigFile = tmpFileName;

            m_mainConfigLockFile.setFileName(m_mainConfigFile + ".lock");
            m_mainConfigLockFile.open(QIODevice::WriteOnly);
            m_configLockFileTimer.start(2000);

            loadSettings();
            inititializeTab();
        }
    }
}

/**
 * Checks the parsed mimium ScriptCommunicator version (SCE file).
 * @param version
 *      The parsed verion.
 * @return
 *      True if the current ScripCommunicator version is sufficient to execute the sce file.
 */
bool MainWindow::checkParsedScVersion(QString version)
{
    bool result = true;

    if(!version.isEmpty())
    {
        QStringList tmpList = VERSION.split(".");
        quint32 currentMajor = tmpList[0].toUInt();
        quint32 currentMinor = tmpList[1].toUInt();

        tmpList = version.split(".");
        if(tmpList.length() == 2)
        {
            bool versionIsOk = false;
            quint32 neededMajor = tmpList[0].toUInt();
            quint32 neededMinor = tmpList[1].toUInt();

            if(neededMajor < currentMajor)
            {
                versionIsOk = true;
            }
            else if(neededMajor == currentMajor)
            {
                if(neededMinor <= currentMinor)
                {
                    versionIsOk = true;
                }
                else
                {
                    versionIsOk = false;
                }

            }
            else
            {//neededMajor > currentMajor
                versionIsOk = false;
            }

            if(!versionIsOk)
            {
                result = false;
                QString neededMinorString = (neededMinor < 10) ? QString("0%1").arg(neededMinor) : QString("%1").arg(neededMinor);
                QString neededMajorString = (neededMajor < 10) ? QString("0%1").arg(neededMajor) : QString("%1").arg(neededMajor);

                QMessageBox box(QMessageBox::Warning, "ScriptCommunicator", QString("The current used version of ScriptCommunicator is to old to execute the current sce file.\nThe needed version is %1.%2"
                                                                                    " and the current version is %3").arg(neededMajorString).arg(neededMinorString).arg(VERSION));
                QApplication::setActiveWindow(&box);
                box.exec();
            }

        }
        else
        {
            result = false;
            QMessageBox box(QMessageBox::Warning, "ScriptCommunicator", QString("invalid version in sce file: ") + version);
            QApplication::setActiveWindow(&box);
            box.exec();
        }
    }


    return result;
}

/**
 * Checks the hash of a scez file.
 * @param fileName
 *      The file name.
 * @return
 *      True on success.
 */
bool MainWindow::checkScezFileHash(QString fileName)
{
    QFile inFile(fileName);
    bool success = inFile.open(QIODevice::ReadOnly);
    if(success)
    {
        qint64 readData = 0;
        QCryptographicHash hashObject(QCryptographicHash::Sha512);
        QByteArray readDataArray;
        while((inFile.size() - 64) > readData)
        {
            readDataArray = inFile.read(100000000);
            readData += readDataArray.length();

            if(readData + 64 >= inFile.size())
            {
                int bytesToRemove = readData - (inFile.size() - 64);
                readDataArray.remove(readDataArray.length() - bytesToRemove, bytesToRemove);
            }
            hashObject.addData(readDataArray);
        }
        QByteArray createdHash = hashObject.result();
        inFile.seek(inFile.size() - 64);
        QByteArray readHeash = inFile.read(64);

        success = (createdHash == readHeash) ? true : false;
        inFile.close();

        if(!success)
        {
            QMessageBox box(QMessageBox::Warning, "ScriptCommunicator", QString("invalid SHA-512 hash in: ") + fileName);
            QApplication::setActiveWindow(&box);
            box.exec();
        }
    }
    else
    {
        QMessageBox box(QMessageBox::Warning, "ScriptCommunicator", QString("could not open: ") + fileName);
        QApplication::setActiveWindow(&box);
        box.exec();
    }

    return success;
}

/**
 * Parses an SCE file.
 * @param fileName
 *      The ECS file name.
 * @param scripts
 *      The parsed scripts.
 * @param extraPluginPaths
 *      The parsed extra plugin paths.
 * @param scriptArguments
 *      The parsed scripts arguments.
 * @param withScriptWindow
 *      The parsed withScriptWindow argument.
 * @param scriptWindowIsMinimized
 *      The parsed scriptWindowIsMinimized argument.
 * @param minimumScVersion
 *      The min. ScriptCommunicator version which is needed to execute this sce file.
 * @param extraPlugInPath
 *      The parsed extra library paths.
 * @return
 *      True on success.
 */
bool MainWindow::parseSceFile(QString fileName, QStringList* scripts, QStringList* extraPluginPaths, QStringList* scriptArguments,
                              bool* withScriptWindow, bool* scriptWindowIsMinimized, QString* minimumScVersion, QStringList *extraPlugInPath)
{
    QFile ecsFile(fileName);
    QDomDocument doc("ScriptCommunicatorExecutable");
    bool result = true;


    if (ecsFile.open(QIODevice::ReadOnly))
    {
        QByteArray content = ecsFile.readAll();
        ecsFile.close();
        if (!doc.setContent(content))
        {
            QMessageBox box(QMessageBox::Warning, "ScriptCommunicator", "could not parse " + fileName);
            QApplication::setActiveWindow(&box);
            box.exec();
            result = false;
        }
        else
        {
            QString ecsFilePath = QFileInfo(fileName).path();
            QDomElement docElem = doc.documentElement();

            QDomNodeList itemList = docElem.elementsByTagName("Options");
            QDomNode nodeSciptWindowItem = itemList.at(0);

            *withScriptWindow = nodeSciptWindowItem.attributes().namedItem("withScriptWindow").nodeValue().toUInt() ? true : false;
            *scriptWindowIsMinimized = nodeSciptWindowItem.attributes().namedItem("notMinimized").nodeValue().toUInt() ? false : true;
            *minimumScVersion = nodeSciptWindowItem.attributes().namedItem("minScVersion").nodeValue();


            //Parse all extra library paths.
            QDomNodeList nodeList = docElem.elementsByTagName("LibraryPath");
            for (int i = 0; i < nodeList.size(); i++)
            {
                QDomNode nodeScriptItem = nodeList.at(i);
                QString libPath = nodeScriptItem.attributes().namedItem("path").nodeValue();
                if(!libPath.isEmpty())
                {
                    if(libPath.startsWith("./"))
                    {
                        libPath.remove(0, 2);
                        libPath = ecsFilePath + "/" + libPath;
                    }
                    else if(libPath.startsWith("../"))
                    {
                        libPath = ecsFilePath + "/" + libPath;
                    }
                    *extraPlugInPath << libPath;
                }
            }

            //Parse all scripts.
            nodeList = docElem.elementsByTagName("Script");
            for (int i = 0; i < nodeList.size(); i++)
            {
                QDomNode nodeScriptItem = nodeList.at(i);
                QString script = nodeScriptItem.attributes().namedItem("path").nodeValue();

                if(script.startsWith("./"))
                {
                    script.remove(0, 2);
                    script = ecsFilePath + "/" + script;
                }
                else if(script.startsWith("../"))
                {
                    script = ecsFilePath + "/" + script;
                }
                *scripts << script;
            }

            //Parse all scripts arguments.
            nodeList = docElem.elementsByTagName("ScriptArgument");
            for (int i = 0; i < nodeList.size(); i++)
            {
                QDomNode nodeScriptArgItem = nodeList.at(i);
                *scriptArguments << nodeScriptArgItem.attributes().namedItem("value").nodeValue();
            }

            //Parse all extra plugin paths.
            nodeList = docElem.elementsByTagName("PluginPath");
            for (int i = 0; i < nodeList.size(); i++)
            {
                QDomNode nodePluginPathItem = nodeList.at(i);
                QString path = nodePluginPathItem.attributes().namedItem("path").nodeValue();
                if(path.startsWith("./"))
                {
                    path.remove(0, 2);
                    path = ecsFilePath + "/" + path;
                }
                else if(path.startsWith("../"))
                {
                    path = ecsFilePath + "/" + path;
                }
                *extraPluginPaths << path;
            }


        }
    }//if (ecsFile.open(QIODevice::ReadOnly))
    else
    {
        QMessageBox box(QMessageBox::Warning, "ScriptCommunicator", "could not open " + fileName);
        QApplication::setActiveWindow(&box);
        box.exec();
        result = false;
    }

    return result;

}

#include "mainwindowHandleData.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "settingsdialog.h"
#include "customConsoleLogObject.h"
#include <QScrollBar>
#include "canTab.h"
#include "mainInterfaceThread.h"
#include <QMessageBox>

/**
 * Constructor.
 * @param mainWindow
 *      Pointer tothe main window.
 * @param settingsDialog
 *      Pointer to the settings dialog.
 * @param userInterface
 *      Pointer to the main window user interface.
 */
MainWindowHandleData::MainWindowHandleData(MainWindow *mainWindow, SettingsDialog *settingsDialog, Ui::MainWindow *userInterface) :
    QObject(mainWindow), m_mainWindow(mainWindow), m_settingsDialog(settingsDialog), m_userInterface(userInterface), m_receivedBytes(0),
    m_sentBytes(0),m_htmlLogFile(), m_HtmlLogFileStream(&m_htmlLogFile),
    m_textLogFile(), m_customLogFile(), m_textLogFileStream(&m_textLogFile), m_customLogFileStream(&m_customLogFile),
    m_bytesInUnprocessedConsoleData(0), m_bytesInStoredConsoleData(0), m_bytesSinceLastNewLineInConsole(0), m_bytesSinceLastNewLineInLog(0),
    m_customLogString(), m_customConsoleObject(0), m_customLogObject(0), m_customConsoleStrings(), m_customConsoleStoredStrings(),
    m_numberOfBytesInCustomConsoleStrings(0), m_numberOfBytesInCustomConsoleStoredStrings(0), m_historySendIsInProgress(false), m_checkDebugWindowsIsClosed(),
    m_queuedReceivedData(), m_queuedReceivedDataTimer()
{
    m_customConsoleObject = new CustomConsoleLogObject(m_mainWindow);
    m_customLogObject = new CustomConsoleLogObject(m_mainWindow);

    m_updateConsoleAndLogTimer = new QTimer(this);
    m_updateConsoleAndLogTimer->setSingleShot(true);
    connect(m_updateConsoleAndLogTimer, SIGNAL(timeout()), this, SLOT(updateConsoleAndLog()));

    m_queuedReceivedDataTimer.setSingleShot(true);
    connect(&m_queuedReceivedDataTimer, SIGNAL(timeout()), this, SLOT(queuedDataReceivedSlot()));

}

/**
 * Destructor.
 */
MainWindowHandleData::~MainWindowHandleData()
{
    delete m_updateConsoleAndLogTimer;
    delete m_customConsoleObject;
    delete m_customLogObject;
}


/**
 * The function append the received/send data to the consoles and the logs.
 * This function is called periodically (log update interval).
 */
void MainWindowHandleData::updateConsoleAndLog(void)
{
    const Settings* settings = m_settingsDialog->settings();

    m_updateConsoleAndLogTimer->stop();

    //Create the log entries and the console strings.
    processDataInStoredData();

    if(settings->logGenerateCustomLog)
    {
        if(!m_customLogFile.isOpen() && (settings->customLogfileName != ""))
        {
            m_mainWindow->customLogActivatedSlot(true);
        }

        if(m_customLogFile.isOpen())
        {
            m_customLogFileStream << m_customLogString.toLocal8Bit();
            m_customLogFileStream.flush();
            m_customLogString.clear();
        }
    }


    //Limit the data in m_storedConsoleData to settings->maxCharsInConsole.
    while(m_bytesInStoredConsoleData > settings->maxCharsInConsole)
    {
        int diff = m_bytesInStoredConsoleData - settings->maxCharsInConsole;
        if(diff >= m_storedConsoleData.at(0).data.length())
        {
            m_bytesInStoredConsoleData -= m_storedConsoleData.at(0).data.length();
            m_storedConsoleData.removeAt(0);
        }
        else
        {
            m_storedConsoleData[0].data.remove(0, diff);
            m_bytesInStoredConsoleData -= diff;
        }
    }

    m_mainWindow->setUpdatesEnabled(false);

    /*********Append the console strings to the corresponding console and clear them.***********/
    if(settings->showAsciiInConsole){m_mainWindow->appendConsoleStringToConsole(&m_consoleDataBufferAscii, m_userInterface->ReceiveTextEditAscii);}
    else{m_consoleDataBufferAscii.clear();}

    if(settings->showHexInConsole){m_mainWindow->appendConsoleStringToConsole(&m_consoleDataBufferHex, m_userInterface->ReceiveTextEditHex);}
    else{m_consoleDataBufferHex.clear();}

    if(settings->showDecimalInConsole){m_mainWindow->appendConsoleStringToConsole(&m_consoleDataBufferDec, m_userInterface->ReceiveTextEditDecimal);}
    else{m_consoleDataBufferDec.clear();}

    if(settings->showMixedConsole){m_mainWindow->appendConsoleStringToConsole(&m_consoleDataBufferMixed, m_userInterface->ReceiveTextEditMixed);}
    else{m_consoleDataBufferMixed.clear();}

    if(settings->showBinaryConsole){m_mainWindow->appendConsoleStringToConsole(&m_consoleDataBufferBinary, m_userInterface->ReceiveTextEditBinary);}
    else{ m_consoleDataBufferBinary.clear();}

    if(settings->consoleShowCustomConsole)
    {
        QString consoleString;
        for(auto el : m_customConsoleStrings)
        {
            m_customConsoleStoredStrings.append(el);
            m_numberOfBytesInCustomConsoleStoredStrings += el.length();
            consoleString += el;

        }

        //Limit the number of bytes in m_customConsoleStoredStrings.
        while(m_numberOfBytesInCustomConsoleStoredStrings > (settings->maxCharsInConsole * 2))
        {
            int diff = m_numberOfBytesInCustomConsoleStoredStrings - (settings->maxCharsInConsole * 2);
            if(diff >= m_customConsoleStoredStrings.at(0).length())
            {
                m_numberOfBytesInCustomConsoleStoredStrings -= m_customConsoleStoredStrings.first().length();
                m_customConsoleStoredStrings.removeFirst();
            }
            else
            {
                m_customConsoleStoredStrings.first().remove(0, m_numberOfBytesInCustomConsoleStoredStrings - settings->maxCharsInConsole);
                m_numberOfBytesInCustomConsoleStoredStrings -= m_numberOfBytesInCustomConsoleStoredStrings - settings->maxCharsInConsole;
            }
        }


        if(!consoleString.isEmpty())
        {
            bool consoleWasEmpty = (m_userInterface->ReceiveTextEditCustom->document()->characterCount() <= 1) ? true : false;
            consoleString = m_consoleData.htmlReceived + consoleString + QString("</span>");

            m_mainWindow->appendConsoleStringToConsole(&consoleString, m_userInterface->ReceiveTextEditCustom);
            m_customConsoleStrings.clear();
            m_numberOfBytesInCustomConsoleStrings = 0;

            if(consoleWasEmpty)
            {
                //Force the custom console to render the content.
                QRect rect = MainWindow::windowPositionAndSize(m_mainWindow);
                rect.setWidth(rect.width() + 1);
                m_mainWindow->m_ignoreNextResizeEventTime = QDateTime::currentDateTime();
                MainWindow::setWindowPositionAndSize(m_mainWindow, rect);

                rect.setWidth(rect.width() - 1);
                MainWindow::setWindowPositionAndSize(m_mainWindow, rect);
            }

        }
    }

    /***************************************************************************************************/

    m_mainWindow->showNumberOfReceivedAndSentBytes();
    m_mainWindow->setUpdatesEnabled(true);
}


/**
 * Appends data to the m_unprocessedConsoleData and m_unprocessedLogData
 * @param data
 *      The data.
 * @param isSend
 *      True if the data has been sent and false if the data has been received.
 * @param isUserMessage
 *      True if the data is a user message.
 * @param isFromCan
 *      True if the data is from CAN.
 * @param forceTimeStamp
 *      True if a time stamp shall be generated (independently from the time stamp settings)
 */
void MainWindowHandleData::appendDataToStoredData(QByteArray &data, bool isSend, bool isUserMessage, bool isFromCan, bool forceTimeStamp)
{
    const Settings* currentSettings = m_settingsDialog->settings();

    if(currentSettings->htmlLogFile || currentSettings->textLogFile)
    {
        if((!isSend && currentSettings->writeReceivedDataInToLog) || (isSend && currentSettings->writeSendDataInToLog) || isUserMessage)
        {
            appendUnprocessLogData(data, isSend, isUserMessage, isFromCan, forceTimeStamp);
        }
    }

    if(currentSettings->logGenerateCustomLog)
    {
        if((!isSend && currentSettings->writeReceivedDataInToLog) || (isSend && currentSettings->writeSendDataInToLog) || isUserMessage)
        {
            bool errorOccured;
            QString timeStamp = QDateTime::currentDateTime().toString(currentSettings->consoleTimestampFormat).toLocal8Bit();
            m_customLogString += m_customLogObject->callScriptFunction(&data, timeStamp, isSend, isUserMessage, isFromCan, true, &errorOccured);

            if(errorOccured)
            {
                Settings settings = *m_settingsDialog->settings();
                settings.logGenerateCustomLog = false;
                settings.logDebugCustomLog = false;
                m_settingsDialog->setAllSettingsSlot(settings, false);

                if(m_customLogObject->scriptIsBlocked())
                {
                    QMessageBox::critical(m_mainWindow, "error in custom log script", settings.consoleScript + " is blocked or overloaded and has been disabled.");
                }
            }


        }
    }

    if((!isSend && currentSettings->showReceivedDataInConsole) || (isSend && currentSettings->showSendDataInConsole) || isUserMessage)
    {
        appendUnprocessConsoleData(data, isSend, isUserMessage, isFromCan, forceTimeStamp);

        if(currentSettings->consoleShowCustomConsole)
        {
            bool errorOccured;
            QString timeStamp = QDateTime::currentDateTime().toString(currentSettings->consoleTimestampFormat).toLocal8Bit();
            QString result = m_customConsoleObject->callScriptFunction(&data, timeStamp, isSend, isUserMessage, isFromCan, false, &errorOccured);

            if(errorOccured)
            {
                Settings settings = *m_settingsDialog->settings();
                settings.consoleShowCustomConsole = false;
                settings.consoleDebugCustomConsole= false;
                m_settingsDialog->setAllSettingsSlot(settings, false);
                m_mainWindow->inititializeTab();

                if(m_customConsoleObject->scriptIsBlocked())
                {
                    QMessageBox::critical(m_mainWindow, "error in custom console script", settings.consoleScript + " is blocked or overloaded and has been disabled.");
                }

            }
            else
            {
                m_customConsoleStrings.append(result);
                m_numberOfBytesInCustomConsoleStrings += result.length();
            }


            if ((m_numberOfBytesInCustomConsoleStrings > (currentSettings->maxCharsInConsole * 2)))
            {
                while(m_numberOfBytesInCustomConsoleStrings > (currentSettings->maxCharsInConsole * 2))
                {//Limit the number of bytes in m_customConsoleStrings.

                    int diff = m_numberOfBytesInCustomConsoleStrings - (currentSettings->maxCharsInConsole * 2);
                    if(diff >= m_customConsoleStrings.at(0).length())
                    {
                        m_numberOfBytesInCustomConsoleStrings -= m_customConsoleStrings.first().length();
                        m_customConsoleStrings.removeFirst();
                    }
                    else
                    {
                        m_customConsoleStrings.first().remove(0, m_numberOfBytesInCustomConsoleStrings - currentSettings->maxCharsInConsole);
                        m_numberOfBytesInCustomConsoleStrings -= m_numberOfBytesInCustomConsoleStrings - currentSettings->maxCharsInConsole;
                    }
                }

                if(m_bytesInStoredConsoleData != 0)
                {
                    //Restart the console/log timer.
                    m_updateConsoleAndLogTimer->start(1);
                }
            }
        }

        if(m_bytesInUnprocessedConsoleData > currentSettings->maxCharsInConsole)
        {
            while(m_bytesInUnprocessedConsoleData > currentSettings->maxCharsInConsole)
            {//m_bytesInUnprocessedConsoleData contains too much data.

                int diff = m_bytesInUnprocessedConsoleData - currentSettings->maxCharsInConsole;
                if(diff >= m_unprocessedConsoleData.at(0).data.length())
                {
                    m_bytesInUnprocessedConsoleData -= m_unprocessedConsoleData.first().data.length();
                    m_unprocessedConsoleData.removeFirst();
                }
                else
                {
                    m_unprocessedConsoleData.first().data.remove(0, m_bytesInUnprocessedConsoleData - currentSettings->maxCharsInConsole);
                    m_bytesInUnprocessedConsoleData -= m_bytesInUnprocessedConsoleData - currentSettings->maxCharsInConsole;

                }
            }

            if(m_bytesInStoredConsoleData != 0)
            {
                //Restart the console/log timer.
                m_updateConsoleAndLogTimer->start(1);
            }

            StoredData storedData;
            storedData.type = STORED_DATA_CLEAR_ALL_STANDARD_CONSOLES;
            m_unprocessedConsoleData.push_front(storedData);
        }

        if(!m_updateConsoleAndLogTimer->isActive())
        {
            m_updateConsoleAndLogTimer->start(currentSettings->updateIntervalConsole);
        }
    }
}


/**
 * Caclulates the console data.
 */
void MainWindowHandleData::calculateConsoleData()
{
    const Settings* currentSettings = m_settingsDialog->settings();

    QFont textEditFont("Courier new",  currentSettings->stringConsoleFontSize.toInt());
    QFontMetrics fm(textEditFont);
    m_consoleData.mixedData.pixelsWide = fm.width("0");

    m_consoleData.mixedData.divider = 1;
    m_consoleData.mixedData.bytesPerDecimal = bytesPerDecimalInConsole(currentSettings->consoleDecimalsType);

    if(currentSettings->showBinaryConsole)
    {
        if((currentSettings->consoleDecimalsType == DECIMAL_TYPE_UINT8)|| (currentSettings->consoleDecimalsType == DECIMAL_TYPE_INT8))
        {
            m_consoleData.mixedData.divider = 9;
        }
        else if((currentSettings->consoleDecimalsType == DECIMAL_TYPE_UINT16)|| (currentSettings->consoleDecimalsType == DECIMAL_TYPE_INT16))
        {
            m_consoleData.mixedData.divider = 8.6;
        }
        else
        {
             m_consoleData.mixedData.divider = 8.2;
        }

        m_consoleData.mixedData.onlyOneType = (!currentSettings->showHexInConsole && !currentSettings->showAsciiInConsole && !currentSettings->showDecimalInConsole) ? true : false;
    }
    else if(currentSettings->showDecimalInConsole)
    {
        if(currentSettings->consoleDecimalsType == DECIMAL_TYPE_UINT8)
        {
           m_consoleData.mixedData.divider = 4;
        }
        else if(currentSettings->consoleDecimalsType == DECIMAL_TYPE_INT8)
        {
           m_consoleData.mixedData.divider = 5;
        }
        else if(currentSettings->consoleDecimalsType == DECIMAL_TYPE_INT16)
        {
           m_consoleData.mixedData.divider = (7.0 / 2.0);
        }
        else if(currentSettings->consoleDecimalsType == DECIMAL_TYPE_UINT32)
        {
           m_consoleData.mixedData.divider = (11.0 / 4.0);
        }
        else
        {
            m_consoleData.mixedData.divider = 2;
        }
        m_consoleData.mixedData.onlyOneType = (!currentSettings->showHexInConsole && !currentSettings->showAsciiInConsole) ? true : false;
    }
    else if(currentSettings->showHexInConsole)
    {
        m_consoleData.mixedData.divider = 3;
        m_consoleData.mixedData.onlyOneType = (!currentSettings->showAsciiInConsole) ? true : false;
    }
    else
    {//ascii
        m_consoleData.mixedData.divider = 1;
        m_consoleData.mixedData.onlyOneType = true;
    }

    if(m_consoleData.mixedData.onlyOneType)
    {
        m_userInterface->ReceiveTextEditMixed->setLineWrapMode(QTextEdit::WidgetWidth);
    }
    else
    {
        m_userInterface->ReceiveTextEditMixed->setLineWrapMode(QTextEdit::NoWrap);
    }

    int lineEditWidth = m_userInterface->ReceiveTextEditMixed->width() - m_userInterface->ReceiveTextEditMixed->verticalScrollBar()->width() - 10;
    m_consoleData.mixedData.maxBytePerLine = (int)(((double)lineEditWidth / (double)m_consoleData.mixedData.pixelsWide) /(double) m_consoleData.mixedData.divider);
    if(currentSettings->showDecimalInConsole)
    {
        int tmp = m_consoleData.mixedData.maxBytePerLine % m_consoleData.mixedData.bytesPerDecimal;
        if(tmp != 0)
        {
            m_consoleData.mixedData.maxBytePerLine -= tmp;
        }
    }

    /******calculate the ascii spaces*********/
    m_consoleData.mixedData.asciiSpaces.clear();
    if(m_consoleData.mixedData.onlyOneType)
    {
        m_consoleData.mixedData.asciiSpaces = "";
    }
    else
    {
        qint32 numberOfSpaces = 0;
        if(currentSettings->showDecimalInConsole)
        {
            if(currentSettings->consoleDecimalsType == DECIMAL_TYPE_UINT8){numberOfSpaces = currentSettings->showBinaryConsole ? 7 : 2;}
            else if(currentSettings->consoleDecimalsType == DECIMAL_TYPE_INT8){numberOfSpaces = currentSettings->showBinaryConsole ? 7 : 3;}
            else if(currentSettings->consoleDecimalsType == DECIMAL_TYPE_UINT16){numberOfSpaces = currentSettings->showBinaryConsole ? 14 : 3;}
            else if(currentSettings->consoleDecimalsType == DECIMAL_TYPE_INT16){numberOfSpaces = currentSettings->showBinaryConsole ? 14 : 4;}
            else if(currentSettings->consoleDecimalsType == DECIMAL_TYPE_UINT32){numberOfSpaces = currentSettings->showBinaryConsole ? 28 : 6;}
            else{numberOfSpaces = currentSettings->showBinaryConsole ? 28 : 7;}
        }
        else if(currentSettings->showBinaryConsole)
        {
            numberOfSpaces = 7;
        }
        else
        {//Hex
            numberOfSpaces = 1;
        }

        for(int i = 0; i < numberOfSpaces; i++)
        {
            m_consoleData.mixedData.asciiSpaces += "&nbsp;";
        }
    }

    /******calculate the hex spaces*********/
    m_consoleData.mixedData.hexExtraSpaces.clear();
    m_consoleData.mixedData.hexSpaces.clear();
    if(m_consoleData.mixedData.onlyOneType)
    {
        m_consoleData.mixedData.hexSpaces = "&nbsp;";
    }
    else if(currentSettings->showAsciiInConsole && !currentSettings->showDecimalInConsole && !currentSettings->showBinaryConsole)
    {
        m_consoleData.mixedData.hexSpaces = "";
    }
    else
    {
        qint32 numberOfSpaces = 0;

        if(currentSettings->showDecimalInConsole)
        {
            if(currentSettings->consoleDecimalsType == DECIMAL_TYPE_UINT8){numberOfSpaces = currentSettings->showBinaryConsole ? 6 : 1;}
            else if(currentSettings->consoleDecimalsType == DECIMAL_TYPE_INT8){numberOfSpaces = currentSettings->showBinaryConsole ? 6 : 2;}
            else if(currentSettings->consoleDecimalsType == DECIMAL_TYPE_UINT16){numberOfSpaces = currentSettings->showBinaryConsole ? 12 : 1;}
            else if(currentSettings->consoleDecimalsType == DECIMAL_TYPE_INT16){numberOfSpaces = currentSettings->showBinaryConsole ? 12 : 2;}
            else if(currentSettings->consoleDecimalsType == DECIMAL_TYPE_UINT32){numberOfSpaces = currentSettings->showBinaryConsole ? 24 : 2;}
            else{numberOfSpaces = currentSettings->showBinaryConsole ? 24 : 3;}
        }
        else if(currentSettings->showBinaryConsole)
        {
            numberOfSpaces = 6;
        }

        for(int i = 0; i < numberOfSpaces; i++)
        {
            m_consoleData.mixedData.hexSpaces += "&nbsp;";
        }
    }

    /******calculate the decimal spaces*********/
    m_consoleData.mixedData.decimalSpaces.clear();
    if(m_consoleData.mixedData.onlyOneType)
    {
        m_consoleData.mixedData.decimalSpaces = "&nbsp;";
    }
    else
    {
        qint32 numberOfSpaces = 0;

        if(currentSettings->consoleDecimalsType == DECIMAL_TYPE_UINT8){numberOfSpaces = currentSettings->showBinaryConsole ? 5 : 0;}
        else if(currentSettings->consoleDecimalsType == DECIMAL_TYPE_INT8){numberOfSpaces = currentSettings->showBinaryConsole ? 4 : 0;}
        else if(currentSettings->consoleDecimalsType == DECIMAL_TYPE_UINT16){numberOfSpaces = currentSettings->showBinaryConsole ? 11 : 0;}
        else if(currentSettings->consoleDecimalsType == DECIMAL_TYPE_INT16){numberOfSpaces = currentSettings->showBinaryConsole ? 10 : 0;}
        else if(currentSettings->consoleDecimalsType == DECIMAL_TYPE_UINT32){numberOfSpaces = currentSettings->showBinaryConsole ? 22 : 0;}
        else{numberOfSpaces = currentSettings->showBinaryConsole ? 21 : 0;}

        for(int i = 0; i < numberOfSpaces; i++)
        {
            m_consoleData.mixedData.decimalSpaces += "&nbsp;";
        }
    }


    /******calculate the HTML data*********/

    m_consoleData.htmlMessageAndTimestamp = QString("<span style=\"color:#" + currentSettings->consoleMessageAndTimestampColor + ";\">");
    m_consoleData.htmlReceived = QString("<span style=\"color:#" + currentSettings->consoleReceiveColor + ";\">");
    m_consoleData.htmlSend = QString("<span style=\"color:#" + currentSettings->consoleSendColor + ";\">");
}

/**
 * Creates the string for the mixed console.
 * @param data
 *      The data.
 * @param hasCanMeta
 *      True if data contains CAN metadata.
 * @return
 *      The created string.
 */
QString MainWindowHandleData::createMixedConsoleString(const QByteArray &data, bool hasCanMeta)
{
    QString result;
    QString tmpString;

    const Settings* currentSettings = m_settingsDialog->settings();

    if(m_consoleData.mixedData.onlyOneType)
    {
        if(currentSettings->showDecimalInConsole)result = MainWindow::byteArrayToNumberString(data, false, false, false, true, true, currentSettings->consoleDecimalsType, currentSettings->targetEndianess) + " ";
        if(currentSettings->showHexInConsole)result = MainWindow::byteArrayToNumberString(data, false, true, false) + " ";
        if(currentSettings->showBinaryConsole)result = MainWindow::byteArrayToNumberString(data, true, false, false) + " ";

        if(currentSettings->showAsciiInConsole)
        {
            //Replace the binary 0 (for the ascii console).
            QByteArray asciiArray = data;
            asciiArray.replace(0, 255);

            result = QString::fromLocal8Bit(asciiArray);
            result.replace("<", "&lt;");
            result.replace(">", "&gt;");
            if(!hasCanMeta){result.replace("\n", "<br>");}
            result.replace(" ", "&nbsp;");
        }
    }
    else
    {
        int convertedBytes = 0;

        do
        {
            QByteArray arrayWithMaxBytes = data.mid(convertedBytes, m_consoleData.mixedData.maxBytePerLine);
            convertedBytes += arrayWithMaxBytes.length();

            if(currentSettings->showAsciiInConsole)
            {
                //Replace the binary 0 (for the ascii console).
                QByteArray asciiArray = arrayWithMaxBytes;
                asciiArray.replace(0, 255);
                QString asciiString;

                tmpString = QString::fromLocal8Bit(asciiArray);
                result += "<br>";

                qint32 modulo = m_consoleData.mixedData.bytesPerDecimal;
                ///Create the ascii string.
                for(int i = 0; i < tmpString.length(); i++)
                {
                    if(!currentSettings->showDecimalInConsole || ((i % modulo) == 0))
                    {
                        if(i != 0)
                        {
                            asciiString += "</span>";
                        }
                        asciiString += "&nbsp;";    // uncolored
                        asciiString += QString("<span style=background-color:#%1>").arg(currentSettings->consoleMixedAsciiColor);
                        asciiString += m_consoleData.mixedData.asciiSpaces;
                    }


                    //Replace tags so our span does not get mangled up.
                    if (tmpString[i] == '<')asciiString += "&lt;";
                    else if (tmpString[i] == '>')asciiString += "&gt;";
                    else if (tmpString[i] < 33 || tmpString[i] > 126) asciiString += 255;
                    else asciiString += tmpString[i];
                }
                asciiString += "</span>";

                result += asciiString;
            }

            if(currentSettings->showHexInConsole)
            {
                result += "<br>";

                //Create the hex string.
                tmpString = MainWindow::byteArrayToNumberString(arrayWithMaxBytes, false, true, false);
                QStringList list = tmpString.split(" ");
                qint32 modulo = m_consoleData.mixedData.bytesPerDecimal;
                for(int i = 0; i < list.length(); i++)
                {
                    if(!currentSettings->showDecimalInConsole || ((i % modulo) == 0))
                    {
                        if(i != 0)
                        {
                            result += "</span>";
                        }
                        result += "&nbsp;";    // uncolored
                        result += QString("<span style=background-color:#%1>").arg(currentSettings->consoleMixedHexadecimalColor);
                        result += m_consoleData.mixedData.hexSpaces;
                    }

                    result += list[i];
                }
                result += "</span>";
            }

            if(currentSettings->showDecimalInConsole)
            {
                result += "<br>";

                ///Create the decimal string.
                tmpString = MainWindow::byteArrayToNumberString(arrayWithMaxBytes, false, false, false, true, true, currentSettings->consoleDecimalsType, currentSettings->targetEndianess);

                QStringList list = tmpString.split(" ");
                for(auto el : list)
                {
                    result += "&nbsp;";     // uncolored
                    result += QString("<span style=background-color:#%1>").arg(currentSettings->consoleMixedDecimalColor);
                    result += m_consoleData.mixedData.decimalSpaces;
                    result += el;
                    result += "</span>";
                }
            }
            if(currentSettings->showBinaryConsole)
            {
                result += "<br>";
                ///Create the binary string.
                tmpString = MainWindow::byteArrayToNumberString(arrayWithMaxBytes, true, false, false);
                QStringList list = tmpString.split(" ");
                qint32 modulo = m_consoleData.mixedData.bytesPerDecimal;
                for(int i = 0; i < list.length(); i++)
                {
                    if(!currentSettings->showDecimalInConsole || ((i % modulo) == 0))
                    {
                        if(i != 0)
                        {
                            result += "</span>";
                        }

                        result += "&nbsp;";     // uncolored
                        result += QString("<span style=background-color:#%1>").arg(currentSettings->consoleMixedBinaryColor);
                    }
                    result += list[i];
                }
                result += "</span>";
            }

            result += "<br>";

        }while(convertedBytes < data.length());

    }

    return result;
}

/**
 * Appends data to the console buffers (m_consoleDataBufferAscii, m_consoleDataBufferHex;
 * m_consoleDataBufferDec)
 * @param data
 *      The data.
 * @param isSend
 *      True if the data has been send and false if the data has been received.
 * @param isUserMessage
 *      True if the data is a user message.
 * @param isTimeStamp
 *      True if the data is a timestamp.
 * @param isFromCan
 *      True if the message is from a can interface.
 * @param isNewLine
 *      True if the data is a new line.
 */
void MainWindowHandleData::appendDataToConsoleStrings(QByteArray &data, const Settings* currentSettings, bool isSend, bool isUserMessage,
                                            bool isTimeStamp, bool isFromCan, bool isNewLine)
{
    QString* html = 0;
    QByteArray* dataArray = &data;
    QByteArray canArray;


    QString canInformation;
    if(data.isEmpty()) return;

    if(isNewLine)
    {
        QString tmpString = QString::fromLocal8Bit(data);
        tmpString.replace("\n", "<br>");

        if(currentSettings->showDecimalInConsole)m_consoleDataBufferDec.append(tmpString);
        if(currentSettings->showHexInConsole)m_consoleDataBufferHex.append(tmpString);
        if(currentSettings->showBinaryConsole)m_consoleDataBufferBinary.append(tmpString);
        if(currentSettings->showAsciiInConsole)m_consoleDataBufferAscii.append(tmpString);
    }
    else
    {

        if(isTimeStamp || isUserMessage)
        {
            html = &m_consoleData.htmlMessageAndTimestamp;
        }
        else
        {
            html =  (isSend) ? &m_consoleData.htmlSend : &m_consoleData.htmlReceived;
        }

        if(currentSettings->showAsciiInConsole)m_consoleDataBufferAscii.append(*html);
        if(currentSettings->showDecimalInConsole)m_consoleDataBufferDec.append(*html);
        if(currentSettings->showHexInConsole)m_consoleDataBufferHex.append(*html);
        if(currentSettings->showBinaryConsole)m_consoleDataBufferBinary.append(*html);
        if(currentSettings->showMixedConsole)m_consoleDataBufferMixed.append(*html);

        if(isUserMessage || isTimeStamp)
        {
            QString tmpString = QString::fromLocal8Bit(data);
            tmpString.replace("<", "&lt;");
            tmpString.replace(">", "&gt;");
            tmpString.replace("\n", "<br>");
            tmpString.replace(" ", "&nbsp;");

            if(currentSettings->showDecimalInConsole)m_consoleDataBufferDec.append(tmpString + QString("</span>"));
            if(currentSettings->showHexInConsole)m_consoleDataBufferHex.append(tmpString + QString("</span>"));
            if(currentSettings->showMixedConsole)m_consoleDataBufferMixed.append(tmpString + QString("</span>"));
            if(currentSettings->showBinaryConsole)m_consoleDataBufferBinary.append(tmpString + QString("</span>"));
            if(currentSettings->showAsciiInConsole)m_consoleDataBufferAscii.append(tmpString + QString("</span>"));
        }
        else
        {
            if(isFromCan)
            {
                canArray = QByteArray(data);

                if(currentSettings->showCanMetaInformationInConsole)
                {
                    quint8 type = canArray[0];
                    QString typeString;

                    QByteArray idArray = canArray.mid(PCANBasicClass::BYTES_FOR_CAN_TYPE, PCANBasicClass::BYTES_FOR_CAN_ID);
                    int length = idArray.length();
                    for(int i = length; i < PCANBasicClass::BYTES_FOR_CAN_ID; i++)
                    {
                        idArray.push_front((char)0);
                    }
                    quint32 messageId = ((quint8)idArray[0] << 24)+ ((quint8)idArray[1] << 16) +
                            ((quint8)idArray[2] << 8) + ((quint8)idArray[3] & 0xff);
                    if(type <= PCAN_MESSAGE_RTR){messageId = messageId & 0x7ff;}
                    else{messageId = messageId & 0x1fffffff;}


                    QString messageIdString = QString::number(messageId, 16);
                    QString leadingZeros;
                    for(int i = 0; i < (8 - messageIdString.size()); i++)
                    {
                        leadingZeros += "0";
                    }
                    messageIdString = leadingZeros + messageIdString;

                    if(type == PCAN_MESSAGE_STANDARD){typeString = "std";}
                    else if(type == PCAN_MESSAGE_RTR){typeString = "rtr";}
                    else if(type == PCAN_MESSAGE_EXTENDED){typeString = "ext";}
                    else if(type == (PCAN_MESSAGE_EXTENDED + PCAN_MESSAGE_RTR)){typeString = "ert";}
                    else
                    {
                        typeString = QString("%1").arg(type) + " (valid range is 0-3)";
                    }

                    canInformation = "<br>id: " +  messageIdString + " type: " + typeString + "   ";
                }

                if(isSend){canArray.remove(0, PCANBasicClass::BYTES_METADATA_SEND);}
                else{canArray.remove(0, PCANBasicClass::BYTES_METADATA_RECEIVE);}
                dataArray = &canArray;

            }

            if(currentSettings->showDecimalInConsole)
            {
                QByteArray tmpData;
                QByteArray* usedArray;
                if(!m_decimalConsoleByteBuffer.isEmpty())
                {
                    tmpData = m_decimalConsoleByteBuffer + *dataArray;
                    usedArray = &tmpData;
                }
                else
                {
                    usedArray = dataArray;
                }
                m_consoleDataBufferDec.append(canInformation + MainWindow::byteArrayToNumberString(*usedArray, false, false, false, true, true, currentSettings->consoleDecimalsType, currentSettings->targetEndianess)
                                              + " " + QString("</span>"));
                qint32 tmp = usedArray->length() % m_consoleData.mixedData.bytesPerDecimal;
                if(tmp != 0)
                {
                    m_decimalConsoleByteBuffer = usedArray->right(tmp);
                }
                else
                {
                    m_decimalConsoleByteBuffer.clear();
                }
            }
            if(currentSettings->showHexInConsole)m_consoleDataBufferHex.append(canInformation + MainWindow::byteArrayToNumberString(*dataArray, false, true, false) + " " + QString("</span>"));
            if(currentSettings->showBinaryConsole)m_consoleDataBufferBinary.append(canInformation + MainWindow::byteArrayToNumberString(*dataArray, true, false, false) + " " + QString("</span>"));

            if(currentSettings->showMixedConsole)
            {
                if(!currentSettings->showDecimalInConsole || m_consoleData.mixedData.bytesPerDecimal == 1)
                {
                    m_consoleDataBufferMixed.append(canInformation + createMixedConsoleString(*dataArray, isFromCan && currentSettings->showCanMetaInformationInConsole) + QString("</span>"));
                }
                else
                {
                    QByteArray tmpData = m_mixedConsoleByteBuffer + *dataArray;

                    qint32 tmp = tmpData.length() % m_consoleData.mixedData.bytesPerDecimal;
                    if(tmp != 0)
                    {
                        m_mixedConsoleByteBuffer = tmpData.mid(tmpData.length() - tmp, tmp);
                        tmpData.remove(tmpData.length() - tmp, tmp);
                    }
                    else
                    {
                        m_mixedConsoleByteBuffer.clear();
                    }

                    m_consoleDataBufferMixed.append(canInformation + createMixedConsoleString(tmpData, isFromCan && currentSettings->showCanMetaInformationInConsole) + QString("</span>"));
                }
            }

            if(currentSettings->showAsciiInConsole)
            {

                //Replace the binary 0 (for the ascii console).
                dataArray->replace(0, 255);

                QString tmpString;
                for(auto el : QString::fromLocal8Bit(*dataArray))
                {
                    if (el == '<')tmpString += "&lt;";
                    else if (el == '>')tmpString += "&gt;";
                    else if (el == ' ')tmpString += "&nbsp;";
                    else if (el == '\n')tmpString += "";
                    else if (el == '\r')tmpString += "";
                    else if (el < 33 || el > 126) tmpString += 255;
                    else tmpString += el;
                }

                m_consoleDataBufferAscii.append(canInformation + tmpString + QString("</span>"));
            }
        }

        //Note: data/dataArray is modified during the creation of dataStringAscii (see above), therefore data/dataArray must not be used.
    }
}

/**
 * Appends data the log file.
 * @param data
 *      The data.
 * @param isSend
 *      True if the data has been send and false if the data has been received.
 * @param isUserMessage
 *      True if the data is a user message.
 * @param isTimeStamp
 *      True if the data is a timestamp.
 * @param isFromCan
 *      True if the message is from a can interface
 * @param isNewLine
 *      True if the data is a new line.
 */
void MainWindowHandleData::appendDataToLog(const QByteArray &data, bool isSend, bool isUserMessage, bool isTimeStamp,
                                 bool isFromCan, bool isNewLine)
{

    const Settings* currentSettings = m_settingsDialog->settings();
    const QByteArray* dataArray = &data;
    QByteArray canArray;


    if(data.size() > 0)
    {

        QString dataString;
        QString canInformation;
        if(isFromCan && !isUserMessage && !isTimeStamp && !isNewLine)
        {
            canArray = QByteArray(data);

            if(currentSettings->writeCanMetaInformationInToLog)
            {
                quint8 type = canArray[0];
                QString typeString;

                QByteArray idArray = canArray.mid(PCANBasicClass::BYTES_FOR_CAN_TYPE, PCANBasicClass::BYTES_FOR_CAN_ID);
                int length = idArray.length();
                for(int i = length; i < PCANBasicClass::BYTES_FOR_CAN_ID; i++)
                {
                    idArray.push_front((char)0);
                }
                quint32 messageId = ((quint8)idArray[0] << 24) + ((quint8)idArray[1] << 16) +
                        ((quint8)idArray[2] << 8) + ((quint8)idArray[3] & 0xff);

                if(type <= PCAN_MESSAGE_RTR){messageId = messageId & 0x7ff;}
                else{messageId = messageId & 0x1fffffff;}

                QString messageIdString = QString::number(messageId, 16);

                QString leadingZeros;
                for(int i = 0; i < (8 - messageIdString.size()); i++)
                {
                    leadingZeros += "0";
                }
                messageIdString = leadingZeros + messageIdString;

                if(type == PCAN_MESSAGE_STANDARD){typeString = "std";}
                else if(type == PCAN_MESSAGE_RTR){typeString = "rtr";}
                else if(type == PCAN_MESSAGE_EXTENDED){typeString = "ext";}
                else if(type == (PCAN_MESSAGE_EXTENDED + PCAN_MESSAGE_RTR)){typeString = "ert";}
                else
                {
                    typeString = QString("%1").arg(type) + " (valid range is 0-3)";
                }


                canInformation = "\nid: " +  messageIdString + " type: " + typeString + "   ";
                dataString += canInformation;
            }
            if(isSend){canArray.remove(0, PCANBasicClass::BYTES_METADATA_SEND);}
            else{canArray.remove(0, PCANBasicClass::BYTES_METADATA_RECEIVE);}
            dataArray = &canArray;

        }

        if(isNewLine && !currentSettings->writeAsciiInToLog)
        {
            return;
        }

        if(currentSettings->writeAsciiInToLog || isTimeStamp || isUserMessage || isNewLine)
        {
            if (isNewLine && (currentSettings->writeDecimalInToLog
                              || currentSettings->writeHexInToLog || currentSettings->writeBinaryInToLog))
            {
                return;
            }

            //Replace the binary 0 (for the ascii string).
            QByteArray asciiArray = QByteArray(*dataArray);
            asciiArray.replace(0, 255);

            QString tmp = QString::fromLocal8Bit(asciiArray);

            if(!isUserMessage && !isTimeStamp && !isNewLine)
            {
                tmp.replace("\n", "");
                tmp.replace("\r", "");
            }

            if(currentSettings->writeAsciiInToLog && (currentSettings->writeDecimalInToLog
                                                     || currentSettings->writeHexInToLog || currentSettings->writeBinaryInToLog))
            {
                if(!isFromCan && !isTimeStamp && !isUserMessage && !isNewLine){dataString = "\n";}

            }

            dataString += tmp;

        }

        if(!isUserMessage && !isTimeStamp && !isNewLine)
        {
            if(currentSettings->writeDecimalInToLog)

            {
                if(!isFromCan){dataString.append("\n");}
                dataString.append(MainWindow::byteArrayToNumberString(*dataArray, false, false,
                                                          (currentSettings->writeAsciiInToLog || currentSettings->writeHexInToLog || currentSettings->writeBinaryInToLog),
                                                           true, true, currentSettings->logDecimalsType, currentSettings->targetEndianess));
            }
            if(currentSettings->writeHexInToLog)
            {
                if(!isFromCan){dataString.append("\n");}
                dataString.append(MainWindow::byteArrayToNumberString(*dataArray, false, true,
                                                          (currentSettings->writeAsciiInToLog || currentSettings->writeDecimalInToLog || currentSettings->writeBinaryInToLog)));
            }
            if(currentSettings->writeBinaryInToLog)
            {
                if(!isFromCan){dataString.append("\n");}
                dataString.append(MainWindow::byteArrayToNumberString(*dataArray, true, false,
                                                          (currentSettings->writeAsciiInToLog || currentSettings->writeDecimalInToLog || currentSettings->writeHexInToLog)));
            }
        }

        if(dataString.size() > 0)
        {

            if(currentSettings->htmlLogFile && !m_htmlLogFile.isOpen() && (currentSettings->htmlLogfileName != ""))
            {
                m_mainWindow->textLogActivatedSlot(true);

            }
            if(currentSettings->textLogFile && !m_textLogFile.isOpen() && (currentSettings->textLogfileName != ""))
            {
                m_mainWindow->htmLogActivatedSlot(true);
            }

            if(currentSettings->textLogFile && m_textLogFile.isOpen())
            {
                m_textLogFileStream << dataString;
            }

            if(currentSettings->htmlLogFile && m_htmlLogFile.isOpen())
            {
                QString color;
                if(isTimeStamp)
                {
                    color = LOG_TIMESTAMP_AND_MESSAGE_COLOR;
                }
                else if(isUserMessage)
                {
                    color = LOG_TIMESTAMP_AND_MESSAGE_COLOR;
                }
                else
                {
                    if(isSend)
                    {
                        color = LOG_SEND_COLOR;
                    }
                    else
                    {
                        color = LOG_RECEIVE_COLOR;
                    }
                }

                QString tmpString;
                for(auto el : dataString)
                {
                    if (el == '<')tmpString += "&lt;";
                    else if (el == '>')tmpString += "&gt;";
                    else if (el == ' ')tmpString += "&nbsp;";
                    else if (el == '\n')tmpString += "<br>";
                    else tmpString += el;
                }
                m_HtmlLogFileStream << QString("<span style=\"  font-family:'") + currentSettings->stringHtmlLogFont +
                                       QString("'; font-size:") + currentSettings->stringHtmlLogFontSize + QString("pt; color:" + color + ";\">")
                                       + tmpString +  QString("</span>");

            }//if(currentSettings->htmlLogFile && m_htmlLogFile.isOpen())

        }//if(dataString.size() > 0)

    }//if(data.size() > 0)

}

/**
 * Appends the queued received data to the stored data.
 */
void MainWindowHandleData::queuedDataReceivedSlot(void)
{
    m_queuedReceivedDataTimer.stop();
    if(!m_queuedReceivedData.isEmpty())
    {
        m_receivedBytes += m_queuedReceivedData.size();
        appendDataToStoredData(m_queuedReceivedData, false, false, m_mainWindow->m_isConnectedWithCan, false);
        m_queuedReceivedData.clear();
    }

}

/**
 * The slot is called if the main interface thread has received can messages.
 * This slot is connected to the MainInterfaceThread::canMessageReceivedSignal signal.
 * @param data
 *      The received data.
 */
void MainWindowHandleData::dataReceivedSlot(QByteArray data)
{
    m_queuedReceivedData.append(data);
    if(!m_queuedReceivedDataTimer.isActive())
    {
        m_queuedReceivedDataTimer.start(1);
    }
}

/**
 * The slot is called if the main interface thread has received CAN messages..
 * This slot is connected to the MainInterfaceThread::dataReceivedSignal signal.
 * @param messages
 *      The received messages.
 */
void MainWindowHandleData::canMessagesReceivedSlot(QVector<QByteArray> messages)
{
    for(auto el : messages)
    {
        m_receivedBytes += el.size();
        m_receivedBytes -= PCANBasicClass::BYTES_METADATA_RECEIVE;

        appendDataToStoredData(el, false, false, m_mainWindow->m_isConnectedWithCan, false);
        m_mainWindow->m_canTab->canMessageReceived(el);
    }
}

/**
 * The slot is called if the main interface thread has send data.
 * This slot is connected to the MainInterfaceThread::sendingFinishedSignal signal.
 * @param data
 *      The send data
 * @param success
 *      True for success.
 * @param id
 *      The send id.
 */
void MainWindowHandleData::dataHasBeenSendSlot(QByteArray data, bool success, uint id)
{
    (void) id;
    if(success)
    {
        if(!m_queuedReceivedData.isEmpty())
        {
            queuedDataReceivedSlot();
        }

        m_sentBytes += data.size();

        if(m_mainWindow->m_isConnectedWithCan)
        {
            m_sentBytes -= PCANBasicClass::BYTES_METADATA_SEND;

            QByteArray tmpIdAndType = data.mid(0, PCANBasicClass::BYTES_METADATA_SEND);
            for(int i = PCANBasicClass::BYTES_METADATA_SEND; i < data.length(); i += PCANBasicClass::MAX_BYTES_PER_MESSAGE)
            {
                QByteArray tmpArray = tmpIdAndType + data.mid(i, PCANBasicClass::MAX_BYTES_PER_MESSAGE);

                appendDataToStoredData(tmpArray, true, false, m_mainWindow->m_isConnectedWithCan, false);
                m_mainWindow->m_canTab->canMessageTransmitted(tmpArray);
            }
        }
        else
        {
            appendDataToStoredData(data, true, false, m_mainWindow->m_isConnectedWithCan, false);
        }
    }

    if(id == MainInterfaceThread::SEND_ID_HISTOTRY)
    {
        if(success)
        {
            if(!m_sendHistorySendData.isEmpty())
            {
                m_sendHistoryTimer.start(m_mainWindow->m_userInterface->sendPauseSpinBox->value());
            }
            else
            {
                m_historySendIsInProgress = false;
                enableHistoryGuiElements(true);
                historyConsoleTimerSlot();
            }
            m_mainWindow->m_userInterface->progressBar->setValue(m_mainWindow->m_userInterface->progressBar->value() + 1);
        }
        else
        {
            m_historySendIsInProgress = false;
            enableHistoryGuiElements(true);
            historyConsoleTimerSlot();
        }
    }
}

/**
 * Clears all stored data.
 */
void MainWindowHandleData::clear(void)
{
    m_consoleDataBufferAscii.clear();
    m_consoleDataBufferHex.clear();
    m_consoleDataBufferDec.clear();
    m_consoleDataBufferMixed.clear();;
    m_customConsoleStoredStrings.clear();
    m_numberOfBytesInCustomConsoleStoredStrings = 0;
    m_customConsoleStrings.clear();
    m_numberOfBytesInCustomConsoleStrings = 0;
    m_consoleDataBufferBinary.clear();
    m_unprocessedConsoleData.clear();
    m_bytesInUnprocessedConsoleData = 0;
    m_storedConsoleData.clear();
    m_bytesInStoredConsoleData = 0;
    m_bytesSinceLastNewLineInConsole = 0;
    m_bytesSinceLastNewLineInLog = 0;
    m_receivedBytes = 0;
    m_sentBytes = 0;
    m_decimalConsoleByteBuffer.clear();
    m_mixedConsoleByteBuffer.clear();
}
/**
 * Append a time stamp to a stored data vector.
 * @param storedDataVector
 *      The stored data vector.
 * @param isSend
 *      True if the time stamp results from a send message
 * @param isUserMessage
 *      True if the time stamp results from a user message.
 * @param isFromCan
 *      True if the time stamp results from a CAN message
 * @param timeStampFormat
 *      The time stamp format.
 */
void MainWindowHandleData::appendTimestamp(QVector<StoredData>* storedDataVector, bool isSend, bool isUserMessage,
                                 bool isFromCan, QString timeStampFormat)
{
    StoredData storedData;
    storedData.isFromCan = isFromCan;
    storedData.data = QDateTime::currentDateTime().toString(timeStampFormat).toLocal8Bit();
    storedData.type = isUserMessage ? STORED_DATA_TYPE_USER_MESSAGE : STORED_DATA_TYPE_TIMESTAMP;
    storedData.isSend = isSend;
    storedDataVector->push_back(storedData);
}

/**
 * Append a new line to a stored data vector.
 * @param storedDataVector
 *      The stored data vector.
 * @param isSend
 *      True if the time stamp results from a send message
 * @param isUserMessage
 *      True if the time stamp results from a user message.
 * @param isFromCan
 *      True if the time stamp results from a CAN message.
 */
void MainWindowHandleData::appendNewLine(QVector<StoredData>* storedDataVector, bool isSend, bool isFromCan)
{
    StoredData storedData;
    storedData.isFromCan = isFromCan;
    storedData.data = QString("\n").toLocal8Bit();
    storedData.type = STORED_DATA_TYPE_NEW_LINE;
    storedData.isSend = isSend;
    storedDataVector->push_back(storedData);
}


/**
 * Appends data to the unprocessed console data.
 * @param data
 *      The data.
 * @param isSend
 *      True if the data has been sent.
 * @param isUserMessage
 *      True if the data is a user message.
 * @param isFromCan
 *      True if the data is from CAN.
 * @param forceTimeStamp
 *      True if a time stamp shall be generated (independently from the time stamp settings)
 * @param isRecursivCall
 *      True if the function is called recursively.
 */
void MainWindowHandleData::appendUnprocessConsoleData(QByteArray &data, bool isSend, bool isUserMessage,
                                            bool isFromCan, bool forceTimeStamp, bool isRecursivCall)
{
    static QDateTime lastConsoleTimeInBuffer = QDateTime::currentDateTime().addSecs(-1000);
    static QDateTime lastSendReceivedDataInConsole = QDateTime::currentDateTime().addYears(1);

    const Settings* currentSettings = m_settingsDialog->settings();
    static bool createTimeStampOnNextCall = false;

    if(!isRecursivCall)
    {
        if(createTimeStampOnNextCall)
        {
            //Enter a console time stamp.
            appendTimestamp(&m_unprocessedConsoleData, isSend, isUserMessage, isFromCan, currentSettings->consoleTimestampFormat);
            m_bytesInUnprocessedConsoleData += m_unprocessedConsoleData.last().data.length();
            createTimeStampOnNextCall = false;
        }
        if(currentSettings->consoleCreateTimestampAt && (data.indexOf((quint8)currentSettings->consoleTimestampAt) != -1) && !isFromCan && !isUserMessage)
        {//Data contains a time stamp at byte.

            /*************************create time stamps for all time stamp at bytes (currentSettings->consoleTimestampAt)**********************/
            QList<QByteArray> splittedData;
            splittedData = data.split((quint8)currentSettings->consoleTimestampAt);

            for(qint32 i = 0; i < splittedData.length(); i++)
            {
                if(i < (splittedData.length() - 1))
                {//The last entry in splittedData is not reached.

                    if(!splittedData[i].isEmpty())
                    {
                        //Append a time stamp byte (was removed during split)
                        appendUnprocessConsoleData(splittedData[i].append((quint8)currentSettings->consoleTimestampAt), isSend, isUserMessage, isFromCan,forceTimeStamp, true);
                    }

                    if((i + 2) == splittedData.length())
                    {//The next to last element has been reached.

                        if(splittedData[i + 1].length() == 0)
                        {//The last element is empty.
                            createTimeStampOnNextCall = true;
                            return;
                        }
                    }

                    //Enter a console time stamp.
                    appendTimestamp(&m_unprocessedConsoleData, isSend, isUserMessage, isFromCan, currentSettings->consoleTimestampFormat);
                    m_bytesInUnprocessedConsoleData += m_unprocessedConsoleData.last().data.length();
                }
                else
                {
                    if(!splittedData[i].isEmpty())
                    {
                        appendUnprocessConsoleData(splittedData[i], isSend, isUserMessage, isFromCan,forceTimeStamp, true);
                    }
                }

            }//for(qint32 i = 0; i < splittedData.length(); i++)

            return;
        }

        if(forceTimeStamp || (currentSettings->generateTimeStampsInConsole && (lastConsoleTimeInBuffer.msecsTo(QDateTime::currentDateTime()) > (qint64)currentSettings->timeStampIntervalConsole)))
        {
            appendTimestamp(&m_unprocessedConsoleData, isSend, isUserMessage, isFromCan, currentSettings->consoleTimestampFormat);
            m_bytesInUnprocessedConsoleData += m_unprocessedConsoleData.last().data.length();
            lastConsoleTimeInBuffer = QDateTime::currentDateTime();

        }

        if((currentSettings->consoleNewLineAt != 0xffff) && (data.indexOf((quint8)currentSettings->consoleNewLineAt) != -1) && !isFromCan && !isUserMessage)
        {//Data contains a new line byte.

            /*************************create new line entries for all new line bytes (currentSettings->consoleNewLineAt)**********************/
            QList<QByteArray> splittedData;
            splittedData = data.split((quint8)currentSettings->consoleNewLineAt);

            for(qint32 i = 0; i < splittedData.length(); i++)
            {

                appendUnprocessConsoleData(splittedData[i], isSend, isUserMessage, isFromCan, isUserMessage, true);

                if(i < (splittedData.length() - 1))
                {//The last entry in splittedData is not reached.

                    //Append a new line byte (was removed during split)
                    m_unprocessedConsoleData.last().data.append((quint8)currentSettings->consoleNewLineAt);
                    m_bytesInUnprocessedConsoleData ++;

                    //Append a new line.
                    appendNewLine(&m_unprocessedConsoleData, isSend, isFromCan);
                    m_bytesInUnprocessedConsoleData += m_unprocessedConsoleData.last().data.length();
                }

            }

            return;
        }

        if(currentSettings->consoleNewLineAfterPause != 0)
        {
            if((lastSendReceivedDataInConsole.msecsTo(QDateTime::currentDateTime()) > (qint64)currentSettings->consoleNewLineAfterPause))
            {
                //Append a new line.
                appendNewLine(&m_unprocessedConsoleData, isSend, isFromCan);
                m_bytesInUnprocessedConsoleData += m_unprocessedConsoleData.last().data.length();
            }

            lastSendReceivedDataInConsole = QDateTime::currentDateTime();
        }
    }// if(!isRecursivCall)


    StoredDataType lastEntryTypeInStoredData = (m_unprocessedConsoleData.length() == 0) ? STORED_DATA_TYPE_INVALID : m_unprocessedConsoleData.last().type;
    StoredDataType newEntryType;

    if(isUserMessage)
    {
        newEntryType = STORED_DATA_TYPE_USER_MESSAGE;
    }
    else
    {
        newEntryType = isSend ? STORED_DATA_TYPE_SEND : STORED_DATA_TYPE_RECEIVE;
    }


    if((lastEntryTypeInStoredData != newEntryType) || isFromCan)
    {
        StoredData newStoredDataEntry;
        newStoredDataEntry.data = data;
        newStoredDataEntry.isFromCan = isFromCan;
        newStoredDataEntry.type = newEntryType;
        newStoredDataEntry.isSend = isSend;
        m_unprocessedConsoleData.push_back(newStoredDataEntry);
    }
    else
    {
        m_unprocessedConsoleData.last().data.append(data);
    }

    m_bytesInUnprocessedConsoleData += data.length();

}


/**
 * Appends data to the unprocessed log data.
 * @param data
 *      The data.
 * @param isSend
 *      True if the data has been sent.
 * @param isUserMessage
 *      True if the data is a user message.
 * @param isFromCan
 *      True if the data is from CAN.
 * @param forceTimeStamp
 *      True if a time stamp shall be generated (independently from the time stamp settings)
 * @param isRecursivCall
 *      True if the function is called recursively.
 */
void MainWindowHandleData::appendUnprocessLogData(const QByteArray &data, bool isSend, bool isUserMessage,
                                        bool isFromCan, bool forceTimeStamp, bool isRecursivCall)
{
    static QDateTime lastLogTimeInBuffer = QDateTime::currentDateTime().addSecs(-1000);
    static QDateTime lastSendReceivedDataInLog = QDateTime::currentDateTime().addYears(1);

    const Settings* currentSettings = m_settingsDialog->settings();
    static bool createTimeStampOnNextCall = false;

    if(!isRecursivCall)
    {
        if(createTimeStampOnNextCall)
        {
            //Enter a log time stamp.
            appendTimestamp(&m_unprocessedLogData, isSend, isUserMessage, isFromCan, currentSettings->logTimestampFormat);
            createTimeStampOnNextCall = false;
        }

        if(currentSettings->logCreateTimestampAt && (data.indexOf((quint8)currentSettings->logTimestampAt) != -1) && !isFromCan && !isUserMessage)
        {//Data contains a time stamp at byte.

            /*************************create time stamps for all time stamp at bytes (currentSettings->logTimestampAt)**********************/
            QList<QByteArray> splittedData;
            splittedData = data.split((quint8)currentSettings->logTimestampAt);

            for(qint32 i = 0; i < splittedData.length(); i++)
            {
                if(i < (splittedData.length() - 1))
                {//The last entry in splittedData is not reached.

                    if(!splittedData[i].isEmpty())
                    {
                        //Append a time stamp byte (was removed during split)
                        appendUnprocessLogData(splittedData[i].append((quint8)currentSettings->logTimestampAt), isSend, isUserMessage, isFromCan,forceTimeStamp, true);
                    }

                    if((i + 2) == splittedData.length())
                    {//The next to last element has been reached.

                        if(splittedData[i + 1].length() == 0)
                        {//The last element is empty.
                            createTimeStampOnNextCall = true;
                            return;
                        }
                    }

                    //Enter a log time stamp.
                    appendTimestamp(&m_unprocessedLogData, isSend, isUserMessage, isFromCan, currentSettings->logTimestampFormat);
                }
                else
                {
                    appendUnprocessLogData(splittedData[i], isSend, isUserMessage, isFromCan,forceTimeStamp, true);
                }
            }//for(qint32 i = 0; i < splittedData.length(); i++)

            return;
        }

        if(forceTimeStamp || (currentSettings->generateTimeStampsInLog && (lastLogTimeInBuffer.msecsTo(QDateTime::currentDateTime()) > (qint64)currentSettings->timeStampIntervalLog)))
        {
            appendTimestamp(&m_unprocessedLogData, isSend, isUserMessage, isFromCan, currentSettings->logTimestampFormat);
            lastLogTimeInBuffer = QDateTime::currentDateTime();

        }


        if((currentSettings->logNewLineAt != 0xffff) && (data.indexOf((quint8)currentSettings->logNewLineAt) != -1) && !isFromCan && !isUserMessage)
        {//Data contains a new line byte.

            /*************************create new line entries for all new line bytes (currentSettings->logNewLineAt)**********************/
            QList<QByteArray> splittedData;
            splittedData = data.split((quint8)currentSettings->logNewLineAt);

            for(qint32 i = 0; i < splittedData.length(); i++)
            {
                appendUnprocessLogData(splittedData[i], isSend, isUserMessage, isFromCan, forceTimeStamp, true);

                if(i < (splittedData.length() - 1))
                {//The last entry in splittedData is not reached.

                    //Append a new line byte (was removed during split)
                    m_unprocessedLogData.last().data.append((quint8)currentSettings->logNewLineAt);

                    //Append a new line.
                    appendNewLine(&m_unprocessedLogData, isSend, isFromCan);
                }

            }

            return;
        }
        if(currentSettings->logNewLineAfterPause != 0)
        {
            if(lastSendReceivedDataInLog.msecsTo(QDateTime::currentDateTime()) > (qint64)currentSettings->logNewLineAfterPause)
            {
                //Append a new line.
                appendNewLine(&m_unprocessedLogData, isSend, isFromCan);

            }

            lastSendReceivedDataInLog = QDateTime::currentDateTime();
        }
    }//if(!isRecursivCall)

    StoredDataType lastEntryTypeInStoredData = (m_unprocessedLogData.length() == 0) ? STORED_DATA_TYPE_INVALID : m_unprocessedLogData.last().type;
    StoredDataType newEntryType;

    if(isUserMessage)
    {
        newEntryType = STORED_DATA_TYPE_USER_MESSAGE;
    }
    else
    {
        newEntryType = isSend ? STORED_DATA_TYPE_SEND : STORED_DATA_TYPE_RECEIVE;
    }


    if((lastEntryTypeInStoredData != newEntryType) || isFromCan)
    {
        StoredData newStoredDataEntry;
        newStoredDataEntry.data = data;
        newStoredDataEntry.isFromCan = isFromCan;
        newStoredDataEntry.type = newEntryType;
        newStoredDataEntry.isSend = isSend;
        m_unprocessedLogData.push_back(newStoredDataEntry);
    }
    else
    {
        m_unprocessedLogData.last().data.append(data);
    }

}

/**
 * Processes the data in m_unprocessedConsoleData (creates the log and the console strings).
 *
 * Note: m_unprocessedConsoleData is cleared in this function.
 */
void MainWindowHandleData::processDataInStoredData()
{
    const Settings* settings = m_settingsDialog->settings();

    for(auto el : m_unprocessedConsoleData)
    {
        if(el.type == STORED_DATA_CLEAR_ALL_STANDARD_CONSOLES)
        {
            m_userInterface->ReceiveTextEditAscii->clear();
            m_consoleDataBufferAscii.clear();
            m_userInterface->ReceiveTextEditHex->clear();
            m_consoleDataBufferHex.clear();
            m_userInterface->ReceiveTextEditDecimal->clear();
            m_consoleDataBufferDec.clear();
            m_userInterface->ReceiveTextEditMixed->clear();
            m_consoleDataBufferMixed.clear();
            m_userInterface->ReceiveTextEditBinary->clear();
            m_consoleDataBufferBinary.clear();

            m_storedConsoleData.clear();
            m_bytesInStoredConsoleData = 0;
            m_decimalConsoleByteBuffer.clear();
            m_mixedConsoleByteBuffer.clear();
        }
        else
           {
            bool isFromAddMessageDialog = (el.type == STORED_DATA_TYPE_USER_MESSAGE) ? true : false;
            bool isTimeStamp = (el.type == STORED_DATA_TYPE_TIMESTAMP) ? true : false;
            bool isNewLine = (el.type == STORED_DATA_TYPE_NEW_LINE) ? true : false;

            if((settings->consoleNewLineAfterBytes != 0) && !isTimeStamp && !isNewLine)
            {//New line after x bytes is activated.

                QByteArray array = el.data;
                StoredData storedData;
                storedData.isFromCan = el.isFromCan;
                storedData.isSend = el.isSend;

                if((array.length() + m_bytesSinceLastNewLineInConsole) >= settings->consoleNewLineAfterBytes)
                {
                    do
                    {
                        QByteArray tmpArray = array.left(settings->consoleNewLineAfterBytes - m_bytesSinceLastNewLineInConsole);
                        //Save the console data bevore calling appendDataToConsoleStrings (0 are replace by 0xff in this function).
                        storedData.data = tmpArray;
                        storedData.type = el.type;
                        m_storedConsoleData.push_back(storedData);
                        m_bytesInStoredConsoleData += storedData.data.size();

                        appendDataToConsoleStrings(tmpArray, settings, el.isSend , isFromAddMessageDialog, isTimeStamp, el.isFromCan, isNewLine);
                        array.remove(0, settings->consoleNewLineAfterBytes - m_bytesSinceLastNewLineInConsole);

                        tmpArray = QString("\n").toLocal8Bit();
                        //Save the console data before calling appendDataToConsoleStrings (0 are replace by 0xff in this function).
                        storedData.data = tmpArray;
                        storedData.type = STORED_DATA_TYPE_NEW_LINE;
                        m_storedConsoleData.push_back(storedData);
                        m_bytesInStoredConsoleData += storedData.data.size();

                        appendDataToConsoleStrings(tmpArray, settings, el.isSend , isFromAddMessageDialog, isTimeStamp, el.isFromCan, true);

                        m_bytesSinceLastNewLineInConsole = 0;

                    }while(array.length() >= (qint32)settings->consoleNewLineAfterBytes);

                }

                if(!array.isEmpty())
                {
                    //Save the console data bevore calling appendDataToConsoleStrings (0 are replace by 0xff in this function).
                    storedData.data = array;
                    storedData.type = el.type;
                    m_storedConsoleData.push_back(storedData);
                    m_bytesInStoredConsoleData += storedData.data.size();

                    appendDataToConsoleStrings(array, settings, el.isSend , isFromAddMessageDialog, isTimeStamp, el.isFromCan, isNewLine);
                    m_bytesSinceLastNewLineInConsole += array.length();
                }

            }
            else
            {//New line after x bytes is not activated.

                //Save the console data bevore calling appendDataToConsoleStrings (0 are replace by 0xff in this function).
                m_storedConsoleData.push_back(el);
                m_bytesInStoredConsoleData += el.data.size();

                appendDataToConsoleStrings(el.data, settings, el.isSend , isFromAddMessageDialog, isTimeStamp, el.isFromCan, isNewLine);
            }

        }

    }//for(auto el : m_unprocessedConsoleData)


    for(auto el : m_unprocessedLogData)
    {
        if(settings->htmlLogFile || settings->textLogFile)
        {
            bool isTimeStamp = (el.type == STORED_DATA_TYPE_TIMESTAMP) ? true : false;
            bool isNewLine = (el.type == STORED_DATA_TYPE_NEW_LINE) ? true : false;
            bool isFromAddMessageDialog = (el.type == STORED_DATA_TYPE_USER_MESSAGE) ? true : false;

            QByteArray* array;
            QByteArray mixedArray;

            if(settings->writeDecimalInToLog && (bytesPerDecimalInConsole(settings->logDecimalsType) != 1))
            {
                mixedArray = m_decimalLogByteBuffer + el.data;

                qint32 tmp = mixedArray.length() % m_consoleData.mixedData.bytesPerDecimal;
                if(tmp != 0)
                {
                    m_decimalLogByteBuffer = mixedArray.mid(mixedArray.length() - tmp, tmp);
                    mixedArray.remove(mixedArray.length() - tmp, tmp);
                }
                else
                {
                    m_decimalLogByteBuffer.clear();
                }
                array = &mixedArray;
            }
            else
            {
                array = &el.data;
            }

            if((settings->logNewLineAfterBytes != 0) && !isTimeStamp && !isNewLine)
            {//New line after x bytes is activated.

                if((array->length() + m_bytesSinceLastNewLineInLog) >= settings->logNewLineAfterBytes)
                {
                    do
                    {
                        appendDataToLog(array->left(settings->logNewLineAfterBytes - m_bytesSinceLastNewLineInLog),
                                        el.isSend , isFromAddMessageDialog, isTimeStamp, el.isFromCan, isNewLine);
                        array->remove(0, settings->logNewLineAfterBytes - m_bytesSinceLastNewLineInLog);

                        QByteArray tmpArray = QString("\n").toLocal8Bit();
                        appendDataToLog(tmpArray, el.isSend , isFromAddMessageDialog, isTimeStamp, el.isFromCan, true);

                        m_bytesSinceLastNewLineInLog = 0;

                    }while(array->length() >= (qint32)settings->logNewLineAfterBytes);

                }

                if(!array->isEmpty())
                {
                    appendDataToLog(*array, el.isSend , isFromAddMessageDialog, isTimeStamp, el.isFromCan, isNewLine);
                    m_bytesSinceLastNewLineInLog += array->length();
                }

            }
            else
            {//New line after x bytes is not activated.
                appendDataToLog(*array, el.isSend, isFromAddMessageDialog, isTimeStamp, el.isFromCan, isNewLine);
            }
        }
    }

    m_unprocessedConsoleData.clear();
    m_bytesInUnprocessedConsoleData = 0;
    m_unprocessedLogData.clear();

    //Flush the log files.
    if(settings->htmlLogFile && m_htmlLogFile.isOpen())
    {
        m_HtmlLogFileStream.flush();
    }
    if(settings->textLogFile && m_textLogFile.isOpen())
    {
        m_textLogFileStream.flush();
    }
}

/**
 * Reinserts the data into the mixed consoles.
 */
void MainWindowHandleData::reInsertDataInMixecConsoleSlot(void)
{
    Settings settings = *m_settingsDialog->settings();

    if(settings.showMixedConsole)
    {
        QMessageBox box(QMessageBox::Information, "ScriptCommunicator", "Recalculating console data",
                        QMessageBox::NoButton, m_mainWindow);
        box.setStandardButtons(QMessageBox::NoButton);

        if(m_bytesInStoredConsoleData > 2500)
        {
            QApplication::setActiveWindow(&box);
            box.setModal(false);
            box.show();

            QApplication::processEvents();
        }

        m_mainWindow->m_resizeTimer.stop();

        int pos = 0;
        pos = m_userInterface->ReceiveTextEditMixed->verticalScrollBar()->value();

        m_userInterface->ReceiveTextEditMixed->clear();
        m_mixedConsoleByteBuffer.clear();
        calculateConsoleData();

        //Only the mixed console data shall be generated.
        settings.showAsciiInConsole = false;
        if(m_consoleData.mixedData.bytesPerDecimal == 1) settings.showDecimalInConsole = false;
        settings.showHexInConsole = false;
        settings.showBinaryConsole = false;

        for(auto el : m_storedConsoleData)
        {
            bool isFromAddMessageDialog = (el.type == STORED_DATA_TYPE_USER_MESSAGE) ? true : false;
            bool isTimeStamp = (el.type == STORED_DATA_TYPE_TIMESTAMP) ? true : false;
            bool isNewLine = (el.type == STORED_DATA_TYPE_NEW_LINE) ? true : false;

            appendDataToConsoleStrings(el.data, &settings, el.isSend , isFromAddMessageDialog, isTimeStamp, el.isFromCan, isNewLine);
        }

        m_consoleDataBufferAscii.clear();
        m_consoleDataBufferHex.clear();
        m_consoleDataBufferDec.clear();
        m_consoleDataBufferBinary.clear();

        m_mainWindow->appendConsoleStringToConsole(&m_consoleDataBufferMixed, m_userInterface->ReceiveTextEditMixed);
        m_userInterface->ReceiveTextEditMixed->verticalScrollBar()->setValue(pos);

        box.close();
    }
}

/**
 * Adds data to the send history.
 * @param data
 *      The data.
 */
void MainWindowHandleData::addDataToSendHistory(const QByteArray* data)
{
    const Settings* currentSettings = m_settingsDialog->settings();

    m_sendHistory.push_front(*data);

    if(m_sendHistory.size() > MAX_SEND_HISTORY_ENTRIES)
    {
        m_sendHistory.pop_back();
    }

    m_mainWindow->m_userInterface->startIndexSpinBox->setMaximum(m_sendHistory.size() - 1);
    m_mainWindow->m_userInterface->endIndexSpinBox->setMaximum(m_sendHistory.size() - 1);

    if(!m_historyConsoleTimer.isActive() && !m_historySendIsInProgress)
    {
        m_historyConsoleTimer.start(currentSettings->updateIntervalConsole);
    }
}

/**
 * The send history timer slot.
 */
void MainWindowHandleData::sendHistoryTimerSlot()
{
    m_sendHistoryTimer.stop();

    if(m_mainWindow->m_isConnected)
    {
        emit sendDataWithTheMainInterfaceSignal(m_sendHistorySendData.first(), MainInterfaceThread::SEND_ID_HISTOTRY);
        m_sendHistorySendData.pop_front();
    }
    else
    {
        m_historySendIsInProgress = false;
        enableHistoryGuiElements(true);
        historyConsoleTimerSlot();
        QMessageBox::critical(m_mainWindow, "error", "sending failed: no connection");
    }
}

/**
 * Slotfunction for m_checkDebugWindowsIsClosed.
 */
void MainWindowHandleData::checkDebugWindowsIsClosedSlot()
{

    Settings settings = *m_settingsDialog->settings();

    if(m_customConsoleObject->getRunsInDebuggerAndDebugWindowIsClosed() && settings.consoleShowCustomConsole)
    {
        settings.consoleDebugCustomConsole = false;
        settings.consoleShowCustomConsole = false;
        m_settingsDialog->setAllSettingsSlot(settings, false);
        m_mainWindow->customConsoleSettingsChangedSlot();
        m_mainWindow->inititializeTab();
    }

    if(m_customLogObject->getRunsInDebuggerAndDebugWindowIsClosed() && settings.logDebugCustomLog)
    {
        settings.logDebugCustomLog  = false;
        settings.logGenerateCustomLog = false;
        m_settingsDialog->setAllSettingsSlot(settings, false);
        m_mainWindow->customLogSettingsChangedSlot();
        m_customLogFile.close();
    }

}

/**
 * Enables/disables the send history GUI elements.
 * @param enable
 */
void MainWindowHandleData::enableHistoryGuiElements(bool enable)
{
    m_mainWindow->m_userInterface->endIndexSpinBox->setEnabled(enable);
    m_mainWindow->m_userInterface->startIndexSpinBox->setEnabled(enable);
    m_mainWindow->m_userInterface->historyFormatComboBox->setEnabled(enable);
    m_mainWindow->m_userInterface->sendPauseSpinBox->setEnabled(enable);
    m_mainWindow->m_userInterface->clearHistoryPushButton->setEnabled(enable);
    m_mainWindow->m_userInterface->createScriptPushButton->setEnabled(enable);

    if(enable)
    {
        m_mainWindow->m_userInterface->sendHistoryPushButton->setText("send");
        m_mainWindow->m_userInterface->sendHistoryPushButton->setToolTip("send the selection");
    }
    else
    {
        m_mainWindow->m_userInterface->sendHistoryPushButton->setText("cancel");
        m_mainWindow->m_userInterface->sendHistoryPushButton->setToolTip("");
    }
}

/**
 * Cancel send the history.
 */
void MainWindowHandleData::cancelSendHistory(void)
{
    m_historySendIsInProgress = false;
    m_sendHistoryTimer.stop();
    m_sendHistorySendData.clear();
    enableHistoryGuiElements(true);
    historyConsoleTimerSlot();
}

/**
 * Send the history.
 */
void MainWindowHandleData::sendHistory(void)
{
    qint32 endIndex = m_mainWindow->m_userInterface->endIndexSpinBox->value();
    qint32 startIndex = m_mainWindow->m_userInterface->startIndexSpinBox->value();
    qint32 repetitionCount = m_mainWindow->m_userInterface->sendRepetitionCountSpinBox->value() + 1U;
    qint32 count = 0;
    m_sendHistorySendData.clear();

    for(qint32 i = 0; i < repetitionCount; i++)
    {
        if(startIndex < endIndex)
        {
            for(qint32 i = startIndex; (i <= endIndex) && (i < m_sendHistory.length()); i++)
            {
                m_sendHistorySendData.push_back(m_sendHistory[i]);
            }
            count += (endIndex - startIndex) + 1;
        }
        else
        {
            for(qint32 i = startIndex; (i >= endIndex) && (i < m_sendHistory.length()); i--)
            {
                m_sendHistorySendData.push_back(m_sendHistory[i]);
            }
            count += (startIndex - endIndex) + 1;
        }
    }

    m_historySendIsInProgress = true;

    enableHistoryGuiElements(false);

    m_mainWindow->m_userInterface->progressBar->setMaximum(count);
    m_mainWindow->m_userInterface->progressBar->setValue(0);

    emit sendDataWithTheMainInterfaceSignal(m_sendHistorySendData.first(), MainInterfaceThread::SEND_ID_HISTOTRY);
    m_sendHistorySendData.pop_front();
}



/**
 * The history console timer slot.
 */
void MainWindowHandleData::historyConsoleTimerSlot()
{
    m_historyConsoleTimer.stop();

    m_mainWindow->m_userInterface->clearHistoryPushButton->setEnabled(true);
    m_mainWindow->m_userInterface->sendHistoryPushButton->setEnabled(true);
    m_mainWindow->m_userInterface->createScriptPushButton->setEnabled(true);

    QString currentFormat = m_mainWindow->m_userInterface->historyFormatComboBox->currentText();
    const Settings* currentSettings = m_settingsDialog->settings();

    bool isBinary = (currentFormat == "bin") ? true : false;
    bool isHex = (currentFormat == "hex") ? true : false;
    bool isAscii = (currentFormat == "ascii") ? true : false;
    DecimalType decimalType = SendWindow::formatToDecimalType(currentFormat);

    m_mainWindow->setUpdatesEnabled(false);
    m_mainWindow->m_userInterface->historyTextEdit->clear();

    for(qint32 i = 0; m_sendHistory.size() > i ; i++)
    {
        QString text = QString("index: %1<br>").arg(i);

        if(isAscii)
        {
            QString tmpString;
            for(auto el : m_sendHistory[i])
            {
                if (el == '<')tmpString += "&lt;";
                else if (el == '>')tmpString += "&gt;";
                else if (el == ' ')tmpString += "&nbsp;";
                else if (el == '\n')tmpString += "";
                else if (el == '\r')tmpString += "";
                else if (el < 33 || el > 126) tmpString += 255;
                else tmpString += el;
            }
            text += tmpString;
        }
        else
        {
            text += MainWindow::byteArrayToNumberString(m_sendHistory[i], isBinary , isHex, false, true, true,
                                                        decimalType, currentSettings->targetEndianess);
        }
        text += "<br>";
        m_mainWindow->m_userInterface->historyTextEdit->append(text);
    }

    m_mainWindow->m_userInterface->historyTextEdit->verticalScrollBar()->setValue(0);
    m_mainWindow->m_userInterface->historyTextEdit->moveCursor(QTextCursor::Start);
    m_mainWindow->setUpdatesEnabled(true);

}

/**
 * Returns the number of bytes for a decimal type.
 * @param decimalType
 *      The decimal type.
 * @return
 *      The number of bytes.
 */
qint32 MainWindowHandleData::bytesPerDecimalInConsole(DecimalType decimalType)
{
    qint32 ret = false;
    if((decimalType == DECIMAL_TYPE_UINT8) || (decimalType == DECIMAL_TYPE_INT8))
    {
        ret = 1;
    }
    else if((decimalType == DECIMAL_TYPE_UINT16) || (decimalType == DECIMAL_TYPE_INT16))
    {
        ret = 2;
    }
    else
    {
        ret = 4;
    }

    return ret;

}
/**
 * Reinserts the data into the consoles.
 */
void MainWindowHandleData::reInsertDataInConsole(void)
{

    int val1 = 0;
    int val2 = 0;
    int val3 = 0;
    int val4 = 0;
    int val5 = 0;
    int val6 = 0;
    QMessageBox box(QMessageBox::Information, "ScriptCommunicator", "Recalculating console data",
                    QMessageBox::NoButton, m_mainWindow);
    box.setStandardButtons(QMessageBox::NoButton);
    const Settings* settings = m_settingsDialog->settings();


    if(settings->showMixedConsole && (m_bytesInStoredConsoleData > 2500))
    {
        QApplication::setActiveWindow(&box);
        box.setModal(false);
        box.show();

        QApplication::processEvents();
    }

    if(settings->showAsciiInConsole){val1 = m_userInterface->ReceiveTextEditAscii->verticalScrollBar()->value();}
    if(settings->showHexInConsole){val2 = m_userInterface->ReceiveTextEditHex->verticalScrollBar()->value();}
    if(settings->showDecimalInConsole){val3 = m_userInterface->ReceiveTextEditDecimal->verticalScrollBar()->value();}
    if(settings->showMixedConsole){val4 = m_userInterface->ReceiveTextEditMixed->verticalScrollBar()->value();}
    if(settings->showBinaryConsole){val5 = m_userInterface->ReceiveTextEditBinary->verticalScrollBar()->value();}
    if(settings->consoleShowCustomConsole){val6 = m_userInterface->ReceiveTextEditCustom->verticalScrollBar()->value();}

    m_userInterface->ReceiveTextEditMixed->clear();
    m_userInterface->ReceiveTextEditAscii->clear();
    m_userInterface->ReceiveTextEditDecimal->clear();
    m_userInterface->ReceiveTextEditHex->clear();
    m_userInterface->ReceiveTextEditBinary->clear();
    m_userInterface->ReceiveTextEditCustom->clear();
    m_decimalConsoleByteBuffer.clear();
    m_mixedConsoleByteBuffer.clear();

    calculateConsoleData();

    for(auto el : m_storedConsoleData)
    {
        bool isFromAddMessageDialog = (el.type == STORED_DATA_TYPE_USER_MESSAGE) ? true : false;
        bool isTimeStamp = (el.type == STORED_DATA_TYPE_TIMESTAMP) ? true : false;
        bool isNewLine = (el.type == STORED_DATA_TYPE_NEW_LINE) ? true : false;

        appendDataToConsoleStrings(el.data, settings, el.isSend , isFromAddMessageDialog, isTimeStamp, el.isFromCan, isNewLine);
    }

    if(settings->consoleShowCustomConsole)
    {
        m_customConsoleStrings = m_customConsoleStoredStrings;
        m_customConsoleStoredStrings.clear();
        m_numberOfBytesInCustomConsoleStoredStrings = 0;
    }

    updateConsoleAndLog();

    if(settings->showAsciiInConsole){m_userInterface->ReceiveTextEditAscii->verticalScrollBar()->setValue(val1);}
    if(settings->showHexInConsole){m_userInterface->ReceiveTextEditHex->verticalScrollBar()->setValue(val2);}
    if(settings->showDecimalInConsole){m_userInterface->ReceiveTextEditDecimal->verticalScrollBar()->setValue(val3);}
    if(settings->showMixedConsole){m_userInterface->ReceiveTextEditMixed->verticalScrollBar()->setValue(val4);}
    if(settings->showBinaryConsole){m_userInterface->ReceiveTextEditBinary->verticalScrollBar()->setValue(val5);}
    if(settings->consoleShowCustomConsole){m_userInterface->ReceiveTextEditCustom->verticalScrollBar()->setValue(val6);}

    box.close();
}

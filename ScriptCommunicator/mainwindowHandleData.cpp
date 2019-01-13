#include "mainwindowHandleData.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "settingsdialog.h"
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
    m_sentBytes(0),m_htmlLogFile(), m_HtmlLogFileStream(&m_htmlLogFile), m_textLogFile(), m_textLogFileStream(&m_textLogFile),
    m_bytesInUnprocessedConsoleData(0), m_bytesInStoredConsoleData(0), m_bytesSinceLastNewLineInConsole(0), m_bytesSinceLastNewLineInLog(0),
    m_historySendIsInProgress(false), m_noConsoleVisible(false)
{

    m_updateConsoleAndLogTimer = new QTimer(this);
    m_updateConsoleAndLogTimer->setSingleShot(true);
    connect(m_updateConsoleAndLogTimer, SIGNAL(timeout()), this, SLOT(updateConsoleAndLog()));

}

/**
 * Destructor.
 */
MainWindowHandleData::~MainWindowHandleData()
{
    delete m_updateConsoleAndLogTimer;
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


    if(m_bytesInStoredConsoleData > (settings->maxCharsInConsole + (settings->maxCharsInConsole / 5)))
    {
        int elementToRemove = 0;

        //Limit the data in m_storedConsoleData to settings->maxCharsInConsole.
        while(m_bytesInStoredConsoleData > settings->maxCharsInConsole)
        {
            int diff = m_bytesInStoredConsoleData - settings->maxCharsInConsole;
            if(diff >= m_storedConsoleData.at(elementToRemove).data.length())
            {
                m_bytesInStoredConsoleData -= m_storedConsoleData.at(elementToRemove).data.length();
                elementToRemove++;
            }
            else
            {
                m_storedConsoleData[elementToRemove].data.remove(0, diff);
                m_bytesInStoredConsoleData -= diff;
            }
        }

        if(elementToRemove != 0)
        {
            m_storedConsoleData.remove(0, elementToRemove);
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
 * @param isFromI2cMaster
 *      True if the data is from a I2C master.
 */
void MainWindowHandleData::appendDataToStoredData(QByteArray &data, bool isSend, bool isUserMessage,
                                                  bool isFromCan, bool forceTimeStamp, bool isFromI2cMaster)
{
    const Settings* currentSettings = m_settingsDialog->settings();

    if(currentSettings->htmlLogFile || currentSettings->textLogFile)
    {
        if((!isSend && currentSettings->writeReceivedDataInToLog) || (isSend && currentSettings->writeSendDataInToLog) || isUserMessage)
        {
            appendUnprocessLogData(data, isSend, isUserMessage, isFromCan, isFromI2cMaster, forceTimeStamp);
        }
    }

    if((!isSend && currentSettings->showReceivedDataInConsole) || (isSend && currentSettings->showSendDataInConsole) || isUserMessage)
    {
        if(!m_noConsoleVisible)
        {
            appendUnprocessConsoleData(data, isSend, isUserMessage, isFromCan, isFromI2cMaster, forceTimeStamp);
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
            {//updateConsoleAndLog has been called already.

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
        m_userInterface->ReceiveTextEditMixed->setWordWrapMode (QTextOption::WrapAnywhere);
    }
    else
    {
        m_userInterface->ReceiveTextEditMixed->setLineWrapMode(QTextEdit::NoWrap);
        m_userInterface->ReceiveTextEditMixed->setWordWrapMode (QTextOption::NoWrap);
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
                bool closeSpan = false;

                tmpString = QString::fromLocal8Bit(asciiArray);
                result += "<br>";

                qint32 modulo = m_consoleData.mixedData.bytesPerDecimal;
                ///Create the ascii string.
                for(int i = 0; i < tmpString.length(); i++)
                {
                    if(!currentSettings->showDecimalInConsole || ((i % modulo) == 0))
                    {
                        asciiString += "&nbsp;";    // uncolored
                        asciiString += QString("<span style=background-color:#%1>").arg(currentSettings->consoleMixedAsciiColor);
                        asciiString += m_consoleData.mixedData.asciiSpaces;
                        closeSpan = true;
                    }


                    //Replace tags so our span does not get mangled up.
                    if (tmpString[i] == '<')asciiString += "&lt;";
                    else if (tmpString[i] == '>')asciiString += "&gt;";
                    else if (tmpString[i] == ' ')
                    {
                        if(i < (tmpString.length() - 1))
                        {//The last element is not reached yet.

                            asciiString += "&nbsp;";
                        }
                        else
                        {
                            //If the last element is a &nbsp; then QTextEdit discards some characters from that line (bug in QTextEdit?).
                            //Because of this a '_' is added whose text color is the same like the background color.
                            asciiString += QString("<span style=\"color:#" + currentSettings->consoleMixedAsciiColor + ";\">");
                            asciiString += "_";
                            asciiString += "</span>";
                        }
                    }

                    else if (tmpString[i] < 33 || tmpString[i] > 126) asciiString += 255;
                    else asciiString += tmpString[i];

                    if(closeSpan)
                    {
                        asciiString += "</span>";
                        closeSpan = false;
                    }
                }

                result += asciiString;
            }

            if(currentSettings->showHexInConsole)
            {
                result += "<br>";

                //Create the hex string.
                tmpString = MainWindow::byteArrayToNumberString(arrayWithMaxBytes, false, true, false);
                QStringList list = tmpString.split(" ");
                qint32 modulo = m_consoleData.mixedData.bytesPerDecimal;
                bool closeSpan = false;

                for(int i = 0; i < list.length(); i++)
                {
                    if(!currentSettings->showDecimalInConsole || ((i % modulo) == 0))
                    {
                        result += "&nbsp;";    // uncolored
                        result += QString("<span style=background-color:#%1>").arg(currentSettings->consoleMixedHexadecimalColor);
                        result += m_consoleData.mixedData.hexSpaces;
                        closeSpan = true;
                    }

                    result += list[i];

                    if(closeSpan)
                    {
                        result += "</span>";
                        closeSpan = false;
                    }
                }
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
                bool closeSpan = false;

                for(int i = 0; i < list.length(); i++)
                {
                    if(!currentSettings->showDecimalInConsole || ((i % modulo) == 0))
                    {
                        result += "&nbsp;";     // uncolored
                        result += QString("<span style=background-color:#%1>").arg(currentSettings->consoleMixedBinaryColor);
                        closeSpan = true;
                    }
                    result += list[i];

                    if(closeSpan)
                    {
                        result += "</span>";
                        closeSpan = false;
                    }
                }
            }

            if(convertedBytes < data.length())
            {
                result += "<br>";
            }


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
 *      True if the message is from a CAN interface.
 * @param isFromI2cMaster
 *      True if the message is from a I2C interface.
 * @param isNewLine
 *      True if the data is a new line.
 */
void MainWindowHandleData::appendDataToConsoleStrings(QByteArray &data, const Settings* currentSettings, bool isSend, bool isUserMessage,
                                            bool isTimeStamp, bool isFromCan, bool isFromI2cMaster, bool isNewLine)
{
    QByteArray* dataArray = &data;
    QByteArray tmpArray;


    QString additionalInformation;
    if(data.isEmpty()) return;

    if(isNewLine)
    {
        QString tmpString = QString::fromLocal8Bit(data);
        //Note: "\n" is not replaces with "<br>" because in MainWindow::appendConsoleStringToConsole for every "\n"
        //a new block is created (much better performance).

        if(currentSettings->showDecimalInConsole)m_consoleDataBufferDec.append(tmpString);
        if(currentSettings->showHexInConsole)m_consoleDataBufferHex.append(tmpString);
        if(currentSettings->showBinaryConsole)m_consoleDataBufferBinary.append(tmpString);
        if(currentSettings->showAsciiInConsole)m_consoleDataBufferAscii.append(tmpString);
    }
    else
    {
        if(isUserMessage || isTimeStamp)
        {
            bool startsWithNewLine = false;

            QString tmpString = QString::fromLocal8Bit(data);

            if(tmpString.startsWith("\n"))
            {
                startsWithNewLine = true;
                tmpString.remove(0, 1);//Remove the first '\n'.
            }
            tmpString.replace("<", "&lt;");
            tmpString.replace(">", "&gt;");
            tmpString.replace("\n", "<br>");
            tmpString.replace(" ", "&nbsp;");

            //Note: The "\n" at the beginning of tmpString is not replaces with "<br>" because in MainWindow::appendConsoleStringToConsole for every "\n"
            //a new block is created (much better performance).
            QString htmlStartString = startsWithNewLine ? "\n" : "";
            htmlStartString += m_consoleData.htmlMessageAndTimestamp + tmpString + QString("</span>");

            if(currentSettings->showDecimalInConsole)m_consoleDataBufferDec.append(htmlStartString);
            if(currentSettings->showHexInConsole)m_consoleDataBufferHex.append(htmlStartString);
            if(currentSettings->showMixedConsole)m_consoleDataBufferMixed.append(htmlStartString);
            if(currentSettings->showBinaryConsole)m_consoleDataBufferBinary.append(htmlStartString);
            if(currentSettings->showAsciiInConsole)m_consoleDataBufferAscii.append(htmlStartString);
        }
        else
        {
            QString* htmlStartString =  (isSend) ? &m_consoleData.htmlSend : &m_consoleData.htmlReceived;

            if(isFromCan)
            {
                tmpArray = QByteArray(data);

                if(currentSettings->showCanMetaInformationInConsole)
                {
                    quint8 type = tmpArray[0];
                    QString typeString;

                    QByteArray idArray = tmpArray.mid(PCANBasicClass::BYTES_FOR_CAN_TYPE, PCANBasicClass::BYTES_FOR_CAN_ID);
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

                    additionalInformation = "<br>id: " +  messageIdString + " type: " + typeString + "&nbsp;&nbsp;&nbsp;";
                }

                if(isSend){tmpArray.remove(0, PCANBasicClass::BYTES_METADATA_SEND);}
                else{tmpArray.remove(0, PCANBasicClass::BYTES_METADATA_RECEIVE);}
                dataArray = &tmpArray;

            }
            if(isFromI2cMaster)
            {

                tmpArray = QByteArray(data);

                if(currentSettings->i2cMetaInformationInConsole != I2C_METADATA_NONE)
                {
                    AardvarkI2cFlags flags = (AardvarkI2cFlags)((quint8)tmpArray[0]);

                    if((currentSettings->i2cMetaInformationInConsole == I2C_METADATA_ADDRESS_AND_FLAGS) ||
                       (currentSettings->i2cMetaInformationInConsole == I2C_METADATA_ADDRESS))
                    {
                        quint16 slaveAddress = (quint16)tmpArray[2] + ((quint16)tmpArray[1] << 8);
                        additionalInformation = "<br>addr: 0x" +  QString::number(slaveAddress, 16);
                        if(currentSettings->i2cMetaInformationInConsole == I2C_METADATA_ADDRESS_AND_FLAGS)
                        {
                            additionalInformation +=  " flags: " + AardvarkI2cSpi::flagsToString(flags);
                        }
                    }
                    else
                    {//I2C_METADATA_FLAGS

                        additionalInformation =  "<br>flags: " + AardvarkI2cSpi::flagsToString(flags);
                    }

                    additionalInformation +=  "&nbsp;&nbsp;&nbsp;";
                }

                if(isSend){tmpArray.remove(0, AardvarkI2cSpi::SEND_CONTROL_BYTES_COUNT);}
                else{tmpArray.remove(0, AardvarkI2cSpi::RECEIVE_CONTROL_BYTES_COUNT);}
                dataArray = &tmpArray;
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
                m_consoleDataBufferDec.append(*htmlStartString + additionalInformation + MainWindow::byteArrayToNumberString(*usedArray, false, false, false, true, true, currentSettings->consoleDecimalsType, currentSettings->targetEndianess)
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
            if(currentSettings->showHexInConsole)m_consoleDataBufferHex.append(*htmlStartString + additionalInformation + MainWindow::byteArrayToNumberString(*dataArray, false, true, false) + " " + QString("</span>"));
            if(currentSettings->showBinaryConsole)m_consoleDataBufferBinary.append(*htmlStartString + additionalInformation + MainWindow::byteArrayToNumberString(*dataArray, true, false, false) + " " + QString("</span>"));

            if(currentSettings->showMixedConsole)
            {
                if(!currentSettings->showDecimalInConsole || m_consoleData.mixedData.bytesPerDecimal == 1)
                {
                    m_consoleDataBufferMixed.append(*htmlStartString + additionalInformation + createMixedConsoleString(*dataArray, isFromCan && currentSettings->showCanMetaInformationInConsole) + QString("</span>\n"));
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

                    m_consoleDataBufferMixed.append(*htmlStartString + additionalInformation + createMixedConsoleString(tmpData, isFromCan && currentSettings->showCanMetaInformationInConsole) + QString("</span>\n"));
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

                m_consoleDataBufferAscii.append(*htmlStartString + additionalInformation + tmpString + QString("</span>"));
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
 *      True if the message is from a CAN interface
 * @param isFromI2cMaster
 *      True if the message is from a I2C interface
 * @param isNewLine
 *      True if the data is a new line.
 */
void MainWindowHandleData::appendDataToLog(const QByteArray &data, bool isSend, bool isUserMessage, bool isTimeStamp,
                                 bool isFromCan, bool isFromI2cMaster, bool isNewLine)
{

    const Settings* currentSettings = m_settingsDialog->settings();
    const QByteArray* dataArray = &data;
    QByteArray tmpArray;


    if(data.size() > 0)
    {

        QString dataString;
        QString additionalInformation;
        if(isFromCan && !isUserMessage && !isTimeStamp && !isNewLine)
        {
            tmpArray = QByteArray(data);

            if(currentSettings->writeCanMetaInformationInToLog)
            {
                quint8 type = tmpArray[0];
                QString typeString;

                QByteArray idArray = tmpArray.mid(PCANBasicClass::BYTES_FOR_CAN_TYPE, PCANBasicClass::BYTES_FOR_CAN_ID);
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


                additionalInformation = "\nid: " +  messageIdString + " type: " + typeString + "   ";
                dataString += additionalInformation;
            }
            if(isSend){tmpArray.remove(0, PCANBasicClass::BYTES_METADATA_SEND);}
            else{tmpArray.remove(0, PCANBasicClass::BYTES_METADATA_RECEIVE);}
            dataArray = &tmpArray;

        }

        if(isFromI2cMaster && !isUserMessage && !isTimeStamp && !isNewLine)
        {

            tmpArray = QByteArray(data);

            if(currentSettings->i2cMetaInformationInLog != I2C_METADATA_NONE)
            {
                AardvarkI2cFlags flags = (AardvarkI2cFlags)((quint8)tmpArray[0]);

                if((currentSettings->i2cMetaInformationInLog == I2C_METADATA_ADDRESS_AND_FLAGS) ||
                   (currentSettings->i2cMetaInformationInLog == I2C_METADATA_ADDRESS))
                {
                    quint16 slaveAddress = (quint16)tmpArray[2] + ((quint16)tmpArray[1] << 8);
                    additionalInformation = "\naddr: 0x" +  QString::number(slaveAddress, 16);
                    if(currentSettings->i2cMetaInformationInLog == I2C_METADATA_ADDRESS_AND_FLAGS)
                    {
                        additionalInformation +=  " flags: " + AardvarkI2cSpi::flagsToString(flags);
                    }
                }
                else
                {//I2C_METADATA_FLAGS

                    additionalInformation =  "\nflags: " + AardvarkI2cSpi::flagsToString(flags);
                }

                additionalInformation +=  "   ";
                dataString += additionalInformation;
            }

            if(isSend){tmpArray.remove(0, AardvarkI2cSpi::SEND_CONTROL_BYTES_COUNT);}
            else{tmpArray.remove(0, AardvarkI2cSpi::RECEIVE_CONTROL_BYTES_COUNT);}
            dataArray = &tmpArray;
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
 * The slot is called if the main interface thread has received can messages.
 * This slot is connected to the MainInterfaceThread::canMessageReceivedSignal signal.
 * @param data
 *      The received data.
 */
void MainWindowHandleData::dataReceivedSlot(QByteArray data)
{
    m_receivedBytes += data.size();

    if(m_mainWindow->m_isConnectedWithI2cMaster)
    {
        m_receivedBytes -= AardvarkI2cSpi::RECEIVE_CONTROL_BYTES_COUNT;
    }

    appendDataToStoredData(data, false, false, m_mainWindow->m_isConnectedWithCan,
                           false, m_mainWindow->m_isConnectedWithI2cMaster);
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

        appendDataToStoredData(el, false, false, m_mainWindow->m_isConnectedWithCan, false, m_mainWindow->m_isConnectedWithI2cMaster);
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
        m_sentBytes += data.size();

        if(m_mainWindow->m_isConnectedWithCan)
        {
            m_sentBytes -= PCANBasicClass::BYTES_METADATA_SEND;

            QByteArray tmpIdAndType = data.mid(0, PCANBasicClass::BYTES_METADATA_SEND);
            for(int i = PCANBasicClass::BYTES_METADATA_SEND; i < data.length(); i += PCANBasicClass::MAX_BYTES_PER_MESSAGE)
            {
                QByteArray tmpArray = tmpIdAndType + data.mid(i, PCANBasicClass::MAX_BYTES_PER_MESSAGE);

                appendDataToStoredData(tmpArray, true, false, m_mainWindow->m_isConnectedWithCan, false, m_mainWindow->m_isConnectedWithI2cMaster);
                m_mainWindow->m_canTab->canMessageTransmitted(tmpArray);
            }
        }
        else
        {
            if(m_mainWindow->m_isConnectedWithI2cMaster)
            {
                m_sentBytes -= AardvarkI2cSpi::SEND_CONTROL_BYTES_COUNT;
            }
            appendDataToStoredData(data, true, false, m_mainWindow->m_isConnectedWithCan, false,m_mainWindow->m_isConnectedWithI2cMaster);
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
 * @param isFromI2cMaster
 *      True if the time stamp results from a I2C message
 * @param timeStampFormat
 *      The time stamp format.
 */
void MainWindowHandleData::appendTimestamp(QVector<StoredData>* storedDataVector, bool isSend, bool isUserMessage,
                                 bool isFromCan, bool isFromI2cMaster, QString timeStampFormat)
{
    StoredData storedData;
    storedData.isFromCan = isFromCan;
    storedData.isFromI2cMaster = isFromI2cMaster;
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
 * @param isFromI2cMaster
 *      True if the time stamp results from a I2C message.
 */
void MainWindowHandleData::appendNewLine(QVector<StoredData>* storedDataVector, bool isSend, bool isFromCan, bool isFromI2cMaster)
{
    StoredData storedData;
    storedData.isFromCan = isFromCan;
    storedData.isFromI2cMaster = isFromI2cMaster;
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
 * @param isFromI2cMaster
 *      True if the data is from a I2C master.
 * @param forceTimeStamp
 *      True if a time stamp shall be generated (independently from the time stamp settings)
 * @param isRecursivCall
 *      True if the function is called recursively.
 */
void MainWindowHandleData::appendUnprocessConsoleData(QByteArray &data, bool isSend, bool isUserMessage,
                                            bool isFromCan, bool isFromI2cMaster, bool forceTimeStamp, bool isRecursivCall)
{
    static QDateTime lastConsoleTimeInBuffer = QDateTime::currentDateTime().addSecs(-1000);
    static QDateTime lastSendReceivedDataInConsole = QDateTime::currentDateTime().addYears(1);

    const Settings* currentSettings = m_settingsDialog->settings();
    static bool createTimeStampOnNextCall = false;

    if(!isRecursivCall)
    {
        bool timeStampIsAllowed = false;
        if((!isFromCan || !currentSettings->showCanMetaInformationInConsole) && (!isFromI2cMaster || (currentSettings->i2cMetaInformationInConsole == I2C_METADATA_NONE))
                && !isUserMessage)
        {
            timeStampIsAllowed = true;
        }

        if(createTimeStampOnNextCall)
        {
            //Enter a console time stamp.
            appendTimestamp(&m_unprocessedConsoleData, isSend, isUserMessage, isFromCan, isFromI2cMaster, currentSettings->consoleTimestampFormat);
            m_bytesInUnprocessedConsoleData += m_unprocessedConsoleData.last().data.length();
            createTimeStampOnNextCall = false;
        }
        if(currentSettings->consoleCreateTimestampAt && (data.indexOf((quint8)currentSettings->consoleTimestampAt) != -1)
           && timeStampIsAllowed)
        {//Data contains a time stamp at byte.

            /*************************create time stamps for all time stamp at bytes (currentSettings->consoleTimestampAt)**********************/
            QList<QByteArray> splittedData;
            splittedData = data.split((quint8)currentSettings->consoleTimestampAt);

            for(qint32 i = 0; i < splittedData.length(); i++)
            {
                if(isFromI2cMaster && (currentSettings->i2cMetaInformationInConsole == I2C_METADATA_NONE) && (i != 0))
                {
                    //Add I2C dummy metadata to every element (al but the first).
                    QByteArray dummyBytes;
                    if(isSend){dummyBytes.fill(0, AardvarkI2cSpi::SEND_CONTROL_BYTES_COUNT);}
                    else{dummyBytes.fill(0, AardvarkI2cSpi::RECEIVE_CONTROL_BYTES_COUNT);}
                    splittedData[i].push_front(dummyBytes);
                }
                else if(isFromCan && (!currentSettings->showCanMetaInformationInConsole) && (i != 0))
                {
                    //Add CAN dummy metadata to every element (al but the first).
                    QByteArray dummyBytes;
                    if(isSend){dummyBytes.fill(0, PCANBasicClass::BYTES_METADATA_SEND);}
                    else{dummyBytes.fill(0, PCANBasicClass::BYTES_METADATA_RECEIVE);}
                    splittedData[i].push_front(dummyBytes);
                }

                if(i < (splittedData.length() - 1))
                {//The last entry in splittedData is not reached.

                    if(!splittedData[i].isEmpty())
                    {
                        //Append a time stamp byte (was removed during split)
                        appendUnprocessConsoleData(splittedData[i].append((quint8)currentSettings->consoleTimestampAt), isSend, isUserMessage, isFromCan, isFromI2cMaster, forceTimeStamp, true);
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
                    appendTimestamp(&m_unprocessedConsoleData, isSend, isUserMessage, isFromCan, isFromI2cMaster, currentSettings->consoleTimestampFormat);
                    m_bytesInUnprocessedConsoleData += m_unprocessedConsoleData.last().data.length();
                }
                else
                {
                    if(!splittedData[i].isEmpty())
                    {
                        appendUnprocessConsoleData(splittedData[i], isSend, isUserMessage, isFromCan, isFromI2cMaster, forceTimeStamp, true);
                    }
                }

            }//for(qint32 i = 0; i < splittedData.length(); i++)

            return;
        }

        if(forceTimeStamp || (currentSettings->generateTimeStampsInConsole && (lastConsoleTimeInBuffer.msecsTo(QDateTime::currentDateTime()) > (qint64)currentSettings->timeStampIntervalConsole)))
        {
            appendTimestamp(&m_unprocessedConsoleData, isSend, isUserMessage, isFromCan, isFromI2cMaster, currentSettings->consoleTimestampFormat);
            m_bytesInUnprocessedConsoleData += m_unprocessedConsoleData.last().data.length();
            lastConsoleTimeInBuffer = QDateTime::currentDateTime();

        }

        if((currentSettings->consoleNewLineAt != 0xffff) && (data.indexOf((quint8)currentSettings->consoleNewLineAt) != -1)
                && timeStampIsAllowed)
        {//Data contains a new line byte.

            /*************************create new line entries for all new line bytes (currentSettings->consoleNewLineAt)**********************/
            QList<QByteArray> splittedData;
            splittedData = data.split((quint8)currentSettings->consoleNewLineAt);

            for(qint32 i = 0; i < splittedData.length(); i++)
            {
                if(isFromI2cMaster && (currentSettings->i2cMetaInformationInConsole == I2C_METADATA_NONE) && (i != 0))
                {
                    //Add I2C dummy metadata to every element (al but the first).
                    QByteArray dummyBytes;
                    if(isSend){dummyBytes.fill(0, AardvarkI2cSpi::SEND_CONTROL_BYTES_COUNT);}
                    else{dummyBytes.fill(0, AardvarkI2cSpi::RECEIVE_CONTROL_BYTES_COUNT);}
                    splittedData[i].push_front(dummyBytes);
                }
                else if(isFromCan && (!currentSettings->showCanMetaInformationInConsole) && (i != 0))
                {
                    //Add CAN dummy metadata to every element (al but the first).
                    QByteArray dummyBytes;
                    if(isSend){dummyBytes.fill(0, PCANBasicClass::BYTES_METADATA_SEND);}
                    else{dummyBytes.fill(0, PCANBasicClass::BYTES_METADATA_RECEIVE);}
                    splittedData[i].push_front(dummyBytes);
                }

                appendUnprocessConsoleData(splittedData[i], isSend, isUserMessage, isFromCan, isFromI2cMaster, isUserMessage, true);

                if(i < (splittedData.length() - 1))
                {//The last entry in splittedData is not reached.

                    //Append a new line byte (was removed during split)
                    m_unprocessedConsoleData.last().data.append((quint8)currentSettings->consoleNewLineAt);
                    m_bytesInUnprocessedConsoleData ++;

                    //Append a new line.
                    appendNewLine(&m_unprocessedConsoleData, isSend, isFromCan, isFromI2cMaster);
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
                appendNewLine(&m_unprocessedConsoleData, isSend, isFromCan, isFromI2cMaster);
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


    if((lastEntryTypeInStoredData != newEntryType) || isFromCan || isFromI2cMaster)
    {
        StoredData newStoredDataEntry;
        newStoredDataEntry.data = data;
        newStoredDataEntry.isFromCan = isFromCan;
        newStoredDataEntry.isFromI2cMaster = isFromI2cMaster;
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
 * @param isFromI2cMaster
 *      True if the data is from a I2C master.
 * @param forceTimeStamp
 *      True if a time stamp shall be generated (independently from the time stamp settings)
 * @param isRecursivCall
 *      True if the function is called recursively.
 */
void MainWindowHandleData::appendUnprocessLogData(const QByteArray &data, bool isSend, bool isUserMessage,
                                        bool isFromCan, bool isFromI2cMaster, bool forceTimeStamp, bool isRecursivCall)
{
    static QDateTime lastLogTimeInBuffer = QDateTime::currentDateTime().addSecs(-1000);
    static QDateTime lastSendReceivedDataInLog = QDateTime::currentDateTime().addYears(1);

    const Settings* currentSettings = m_settingsDialog->settings();
    static bool createTimeStampOnNextCall = false;

    if(!isRecursivCall)
    {
        bool timeStampIsAllowed = false;
        if((!isFromCan || !currentSettings->writeCanMetaInformationInToLog) && (!isFromI2cMaster || (currentSettings->i2cMetaInformationInLog == I2C_METADATA_NONE))
                && !isUserMessage)
        {
            timeStampIsAllowed = true;
        }

        if(createTimeStampOnNextCall)
        {
            //Enter a log time stamp.
            appendTimestamp(&m_unprocessedLogData, isSend, isUserMessage, isFromCan, isFromI2cMaster, currentSettings->logTimestampFormat);
            createTimeStampOnNextCall = false;
        }

        if(currentSettings->logCreateTimestampAt && (data.indexOf((quint8)currentSettings->logTimestampAt) != -1)
                && timeStampIsAllowed)
        {//Data contains a time stamp at byte.

            /*************************create time stamps for all time stamp at bytes (currentSettings->logTimestampAt)**********************/
            QList<QByteArray> splittedData;
            splittedData = data.split((quint8)currentSettings->logTimestampAt);

            for(qint32 i = 0; i < splittedData.length(); i++)
            {
                if(isFromI2cMaster && (currentSettings->i2cMetaInformationInLog == I2C_METADATA_NONE) && (i != 0))
                {
                    //Add I2C dummy metadata to every element (al but the first).
                    QByteArray dummyBytes;
                    if(isSend){dummyBytes.fill(0, AardvarkI2cSpi::SEND_CONTROL_BYTES_COUNT);}
                    else{dummyBytes.fill(0, AardvarkI2cSpi::RECEIVE_CONTROL_BYTES_COUNT);}
                    splittedData[i].push_front(dummyBytes);
                }
                else if(isFromCan && (!currentSettings->writeCanMetaInformationInToLog) && (i != 0))
                {
                    //Add CAN dummy metadata to every element (al but the first).
                    QByteArray dummyBytes;
                    if(isSend){dummyBytes.fill(0, PCANBasicClass::BYTES_METADATA_SEND);}
                    else{dummyBytes.fill(0, PCANBasicClass::BYTES_METADATA_RECEIVE);}
                    splittedData[i].push_front(dummyBytes);
                }

                if(i < (splittedData.length() - 1))
                {//The last entry in splittedData is not reached.

                    if(!splittedData[i].isEmpty())
                    {
                        //Append a time stamp byte (was removed during split)
                        appendUnprocessLogData(splittedData[i].append((quint8)currentSettings->logTimestampAt), isSend, isUserMessage,
                                               isFromCan, isFromI2cMaster, forceTimeStamp, true);
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
                    appendTimestamp(&m_unprocessedLogData, isSend, isUserMessage, isFromCan, isFromI2cMaster, currentSettings->logTimestampFormat);
                }
                else
                {
                    appendUnprocessLogData(splittedData[i], isSend, isUserMessage, isFromCan, isFromI2cMaster, forceTimeStamp, true);
                }
            }//for(qint32 i = 0; i < splittedData.length(); i++)

            return;
        }

        if(forceTimeStamp || (currentSettings->generateTimeStampsInLog &&
          (lastLogTimeInBuffer.msecsTo(QDateTime::currentDateTime()) > (qint64)currentSettings->timeStampIntervalLog)))
        {
            appendTimestamp(&m_unprocessedLogData, isSend, isUserMessage, isFromCan, isFromI2cMaster, currentSettings->logTimestampFormat);
            lastLogTimeInBuffer = QDateTime::currentDateTime();

        }


        if((currentSettings->logNewLineAt != 0xffff) && (data.indexOf((quint8)currentSettings->logNewLineAt) != -1)
                && timeStampIsAllowed)
        {//Data contains a new line byte.

            /*************************create new line entries for all new line bytes (currentSettings->logNewLineAt)**********************/
            QList<QByteArray> splittedData;
            splittedData = data.split((quint8)currentSettings->logNewLineAt);

            for(qint32 i = 0; i < splittedData.length(); i++)
            {
                if(isFromI2cMaster && (currentSettings->i2cMetaInformationInLog == I2C_METADATA_NONE) && (i != 0))
                {
                    //Add I2C dummy metadata to every element (al but the first).
                    QByteArray dummyBytes;
                    if(isSend){dummyBytes.fill(0, AardvarkI2cSpi::SEND_CONTROL_BYTES_COUNT);}
                    else{dummyBytes.fill(0, AardvarkI2cSpi::RECEIVE_CONTROL_BYTES_COUNT);}
                    splittedData[i].push_front(dummyBytes);
                }
                else if(isFromCan && (!currentSettings->writeCanMetaInformationInToLog) && (i != 0))
                {
                    //Add CAN dummy metadata to every element (al but the first).
                    QByteArray dummyBytes;
                    if(isSend){dummyBytes.fill(0, PCANBasicClass::BYTES_METADATA_SEND);}
                    else{dummyBytes.fill(0, PCANBasicClass::BYTES_METADATA_RECEIVE);}
                    splittedData[i].push_front(dummyBytes);
                }

                appendUnprocessLogData(splittedData[i], isSend, isUserMessage, isFromCan, isFromI2cMaster, forceTimeStamp, true);

                if(i < (splittedData.length() - 1))
                {//The last entry in splittedData is not reached.

                    //Append a new line byte (was removed during split)
                    m_unprocessedLogData.last().data.append((quint8)currentSettings->logNewLineAt);

                    //Append a new line.
                    appendNewLine(&m_unprocessedLogData, isSend, isFromCan, isFromI2cMaster);
                }

            }

            return;
        }
        if(currentSettings->logNewLineAfterPause != 0)
        {
            if(lastSendReceivedDataInLog.msecsTo(QDateTime::currentDateTime()) > (qint64)currentSettings->logNewLineAfterPause)
            {
                //Append a new line.
                appendNewLine(&m_unprocessedLogData, isSend, isFromCan, isFromI2cMaster);

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


    if((lastEntryTypeInStoredData != newEntryType) || isFromCan || isFromI2cMaster)
    {
        StoredData newStoredDataEntry;
        newStoredDataEntry.data = data;
        newStoredDataEntry.isFromCan = isFromCan;
        newStoredDataEntry.isFromI2cMaster = isFromI2cMaster;
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
            m_userInterface->ReceiveTextEditAscii->document()->blockSignals(true);
            m_userInterface->ReceiveTextEditHex->document()->blockSignals(true);
            m_userInterface->ReceiveTextEditDecimal->document()->blockSignals(true);
            m_userInterface->ReceiveTextEditMixed->document()->blockSignals(true);
            m_userInterface->ReceiveTextEditBinary->document()->blockSignals(true);

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

            m_userInterface->ReceiveTextEditAscii->document()->blockSignals(false);
            m_userInterface->ReceiveTextEditHex->document()->blockSignals(false);
            m_userInterface->ReceiveTextEditDecimal->document()->blockSignals(false);
            m_userInterface->ReceiveTextEditMixed->document()->blockSignals(false);
            m_userInterface->ReceiveTextEditBinary->document()->blockSignals(false);
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

                        appendDataToConsoleStrings(tmpArray, settings, el.isSend , isFromAddMessageDialog, isTimeStamp,
                                                   el.isFromCan, el.isFromI2cMaster, isNewLine);
                        array.remove(0, settings->consoleNewLineAfterBytes - m_bytesSinceLastNewLineInConsole);

                        tmpArray = QString("\n").toLocal8Bit();
                        //Save the console data before calling appendDataToConsoleStrings (0 are replace by 0xff in this function).
                        storedData.data = tmpArray;
                        storedData.type = STORED_DATA_TYPE_NEW_LINE;
                        m_storedConsoleData.push_back(storedData);
                        m_bytesInStoredConsoleData += storedData.data.size();

                        appendDataToConsoleStrings(tmpArray, settings, el.isSend , isFromAddMessageDialog, isTimeStamp,
                                                   el.isFromCan, el.isFromI2cMaster, true);

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

                    appendDataToConsoleStrings(array, settings, el.isSend , isFromAddMessageDialog, isTimeStamp,
                                               el.isFromCan, el.isFromI2cMaster, isNewLine);
                    m_bytesSinceLastNewLineInConsole += array.length();
                }

            }
            else
            {//New line after x bytes is not activated.

                //Save the console data bevore calling appendDataToConsoleStrings (0 are replace by 0xff in this function).
                m_storedConsoleData.push_back(el);
                m_bytesInStoredConsoleData += el.data.size();

                appendDataToConsoleStrings(el.data, settings, el.isSend , isFromAddMessageDialog, isTimeStamp,
                                           el.isFromCan, el.isFromI2cMaster, isNewLine);

                if((m_mainWindow->m_dataRateReceive + m_mainWindow->m_dataRateSend) > 10000)
                {//The send/received data rate is bigger then 10000.

                    m_bytesSinceLastNewLineInConsole += el.data.length();

                    if(m_bytesSinceLastNewLineInConsole > 1000000)
                    {
                        //After 1000000 added bytes without a new line (and with a data rate bigger then 10000) a new line is added to
                        //improve the performance of the consoles.
                        QByteArray tmp = QString("\n").toLocal8Bit();
                        appendDataToConsoleStrings(tmp, settings, el.isSend , isFromAddMessageDialog, isTimeStamp,
                                                   el.isFromCan, el.isFromI2cMaster,  true);
                        m_bytesSinceLastNewLineInConsole = 0;
                    }
                }
                else
                {
                    m_bytesSinceLastNewLineInConsole = 0;
                }
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
                                        el.isSend , isFromAddMessageDialog, isTimeStamp, el.isFromCan, el.isFromI2cMaster, isNewLine);
                        array->remove(0, settings->logNewLineAfterBytes - m_bytesSinceLastNewLineInLog);

                        QByteArray tmpArray = QString("\n").toLocal8Bit();
                        appendDataToLog(tmpArray, el.isSend , isFromAddMessageDialog, isTimeStamp, el.isFromCan, el.isFromI2cMaster, true);

                        m_bytesSinceLastNewLineInLog = 0;

                    }while(array->length() >= (qint32)settings->logNewLineAfterBytes);

                }

                if(!array->isEmpty())
                {
                    appendDataToLog(*array, el.isSend , isFromAddMessageDialog, isTimeStamp, el.isFromCan, el.isFromI2cMaster, isNewLine);
                    m_bytesSinceLastNewLineInLog += array->length();
                }

            }
            else
            {//New line after x bytes is not activated.
                appendDataToLog(*array, el.isSend, isFromAddMessageDialog, isTimeStamp, el.isFromCan, el.isFromI2cMaster, isNewLine);
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

    m_mainWindow->m_resizeTimer.stop();

    if(settings.showMixedConsole)
    {
        QMessageBox box(QMessageBox::Information, "ScriptCommunicator", "Reformatting console data",
                        QMessageBox::NoButton, m_mainWindow);
        box.setStandardButtons(QMessageBox::NoButton);

        if(m_bytesInStoredConsoleData > 5000)
        {
            QApplication::setActiveWindow(&box);
            box.setModal(false);
            box.show();

            QApplication::processEvents();
        }


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

            appendDataToConsoleStrings(el.data, &settings, el.isSend , isFromAddMessageDialog, isTimeStamp,
                                       el.isFromCan, el.isFromI2cMaster, isNewLine);
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

    m_sendHistory.push_back(*data);

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

    int pos = m_mainWindow->m_userInterface->historyTextEdit->verticalScrollBar()->value();
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

    m_mainWindow->m_userInterface->historyTextEdit->verticalScrollBar()->setValue(pos);
    //m_mainWindow->m_userInterface->historyTextEdit->moveCursor(QTextCursor::Start);
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

    QMessageBox box(QMessageBox::Information, "ScriptCommunicator", "Recalculating console data",
                    QMessageBox::NoButton, m_mainWindow);
    box.setStandardButtons(QMessageBox::NoButton);
    const Settings* settings = m_settingsDialog->settings();


    m_userInterface->ReceiveTextEditMixed->document()->blockSignals(true);
    m_userInterface->ReceiveTextEditAscii->document()->blockSignals(true);
    m_userInterface->ReceiveTextEditDecimal->document()->blockSignals(true);
    m_userInterface->ReceiveTextEditHex->document()->blockSignals(true);
    m_userInterface->ReceiveTextEditBinary->document()->blockSignals(true);

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

    m_userInterface->ReceiveTextEditMixed->clear();
    m_userInterface->ReceiveTextEditAscii->clear();
    m_userInterface->ReceiveTextEditDecimal->clear();
    m_userInterface->ReceiveTextEditHex->clear();
    m_userInterface->ReceiveTextEditBinary->clear();
    m_decimalConsoleByteBuffer.clear();
    m_mixedConsoleByteBuffer.clear();

    m_bytesSinceLastNewLineInConsole = 0;

    calculateConsoleData();

    for(auto el : m_storedConsoleData)
    {
        bool isFromAddMessageDialog = (el.type == STORED_DATA_TYPE_USER_MESSAGE) ? true : false;
        bool isTimeStamp = (el.type == STORED_DATA_TYPE_TIMESTAMP) ? true : false;
        bool isNewLine = (el.type == STORED_DATA_TYPE_NEW_LINE) ? true : false;

        appendDataToConsoleStrings(el.data, settings, el.isSend , isFromAddMessageDialog, isTimeStamp,
                                   el.isFromCan, el.isFromI2cMaster, isNewLine);
    }


    updateConsoleAndLog();

    if(settings->showAsciiInConsole){m_userInterface->ReceiveTextEditAscii->verticalScrollBar()->setValue(val1);}
    if(settings->showHexInConsole){m_userInterface->ReceiveTextEditHex->verticalScrollBar()->setValue(val2);}
    if(settings->showDecimalInConsole){m_userInterface->ReceiveTextEditDecimal->verticalScrollBar()->setValue(val3);}
    if(settings->showMixedConsole){m_userInterface->ReceiveTextEditMixed->verticalScrollBar()->setValue(val4);}
    if(settings->showBinaryConsole){m_userInterface->ReceiveTextEditBinary->verticalScrollBar()->setValue(val5);}

    m_userInterface->ReceiveTextEditMixed->document()->blockSignals(false);
    m_userInterface->ReceiveTextEditAscii->document()->blockSignals(false);
    m_userInterface->ReceiveTextEditDecimal->document()->blockSignals(false);
    m_userInterface->ReceiveTextEditHex->document()->blockSignals(false);
    m_userInterface->ReceiveTextEditBinary->document()->blockSignals(false);

    box.close();
}

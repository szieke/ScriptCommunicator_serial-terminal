#ifndef MAINWINDOWHANDLEDATA_H
#define MAINWINDOWHANDLEDATA_H

#include <QObject>
#include <QThread>
#include <QTime>
#include <QFile>
#include<QTextStream>
#include <QMessageBox>
#include <QXmlStreamWriter>
#include <QTimer>
#include <QJSEngine>
#include "settingsdialog.h"


class MainWindow;

namespace Ui {
class MainWindow;
}

///The type of data in a StoredData struct.
typedef enum
{
    ///Received data.
    STORED_DATA_TYPE_RECEIVE,

    ///Send data.
    STORED_DATA_TYPE_SEND,

    ///Timestamp.
    STORED_DATA_TYPE_TIMESTAMP,

    ///User message (add message dialog).
    STORED_DATA_TYPE_USER_MESSAGE,

    ///New line.
    STORED_DATA_TYPE_NEW_LINE,

    ///Clear all standard consoles.
    STORED_DATA_CLEAR_ALL_STANDARD_CONSOLES,

    ///Invalid entry.
    STORED_DATA_TYPE_INVALID

}StoredDataType;

///The stored console/log data.
typedef struct
{
    StoredDataType type;
    QByteArray data;
    bool isFromCan;
    bool isFromI2cMaster;
    bool isSend;

}StoredData;

///The precalculated data which is needed for the mixed console.
typedef struct
{   ///The number of pixels per character.
    int pixelsWide;

    ///The divider for the bytes bytes per line calcualtion.
    double divider;

    ///True if only the type is in the mixed console (utf8, or hex...).
    bool onlyOneType;

    ///The bytes per decimal.
    int bytesPerDecimal;

    ///The max. number of bytes per line.
    int maxBytePerLine;

    ///The spaces for the utf8 characters.
    QString utf8Spaces;

    ///The spaces for the hex characters.
    QString hexSpaces;

    ///The extra spaces for the hex characters.
    QString hexExtraSpaces;

    ///The spaces for the decimal characters.
    QString decimalSpaces;


}MixedConsoleData;

///The precalculated data which is needed for the all consoles.
typedef struct
{
    ///The data for the mixed console.
    MixedConsoleData mixedData;

    ///The HTML data for time stamps and user messages.
    QString htmlMessageAndTimestamp;

    ///The HTML data for received data.
    QString htmlReceived;

    ///The HTML data for sent data.
    QString htmlSend;

}ConsoleData;


///Contains the MainWindow functions for handling the sent and recieved data.
class MainWindowHandleData : public QObject
{
    Q_OBJECT
    friend class MainWindow;
public:
    explicit MainWindowHandleData(MainWindow* mainWindow, SettingsDialog* settingsDialog,
                                  Ui::MainWindow* userInterface);

    ~MainWindowHandleData();

    ///Appends data to the m_storedData.
    void appendDataToStoredData(QByteArray &data, bool isSend, bool isUserMessage, bool isFromCan, bool forceTimeStamp, bool isFromI2cMaster);

    ///Creates the string for the mixed console.
    QString createMixedConsoleString(const QByteArray &data, bool hasCanMeta);

    ///Caclulates the console data.
    void calculateConsoleData();

    ///Appends data to the console strings (m_consoleDataBufferUtf8, m_consoleDataBufferHex;
    ///m_consoleDataBufferDec)
    void appendDataToConsoleStrings(QByteArray& data, const Settings *currentSettings, bool isSend, bool isUserMessage,
                                    bool isTimeStamp, bool isFromCan, bool isFromI2cMaster, bool isNewLine);

    ///Appends data the log file.
    void appendDataToLog(const QByteArray& data, bool isSend, bool isUserMessage, bool isTimeStamp, bool isFromCan,
                         bool isFromI2cMaster, bool isNewLine);

    ///Clears all stored data.
    void clear(void);

    ///Reinserts the data into the consoles.
    void reInsertDataInConsole(void);

    ///Returns the number of bytes for a decimal type.
    qint32 bytesPerDecimalInConsole(DecimalType decimalType);

    ///Adds data to the send history.
    void addDataToSendHistory(const QByteArray* data);

    ///Send the history.
    void sendHistory(void);

    ///Cancel send the history.
    void cancelSendHistory(void);

    ///The color of timestamps and user messages in the HTML log.
    static constexpr char* LOG_TIMESTAMP_AND_MESSAGE_COLOR = "#7c0000";

    ///The color of the sent data in the HTML log.
    static constexpr char* LOG_SEND_COLOR = "#7c0000";

    ///The color of the received data in the HTML log.
    static constexpr char* LOG_RECEIVE_COLOR = "#000000";

    ///The background color in the HTML log.
    static constexpr char* LOG_BACKGROUND_COLOR = "#efefef";

signals:
    ///This signal is emitted for sending data with the main interface.
    void sendDataWithTheMainInterfaceSignal(const QByteArray data, uint id);

public slots:

    ///The history console timer slot.
    void historyConsoleTimerSlot();

    ///The send history timer slot.
    void sendHistoryTimerSlot();

    ///This function appends the received/send data to the consoles and the logs.
    ///It is called periodically (log update interval).
    void updateConsoleAndLog(void);

    ///This slot is called if the main interface thread has received data.
    ///It is connected to the MainInterfaceThread::dataReceivedSignal signal.
    void dataReceivedSlot(QByteArray data);

    ///This slot is called if the main interface thread has received data.
    ///It is connected to the MainInterfaceThread::dataReceivedSignal signal.
    void canMessagesReceivedSlot(QVector<QByteArray> messages);

    ///This slot is called if the main interface thread has send data.
    ///It is connected to the MainInterfaceThread::sendingFinishedSignal signal.
    void dataHasBeenSendSlot(QByteArray data, bool success, uint id);

    ///Reinserts the data into the mixed consoles.
    void reInsertDataInMixecConsoleSlot(void);

private:

    ///Enables/disables the send history GUI elements.
    void enableHistoryGuiElements(bool enable);

    ///Append a time stamp to a stored data vector.
    void appendTimestamp(QVector<StoredData>* storedDataVector, bool isSend, bool isUserMessage,
                         bool isFromCan, bool isFromI2cMaster, QString timeStampFormat);

    ///Append a new line to a stored data vector.
    void appendNewLine(QVector<StoredData>* storedDataVector, bool isSend,
                                     bool isFromCan, bool isFromI2cMaster);

    ///Appends data to the unprocessed console data.
    void appendUnprocessConsoleData(QByteArray &data, bool isSend, bool isUserMessage, bool isFromCan, bool isFromI2cMaster,
                                    bool forceTimeStamp=false, bool isRecursivCall=false);

    ///Appends data to the unprocessed log data.
    void appendUnprocessLogData(const QByteArray &data, bool isSend, bool isUserMessage, bool isFromCan,
                                bool isFromI2cMaster, bool forceTimeStamp=false, bool isRecursivCall=false);

    ///Processes the data in m_storedData (creates the log and the console strings).
    ///Note: m_storedData is cleared in this function.
   void processDataInStoredData();

    ///Pointer to the main window.
    MainWindow* m_mainWindow;

    ///Pointer to the settings dialog.
    SettingsDialog *m_settingsDialog;

    ///Pointer to the main windiw user interface.
    Ui::MainWindow *m_userInterface;

    ///Cyclic timer which call the function updateConsoleAndLog.
    QTimer *m_updateConsoleAndLogTimer ;

    ///The data buffer for the utf8 console.
    QString m_consoleDataBufferUtf8;

    ///The data buffer for the hex console.
    QString m_consoleDataBufferHex;

    ///The data buffer for the decimal console.
    QString m_consoleDataBufferDec;

    ///The data buffer for the mixed console.
    QString m_consoleDataBufferMixed;

    ///The data buffer for the binary console.
    QString m_consoleDataBufferBinary;

    ///Time stamp for the last console entry.
    QTime lastTimeInConsole;

    ///The number of received bytes.
    quint64 m_receivedBytes;

    ///The number of sent bytes.
    quint64 m_sentBytes;

    ///The html log file.
    QFile m_htmlLogFile;

    ///The html log file stream.
    QTextStream m_HtmlLogFileStream;

    ///The text log file.
    QFile m_textLogFile;

    ///The text log file stream.
    QTextStream m_textLogFileStream;

    ///The unprocessed console data.
    QVector<StoredData> m_unprocessedConsoleData;

    ///The unprocessed log data.
    QVector<StoredData> m_unprocessedLogData;

    ///Bytes in m_unprocessedLogData;
    quint32 m_bytesInUnprocessedConsoleData;

    ///The stored console data.
    QVector<StoredData> m_storedConsoleData;

    ///Bytes in m_storedConsoleData;
    quint32 m_bytesInStoredConsoleData;

    ///The number of sent/received bytes after the last new line console
    quint32 m_bytesSinceLastNewLineInConsole;

    ///The number of sent/received bytes after the last new line log
    quint32 m_bytesSinceLastNewLineInLog;

    ///The precalculated console data.
    ConsoleData m_consoleData;

    ///The byte buffer for the decimal console.
    ///If insufficent number of bytes for a decimal are received then these a bytes are stored here.
    QByteArray m_decimalConsoleByteBuffer;

    ///The byte buffer for the mixed console.
    ///If insufficent number of bytes for a decimal are received then these a bytes are stored here.
    QByteArray m_mixedConsoleByteBuffer;

    ///The byte buffer for the decimal log.
    ///If insufficent number of bytes for a decimal are received then these a bytes are stored here.
    QByteArray m_decimalLogByteBuffer;

    ///The send history buffer.
    QVector<QByteArray> m_sendHistory;

    ///The history data which must be sent.
    QVector<QByteArray> m_sendHistorySendData;

    ///The max. number of elements in m_sendHistory.
    static const qint32 MAX_SEND_HISTORY_ENTRIES = 30;

    ///The history console timer.
    QTimer m_historyConsoleTimer;

    ///The send history timer.
    QTimer m_sendHistoryTimer;

    ///True if the history is currently sent.
    bool m_historySendIsInProgress;

    ///True if no console is visible in the main window.
    bool m_noConsoleVisible;

};

#endif // MAINWINDOWHANDLEDATA_H

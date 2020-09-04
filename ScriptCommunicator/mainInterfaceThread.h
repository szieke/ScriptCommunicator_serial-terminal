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

#ifndef MAININTERFACETHREAD_H
#define MAININTERFACETHREAD_H

#include "mainwindow.h"
#include <QTimer>
#include "PCANBasicClass.h"
#include "aardvarkI2cSpi.h"
#include <QNetworkProxy>


///The thread for the main interface.
class MainInterfaceThread : public QThread
{
    Q_OBJECT
    friend class MainWindow;
    friend class ScriptThread;
    friend class ScriptInf;

public:
    MainInterfaceThread(MainWindow* mainWindow);
    ~MainInterfaceThread();

    ///Timeout (ms)for sending data (5 minutes).
    static const uint SEND_TIMEOUT = (1000 * 60 * 5);

    ///Start value for the send thread send ids .
    static const quint32 SEND_ID_SCRIPTS_START = 1000;

    ///Send id for the cyclic sending in the send window.
    static const quint32 SEND_ID_SEND_WINDOW_CYCLIC = 1;

    ///Send id for the single sending in the send window.
    static const quint32 SEND_ID_SEND_WINDOW_SINGLE = 2;

    ///Send id for the routing functionality.
    static const quint32 SEND_ID_ROUTING = 3;

    ///Send id for the send history functionality.
    static const quint32 SEND_ID_HISTOTRY = 4;

    ///Send id for I2C/SPI slave send data.
    static const quint32 SEND_ID_I2C_SPI_SLAVE = 5;

    ///The max. send size for an UDP socket.
    static const qint32 UDP_MAX_SEND_SIZE = 512;

    ///True if the main interface thread is connected.
    bool isConnected();

    ///True if the main interface thread is connected with a CAN interface.
    bool isConnectedWithCan();

    ///True if the main interface thread is connected with a I2C master interface.
    bool isConnectedWithI2cMaster();

    ///Returns true if the main interface thread is connected with a I2C slave interface.
    bool isConnectedWithI2cSlave();

    ///Returns true if the main interface thread is connected with a SPI slave interface.
    bool isConnectedWithSpiSlave();

    ///True if the main interface thread is connected with a I2C interface.
    bool isConnectedWithI2c();

    ///Converts the serial port pinout signal to an information string
    ///(RTS=0, CTS=0, DSR=0, DCD=0, DTR=0, RI=0).
    QString serialPortPinoutSignalsToInfoString(void);

    ///This function is called if data has been received
    ///and emits the dataReceivedSignal signal.
    void dataReceived(QByteArray& data);

    ///Returns m_isInitialized.
    bool isInitialized(void){return m_isInitialized;}

    ///The max. number for queued send orders.
    static const qint32 MAX_NUMBER_IN_SEND_QUEUE = 20;

    ///The time base for the data rate calcualtion.
    static const quint32 DATA_RATE_TIME_BASE_SECONDS = 2;

signals:

    ///The main interface thread emits this signal if his connection state has been changed.
    void dataConnectionStatusSignal(bool isConnected, QString message, bool isWaiting);

    ///The main interface thread emits this signal to show additional information about the connection in the main window.
    void showAdditionalConnectionInformationSignal(QString text);

    ///The main interface thread emits this signal if data has been received.
    void dataReceivedSignal(QByteArray data);

    ///The main interface thread emits this signal if can messages have been received.
    void canMessagesReceivedSignal(QVector<QByteArray> messages);

    ///The main interface thread emits this signal if the sending of data has been finished.
    void sendingFinishedSignal(bool success, uint id);

    ///The main interface thread emits this signal if the sending of data has been finished.
    void sendingFinishedSignal(QByteArray data, bool success, uint id);

    ///The main interface thread emits this signal to enable or disable main window connect button.
    void setConnectionButtonsSignal(bool enable);

    ///The main interface thread emits this signal to show a QMessageBox dialog in the main window.
    void showMessageBoxSignal(QMessageBox::Icon icon, QString title, QString text, QMessageBox::StandardButtons buttons);

    ///Disables all mouse events for all windows.
    void disableMouseEventsSignal(void);

    ///Enables all mouse events for all windows.
    void enableMouseEventsSignal(void);

    ///Signal which publishes the current data rates.
    void dataRateUpdateSignal(quint32 dataRateSend, quint32 dataRateReceive);

    ///Send the send data to all worker scripts which must send the data too.
    void sendDataWithWorkerScriptsSignal(const QByteArray data);

    ///Is called if the main interface is a I2C or SPI slave and has sent data.
    void slaveDataSentSignal(QByteArray data);


protected:
    ///The main interface thread main function.
    void run();

public slots:

    ///The slot function is used to connect to or disconnect from the main interface.
    void connectDataConnectionSlot(Settings globalSettings, bool shallConnect, bool showMessageBoxOnError);

    ///The slot function is called if the global settings have been changed.
    void globalSettingsChangedSlot(Settings globalSettings);

    ///Call this slot function to exit the main interface thread.
    void exitThreadSlot(void);

    ///Slot function for sending data with main interface thread.
    void sendDataSlot(const QByteArray data, uint id);

    ///Returns the state of the serial port signals (pins).
    ///The signals are bit coded:
    ///NoSignal = 0x00,
    ///DataTerminalReadySignal = 0x04,
    ///DataCarrierDetectSignal = 0x08,
    ///DataSetReadySignal = 0x10,
    ///RingIndicatorSignal = 0x20,
    ///RequestToSendSignal = 0x40,
    ///ClearToSendSignal = 0x80,
    void getSerialPortSignals(uint32_t* bits){if(m_serial){*bits = (((quint32)m_serial->pinoutSignals()) & 0xfc);}}


private slots:

   ///This timer function updates the additional connection information which are shown in the main window.
   void showAdditionalInformationTimerSlot(void);

    ///This slot function is called if data has been received from the serial port.
    void serialPortReceivedDataSlot(void);

    ///This slot function is called if data has been received from the aardvard interface (I2C/SPI slave mode).
    void aardvardSlaveDataReceivedSlot(void);

    ///This slot function is called if data has been received from the pcan interface.
    void pcanReceivedDataSlot(void);

    ///This slot function is called if an external tcp client has been connected to the internal tcp server.
    void tcpServerOnNewConnectionSlot(void);

    ///This slot function is called if an external tcp client has been disconnected from the internal tcp server.
    void tcpServerSocketOnDisconnectedSlot(void);

    ///This slot function is called of the internal tcp server has received data.
    void tcpServerSocketOnReadyReadSlot(void);

    ///This slot function is called if an internal tcp client has been connected to an external tcp server.
    void tcpClientSocketOnConnectedSlot(void);

    ///Is called if an error has ocurred.
    void tcpClientSocketErrorSlot(QAbstractSocket::SocketError socketError);

    ///This slot function is called if an internal tcp client has been disconnected from an external tcp server.
    void tcpClientSocketOnDisconnectedSlot(void);

    ///This slot function is called of the internal tcp client has received data.
    void tcpClientSocketOnReadyReadSlot(void);

    ///This slot function is called of the internal udp server has received data.
    void udpServerSocketOnReadyReadSlot(void);

    ///Data rate timer slot.
    void dataRateTimerSlot(void);

private:

    ///Creates a network proxy.
    QNetworkProxy createProxy();

    ///Shows a message box.
   void showMessageBox(QMessageBox::Icon icon, QString title, QString text);

    ///Sends data with the main interface.
    bool sendDataWithTheMainInterface(const QByteArray &data, QByteArray& receivedData,
                                      bool waitForSendingFinished, bool *serialPortSignalBlocked);

    ///If true, then the main interface thread shall exit.
    bool m_exit;


    ///Pointer to the main window.
    MainWindow* m_mainWindow;

    ///Pointer to the serial port.
    QSerialPort* m_serial;

    ///Pointer to the tcp server.
    QTcpServer* m_tcpServer;

    ///Pointer to the tcp sever sockets.
    QVector<QTcpSocket*> m_tcpServerSockets;

    ///Pointer to the tcp client socket.
    QTcpSocket* m_tcpClientSocket;

    ///The current global setting.
    Settings m_currentGlobalSettings;

    ///Pointer to the udp server socket.
    QUdpSocket* m_udpServerSocket;

    ///Pointer to the udp client socket.
    QUdpSocket* m_udpClientSocket;

    ///Pointer to the aardvark I2C SPI interface.
    AardvarkI2cSpi* m_aardvarkI2cSpi;

    ///True of the main interface is connected.
    bool m_isConnected;

    ///This timer calls the showAdditionalInformationTimerSlot periodically.
    QTimer* m_showAdditionalInformationTimer;

    ///PCAN Interface.
    PCANBasicClass* m_pcanInterface;

    ///The current number of sent bytes.
    uint64_t m_numberOfSentBytes;

    ///The last number of sent bytes.
    uint64_t m_lastNumberOfSentBytes;

    ///The current number of received bytes.
    uint64_t m_numberOfReceivedBytes;

    ///The last number of received bytes.
    uint64_t m_lastNumberOfReceivedBytes;

    ///Data rate timer.
    QTimer* m_dataRateTimer;

    ///True if the main interface thread is initialized.
    bool m_isInitialized;

    ///True if a message box shall be shown in case of an error.
    bool m_showMessageBoxOnError;

};

#endif // MAININTERFACETHREAD_H

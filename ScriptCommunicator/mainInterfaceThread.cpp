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

#include "mainInterfaceThread.h"
#include "ui_mainwindow.h"
#include "settingsdialog.h"
#include "sendwindow.h"
#include <QTimer>
#include <QScrollBar>
#include <QMutex>
#include <QTime>

#include <QMessageBox>
#include <QtSerialPort/QSerialPort>
#include <QFileDialog>
#include <QDomDocument>
#include "scriptwindow.h"
#include <QNetworkProxyFactory>
#include <QNetworkProxy>
#include "scriptTcpClient.h"



/**
 * Constructor.
 * @param mainWindow
 *      Pointer to the main window.
 */
MainInterfaceThread::MainInterfaceThread(MainWindow* mainWindow):m_exit(false),
    m_serial(0),m_tcpServer(0),m_tcpServerSockets(),m_tcpClientSocket(0),
    m_udpServerSocket(0), m_udpClientSocket(0), m_aardvarkI2cSpi(0), m_isConnected(false), m_showAdditionalInformationTimer(0), m_pcanInterface(0),
    m_numberOfSentBytes(0), m_lastNumberOfSentBytes(0), m_numberOfReceivedBytes(0),m_lastNumberOfReceivedBytes(0),  m_dataRateTimer(0),
    m_isInitialized(false), m_showMessageBoxOnError(true)
{
    m_mainWindow = mainWindow;
}

/**
 * Destructor.
 */
MainInterfaceThread::~MainInterfaceThread()
{

}

/**
 * This slot function is called if data has been received from the aardvard interface (I2C/SPI slave mode).
 */
void MainInterfaceThread::aardvardSlaveDataReceivedSlot(void)
{
    QVector<AardvardkI2cSpiSlaveData> transActions = m_aardvarkI2cSpi->readLastSlaveData();

    for(auto el : transActions)
    {
        if(el.isReceiveData)
        {
            dataReceived(el.data);
        }
        else
        {
            m_numberOfSentBytes += el.data.size();
            emit sendingFinishedSignal(el.data, true, SEND_ID_I2C_SPI_SLAVE);
            emit slaveDataSentSignal(el.data);
        }
    }
}

/**
 * This slot function is called if data has been received from the pcan interface.
 */
void MainInterfaceThread::pcanReceivedDataSlot(void)
{
    QByteArray data = m_pcanInterface->readLastMessage();
    QVector<QByteArray> messages;
    while(!data.isEmpty())
    {
        messages.append(data);
        m_numberOfReceivedBytes += data.size() - PCANBasicClass::BYTES_METADATA_RECEIVE;

        data = m_pcanInterface->readLastMessage();
    }

    if(!messages.empty())
    {
        emit canMessagesReceivedSignal(messages);
    }
}

/**
 * This slot function is called if data has been received from the serial port.
 */
void MainInterfaceThread::serialPortReceivedDataSlot(void)
{
    QByteArray data = m_serial->readAll();
    if(!data.isEmpty())
    {
        dataReceived(data);
    }
}

/**
 * This slot function is called if an external tcp client has been disconnected from the internal tcp server.
 */
void MainInterfaceThread::tcpServerSocketOnDisconnectedSlot(void)
{
    QTcpSocket* socket = static_cast<QTcpSocket*>(sender());
    disconnect(socket, SIGNAL(disconnected()));
    disconnect(socket, SIGNAL(readyRead()));

    m_isConnected = false;

    socket->deleteLater();

    for(int i = 0; i < m_tcpServerSockets.length(); i++)
    {
        if(socket == m_tcpServerSockets[i])
        {
            m_tcpServerSockets.remove(i);
            break;
        }
    }

    if(m_tcpServerSockets.length() == 0)
    {
        connectDataConnectionSlot(m_currentGlobalSettings, true, true);
    }
    else
    {
        QString tmp = "connected to";

        for(int i = 0; i < m_tcpServerSockets.length(); i++)
        {
            if(i > 0)
            {
                tmp +=" |";
            }
            tmp += QString(" address:%1 port:%2").arg(
                        socket->peerAddress().toString()).arg(
                        socket->peerPort());
        }
        emit dataConnectionStatusSignal(true, tmp, false);
        emit showAdditionalConnectionInformationSignal("");
    }
}

/**
 * This slot function is called of the internal tcp server has received data.
 */
void MainInterfaceThread::tcpServerSocketOnReadyReadSlot(void)
{
    QTcpSocket* socket = static_cast<QTcpSocket*>(sender());
    if(socket->isReadable())
    {
        QByteArray data = socket->readAll();
        dataReceived(data);
    }
}

/**
 * Converts the serial port pinout signal to an information string
 * (CTS=0, DSR=0, DCD=0, DTR=0).
 * @return
 *      The created string.
 */
QString MainInterfaceThread::serialPortPinoutSignalsToInfoString(void)
{
   QSerialPort::PinoutSignals pinSignals = m_serial->pinoutSignals();

    QString result = QString("CTS=%1, DSR=%2, DCD=%3, RI=%4")
            .arg((QSerialPort::ClearToSendSignal & pinSignals) ? 1 : 0)
            .arg((QSerialPort::DataSetReadySignal & pinSignals) ? 1 : 0)
            .arg((QSerialPort::DataCarrierDetectSignal & pinSignals) ? 1 : 0)
            .arg((QSerialPort::RingIndicatorSignal & pinSignals) ? 1 : 0);

    return result;
}

/**
 * Returns true if the main interface thread is connected.
 */
bool MainInterfaceThread::isConnected()
{
    return m_isConnected;
}

/**
 * Returns true if the main interface thread is connected with a CAN interface.
 */
bool MainInterfaceThread::isConnectedWithCan()
{
    return m_pcanInterface->isConnected();
}

/**
 * Returns true if the main interface thread is connected with a I2C interface.
 */
bool MainInterfaceThread::isConnectedWithI2c()
{
    bool result = false;

    if((m_aardvarkI2cSpi->isConnected()) &&
       ((m_currentGlobalSettings.aardvarkI2cSpi.deviceMode == AARDVARK_I2C_SPI_DEVICE_MODE_I2C_MASTER) ||
        (m_currentGlobalSettings.aardvarkI2cSpi.deviceMode == AARDVARK_I2C_SPI_DEVICE_MODE_I2C_SLAVE)))
    {
        result = true;
    }

    return result;
}


/**
 * Returns true if the main interface thread is connected with a I2C master interface.
 */
bool MainInterfaceThread::isConnectedWithI2cMaster()
{
    bool result = false;

    if((m_aardvarkI2cSpi->isConnected()) &&
       (m_currentGlobalSettings.aardvarkI2cSpi.deviceMode == AARDVARK_I2C_SPI_DEVICE_MODE_I2C_MASTER))
    {
        result = true;
    }

    return result;
}

/**
 * Returns true if the main interface thread is connected with a I2C slave interface.
 */
bool MainInterfaceThread::isConnectedWithI2cSlave()
{
    bool result = false;

    if((m_aardvarkI2cSpi->isConnected()) &&
       (m_currentGlobalSettings.aardvarkI2cSpi.deviceMode == AARDVARK_I2C_SPI_DEVICE_MODE_I2C_SLAVE))
    {
        result = true;
    }

    return result;
}

/**
 * Returns true if the main interface thread is connected with a SPI slave interface.
 */
bool MainInterfaceThread::isConnectedWithSpiSlave()
{
    bool result = false;

    if((m_aardvarkI2cSpi->isConnected()) &&
       (m_currentGlobalSettings.aardvarkI2cSpi.deviceMode == AARDVARK_I2C_SPI_DEVICE_MODE_SPI_SLAVE))
    {
        result = true;
    }

    return result;
}

/**
 * This slot function is called if an external tcp client has been connected to the internal tcp server.
 */
void MainInterfaceThread::tcpServerOnNewConnectionSlot(void)
{
    QTcpSocket* socket = m_tcpServer->nextPendingConnection();


    m_tcpServerSockets.append(socket);

    connect(socket, SIGNAL(disconnected()),this, SLOT(tcpServerSocketOnDisconnectedSlot()));
    connect(socket, SIGNAL(readyRead()),this, SLOT(tcpServerSocketOnReadyReadSlot()));

    m_isConnected = true;
    QString tmp = "connected to";

    for(int i = 0; i < m_tcpServerSockets.length(); i++)
    {
        if(i > 0)
        {
            tmp +=" |";
        }
        tmp += QString(" address:%1 port:%2").arg(
                    socket->peerAddress().toString()).arg(
                    socket->peerPort());
    }
    emit dataConnectionStatusSignal(true, tmp, false);
    emit showAdditionalConnectionInformationSignal("");

}

/**
 * This function is called if data has been received and emits the dataReceivedSignal signal.
 * @param data
 *      The received data.
 */
void MainInterfaceThread::dataReceived(QByteArray& data)
{
    if(!data.isEmpty())
    {
        emit dataReceivedSignal(data);
        m_numberOfReceivedBytes += data.size();

        if(isConnectedWithI2cMaster())
        {
            ///Remove the bytes for the metadata.
            m_numberOfReceivedBytes -= AardvarkI2cSpi::RECEIVE_CONTROL_BYTES_COUNT;
        }
    }

}

/**
 * This slot function is called of the internal tcp client has received data.
 */
void MainInterfaceThread::tcpClientSocketOnReadyReadSlot()
{
    if(m_tcpClientSocket->isReadable())
    {
        QByteArray data = m_tcpClientSocket->readAll();
        dataReceived(data);
    }
}

/**
 * This slot function is called if an internal tcp client has been connected to an external tcp server.
 */
void MainInterfaceThread::tcpClientSocketOnConnectedSlot(void)
{
    connect(m_tcpClientSocket, SIGNAL(disconnected()),this, SLOT(tcpClientSocketOnDisconnectedSlot()));
    connect(m_tcpClientSocket, SIGNAL(readyRead()),this, SLOT(tcpClientSocketOnReadyReadSlot()));

    m_isConnected = true;
    emit dataConnectionStatusSignal(true, tr("connected to address:%1  port:%2").arg(
                                        m_currentGlobalSettings.socketSettings.destinationIpAddress).arg(
                                        m_currentGlobalSettings.socketSettings.destinationPort), false);
    emit showAdditionalConnectionInformationSignal("");
    emit setConnectionButtonsSignal(true);
}

/**
 * Is called if an error has ocurred.
 * @param socketError
 *  The error.
 */
void MainInterfaceThread::tcpClientSocketErrorSlot(QAbstractSocket::SocketError socketError)
{
    switch (socketError)
    {
        case QAbstractSocket::RemoteHostClosedError:
        {
            break;
        }
        case QAbstractSocket::HostNotFoundError:
        {
            showMessageBox(QMessageBox::Critical, tr("TCP error"),
                                     tr("The server was not found. Please check the "
                                        "host name and port settings."));
            break;
        }
        case QAbstractSocket::ConnectionRefusedError:
        {
            showMessageBox(QMessageBox::Critical, tr("TCP error"),
                                     tr("The connection was refused. "
                                        "Make sure the server is running, "
                                        "and check that the host name and port "
                                        "settings are correct."));
            break;
        }
        default:
        {
            showMessageBox(QMessageBox::Critical, tr("TCP error"),
                                     tr("The following error occurred: %1.")
                                     .arg(m_tcpClientSocket->errorString()));
        }
    }

    connectDataConnectionSlot(m_currentGlobalSettings, false, true);
    emit setConnectionButtonsSignal(true);
}

/**
 * This slot function is called if an internal tcp client has been disconnected from an external tcp server.
 */
void MainInterfaceThread::tcpClientSocketOnDisconnectedSlot(void)
{
    disconnect(m_tcpClientSocket, SIGNAL(disconnected()));
    disconnect(m_tcpClientSocket, SIGNAL(readyRead()));

    m_isConnected = false;
    emit dataConnectionStatusSignal(false, tr("Disconnected"), false);
    emit showAdditionalConnectionInformationSignal("");
}

/**
 * This slot function is called of the internal udp server has received data.
 */
void MainInterfaceThread::udpServerSocketOnReadyReadSlot(void)
{
    while (m_udpServerSocket->hasPendingDatagrams())
    {
        QByteArray datagram;
        datagram.resize(m_udpServerSocket->pendingDatagramSize());
        QHostAddress sender;
        quint16 senderPort;

        m_udpServerSocket->readDatagram(datagram.data(), datagram.size(),
                                        &sender, &senderPort);
        dataReceived(datagram);
    }
}

/**
 * Data rate timer slot.
 */
void MainInterfaceThread::dataRateTimerSlot(void)
{

    uint32_t dataRateSend = 0;
    uint32_t dataRateReceive = 0;

    dataRateSend = (m_numberOfSentBytes - m_lastNumberOfSentBytes) / DATA_RATE_TIME_BASE_SECONDS;
    dataRateReceive = (m_numberOfReceivedBytes - m_lastNumberOfReceivedBytes) / DATA_RATE_TIME_BASE_SECONDS;

    emit dataRateUpdateSignal(dataRateSend, dataRateReceive);

    m_lastNumberOfSentBytes = m_numberOfSentBytes;
    m_lastNumberOfReceivedBytes = m_numberOfReceivedBytes;
}

/**
 * Creates a network proxy.
 * @return
 *  The proxy.
 */
QNetworkProxy MainInterfaceThread::createProxy()
{
    QString type = "NO_PROXY";
    if(m_currentGlobalSettings.socketSettings.proxySettings == 1)
    {
        type = "SYSTEM_PROXY";
    }
    else if(m_currentGlobalSettings.socketSettings.proxySettings == 2)
    {
        type = "CUSTOM_PROXY";
    }
    else
    {
        type = "NO_PROXY";
    }

    QNetworkProxy proxy = ScriptTcpClient::createProxy(type, m_currentGlobalSettings.socketSettings.proxyUserName,
                                                       m_currentGlobalSettings.socketSettings.proxyPassword,
                                                        m_currentGlobalSettings.socketSettings.proxyIpAddress,
                                                        m_currentGlobalSettings.socketSettings.proxyPort);

    return proxy;
}

/**
 * The main interface thread main function.
 */
void MainInterfaceThread::run(void)
{

    m_serial = new QSerialPort(this);
    connect(m_serial, SIGNAL(readyRead()),this, SLOT(serialPortReceivedDataSlot()));

    m_tcpServer = new QTcpServer(this);
    connect(m_tcpServer, SIGNAL(newConnection()),this, SLOT(tcpServerOnNewConnectionSlot()));

    m_tcpClientSocket = new QTcpSocket(this);
    connect(m_tcpClientSocket, SIGNAL(connected()),this, SLOT(tcpClientSocketOnConnectedSlot()));
    connect(m_tcpClientSocket, SIGNAL(error(QAbstractSocket::SocketError)),this, SLOT(tcpClientSocketErrorSlot(QAbstractSocket::SocketError)));

    m_udpServerSocket = new QUdpSocket(this);
    connect(m_udpServerSocket, SIGNAL(readyRead()),this, SLOT(udpServerSocketOnReadyReadSlot()));

    m_aardvarkI2cSpi = new AardvarkI2cSpi(this);
    connect(m_aardvarkI2cSpi, SIGNAL(readyRead()),this, SLOT(aardvardSlaveDataReceivedSlot()));

    m_showAdditionalInformationTimer = new QTimer(this);
    connect(m_showAdditionalInformationTimer, SIGNAL(timeout()),this, SLOT(showAdditionalInformationTimerSlot()));
    m_showAdditionalInformationTimer->start(250);

    m_pcanInterface = new PCANBasicClass(this);
    connect(m_pcanInterface, SIGNAL(readyRead()),this, SLOT(pcanReceivedDataSlot()));

    m_dataRateTimer = new QTimer(this);
    connect(m_dataRateTimer, SIGNAL(timeout()),this, SLOT(dataRateTimerSlot()));
    m_dataRateTimer->start(DATA_RATE_TIME_BASE_SECONDS * 1000);

    m_isInitialized = true;

    exec();

    m_showAdditionalInformationTimer->stop();
    m_dataRateTimer->stop();
}

/**
 * Slot function for sending data with main interface thread.
 * @param data
 *      The data.
 * @param id
 *      The send id.
 */
void MainInterfaceThread::sendDataSlot(const QByteArray data, uint id)
{
    if(data.size() > 0)
    {
        QByteArray receivedData;

        emit sendDataWithWorkerScriptsSignal(data);

        bool serialPortSignalBlocked;
        if(sendDataWithTheMainInterface(data, receivedData, true, &serialPortSignalBlocked))
        {
            bool isI2cRead = false;
            if(isConnectedWithI2cMaster())
            {
                isI2cRead = (data.size() == AardvarkI2cSpi::SEND_CONTROL_BYTES_COUNT) ? true : false;
            }

            sendingFinishedSignal(true, id);

            if(!isI2cRead)
            {
                if(!isConnectedWithI2cSlave() && !isConnectedWithSpiSlave())
                {
                    emit sendingFinishedSignal(data, true, id);
                    m_numberOfSentBytes += data.size();
                }

                if(isConnectedWithCan())
                {
                    ///Remove the bytes for the metadata.
                    m_numberOfSentBytes -= PCANBasicClass::BYTES_METADATA_SEND;
                }

                if(isConnectedWithI2cMaster())
                {
                    ///Remove the bytes for the I2C metadata.
                    m_numberOfSentBytes -= AardvarkI2cSpi::SEND_CONTROL_BYTES_COUNT;
                }

                if(serialPortSignalBlocked && (m_serial->bytesAvailable() != 0))
                {//During waitForBytesWritten the serial port receive signal has been blocked. For this reason
                 //serialPortReceivedDataSlot is manually called if bytes have been received.
                    serialPortReceivedDataSlot();
                }
            }

            if(!receivedData.isEmpty())
            {
                dataReceived(receivedData);
            }

        }
        else
        {
            emit sendingFinishedSignal(false, id);
        }

    }//if(data.size() > 0)
    else
    {
        emit sendingFinishedSignal(false, id);
    }
}

/**
 * Call this slot function to exit the main interface thread.
 */
void MainInterfaceThread::exitThreadSlot()
{
    emit dataConnectionStatusSignal(false, "", false);

    m_serial->close();
    m_tcpClientSocket->close();
    m_tcpServer->close();
    for(int i = 0; i < m_tcpServerSockets.length(); i++)
    {
        m_tcpServerSockets[i]->blockSignals(true);
        m_tcpServerSockets[i]->close();

    }
    m_udpServerSocket->close();

    m_exit = true;
    exit();
}

/**
 * This timer function updates the additional connection information which are shown in the main window.
 */
void MainInterfaceThread::showAdditionalInformationTimerSlot(void)
{
    if(m_serial->isOpen())
    {
        emit showAdditionalConnectionInformationSignal(serialPortPinoutSignalsToInfoString());
    }
}


/**
 * The slot function is called when the global settings have been changed
 * @param globalSettings
 *      The current global settings.
 */
void MainInterfaceThread::globalSettingsChangedSlot(Settings globalSettings)
{

    if(m_serial->isOpen())
    {
        m_currentGlobalSettings.serialPort.setDTR = globalSettings.serialPort.setDTR;
        m_currentGlobalSettings.serialPort.setRTS = globalSettings.serialPort.setRTS;

        m_serial->setDataTerminalReady(m_currentGlobalSettings.serialPort.setDTR);
        m_serial->setRequestToSend(m_currentGlobalSettings.serialPort.setRTS);

    }

}

/**
 * The slot function is used to connect to or disconnect from the main interface.
 * @param globalSettings
 *      The current global settings.
 * @param shallConnect
 *      True for connect or false for disconnect
 */
void MainInterfaceThread::connectDataConnectionSlot(Settings globalSettings, bool shallConnect, bool showMessageBoxOnError)
{
    m_currentGlobalSettings = globalSettings;
    m_showMessageBoxOnError = showMessageBoxOnError;

    if(m_serial->isOpen())
    {
        m_serial->setDataTerminalReady(false);
        m_serial->setRequestToSend(false);
        m_serial->close();
    }

    m_tcpClientSocket->close();

    for(int i = 0; i < m_tcpServerSockets.length(); i++)
    {
        m_tcpServerSockets[i]->blockSignals(true);
        m_tcpServerSockets[i]->close();
        m_tcpServerSockets[i]->deleteLater();
    }
    m_tcpServerSockets.clear();
    m_tcpServer->close();

    m_udpServerSocket->close();
    m_aardvarkI2cSpi->disconnect();
    m_pcanInterface->close();
    m_isConnected = false;

    m_numberOfSentBytes = 0;
    m_lastNumberOfSentBytes = 0;
    m_numberOfReceivedBytes = 0;
    m_lastNumberOfReceivedBytes = 0;

    if(shallConnect)
    {

        if(m_currentGlobalSettings.connectionType == CONNECTION_TYPE_SERIAL_PORT)
        {
            m_serial->setPortName(m_currentGlobalSettings.serialPort.name);
            if (m_serial->open(QIODevice::ReadWrite))
            {
                if (m_serial->setBaudRate(m_currentGlobalSettings.serialPort.baudRate)
                        && m_serial->setDataBits(m_currentGlobalSettings.serialPort.dataBits)
                        && m_serial->setParity(m_currentGlobalSettings.serialPort.parity)
                        && m_serial->setStopBits(m_currentGlobalSettings.serialPort.stopBits)
                        && m_serial->setFlowControl(m_currentGlobalSettings.serialPort.flowControl))
                {
                    m_isConnected = true;

                    m_serial->setDataTerminalReady(m_currentGlobalSettings.serialPort.setDTR);
                    m_serial->setRequestToSend(m_currentGlobalSettings.serialPort.setRTS);

                    emit dataConnectionStatusSignal(true, tr("Connected to %1: baudrate=%2, data bits=%3, parity=%4, stop bits=%5, flow control=%6")
                                                    .arg(m_currentGlobalSettings.serialPort.name)
                                                    .arg(m_currentGlobalSettings.serialPort.stringBaudRate)
                                                    .arg(m_currentGlobalSettings.serialPort.stringDataBits)
                                                    .arg(m_currentGlobalSettings.serialPort.stringParity)
                                                    .arg(m_currentGlobalSettings.serialPort.stringStopBits)
                                                    .arg(m_currentGlobalSettings.serialPort.stringFlowControl), false);
                    emit showAdditionalConnectionInformationSignal(serialPortPinoutSignalsToInfoString());
                }
                else
                {
                    m_isConnected = false;

                    if(m_serial->setBaudRate(m_currentGlobalSettings.serialPort.baudRate))
                    {
                        showMessageBox(QMessageBox::Critical, tr("configure error"), "invalid serial port settings");
                    }
                    else
                    {
                        showMessageBox(QMessageBox::Critical, tr("configure error"), "the current baudrate is not supported by the device");
                    }
                    emit dataConnectionStatusSignal(false, tr("configure error"), false);
                    emit showAdditionalConnectionInformationSignal("CTS=0, DSR=0, DCD=0, RI=0");

                    m_serial->setDataTerminalReady(false);
                    m_serial->setRequestToSend(false);
                    m_serial->close();
                }
            }
            else
            {
                showMessageBox(QMessageBox::Critical, tr("open error"),
                                          "could not open serial port: " + m_currentGlobalSettings.serialPort.name);
                m_isConnected = false;
                emit dataConnectionStatusSignal(false, tr("open error"), false);
                emit showAdditionalConnectionInformationSignal("CTS=0, DSR=0, DCD=0, RI=0");
            }
        }//if(m_currentGlobalSettings.connectionType == CONNECTION_TYPE_SERIAL_PORT)
        else if(m_currentGlobalSettings.connectionType == CONNECTION_TYPE_TCP_CLIENT)
        {
            m_isConnected = false;
            emit dataConnectionStatusSignal(false, tr("connecting to address:%1  port:%2")
                                            .arg( m_currentGlobalSettings.socketSettings.destinationIpAddress)
                                            .arg(m_currentGlobalSettings.socketSettings.destinationPort), false);
            emit showAdditionalConnectionInformationSignal("");

            emit setConnectionButtonsSignal(false);
            m_tcpClientSocket->setProxy(createProxy());
            QHostAddress address(m_currentGlobalSettings.socketSettings.destinationIpAddress);
            m_tcpClientSocket->connectToHost(address, m_currentGlobalSettings.socketSettings.destinationPort);


        }
        else if(m_currentGlobalSettings.connectionType == CONNECTION_TYPE_TCP_SERVER)
        {
            m_isConnected = false;

            if(!m_tcpServer->listen(QHostAddress::Any, m_currentGlobalSettings.socketSettings.ownPort))
            {
                m_tcpServer->close();
                emit dataConnectionStatusSignal(false, tr("could not create tcp server"), false);
                emit showAdditionalConnectionInformationSignal("");

                showMessageBox(QMessageBox::Critical, tr("server error"), QString("could not create tcp server")+
                               " (is the own port already used?)");

            }
            else
            {
                emit dataConnectionStatusSignal(false, tr("waiting for client: port:%1").arg(
                                                        m_tcpServer->serverPort()), true);

                emit showAdditionalConnectionInformationSignal("");
            }
        }//
        else if(m_currentGlobalSettings.connectionType == CONNECTION_TYPE_UDP_SOCKET)
        {
            m_isConnected = true;


            if(!m_udpServerSocket->bind(QHostAddress::Any, m_currentGlobalSettings.socketSettings.ownPort))
            {
                m_udpServerSocket->close();
                emit dataConnectionStatusSignal(false, tr("could not create udp socket"), false);
                emit showAdditionalConnectionInformationSignal("");

                showMessageBox(QMessageBox::Critical, tr("socket error"), QString("could not create udp socket")+
                               " (is the own port already used?)");

            }
            else
            {
                emit dataConnectionStatusSignal(true, tr("waiting for data: port:%1").arg(
                                                       m_udpServerSocket->localPort()), false);
                emit showAdditionalConnectionInformationSignal("");
            }
        }
        else if(m_currentGlobalSettings.connectionType == CONNECTION_TYPE_AARDVARK)
        {
            int deviceBitrate;
            m_isConnected = m_aardvarkI2cSpi->connectToDevice(m_currentGlobalSettings.aardvarkI2cSpi, deviceBitrate);

            QString mode;
            quint16 baudrateSetValue = 0;
            if(m_currentGlobalSettings.aardvarkI2cSpi.deviceMode == AARDVARK_I2C_SPI_DEVICE_MODE_I2C_MASTER)
            {
                mode = "i2c master";
                baudrateSetValue = m_currentGlobalSettings.aardvarkI2cSpi.i2cBaudrate;
            }
            else if(m_currentGlobalSettings.aardvarkI2cSpi.deviceMode == AARDVARK_I2C_SPI_DEVICE_MODE_I2C_SLAVE)
            {
                mode = QString("i2c slave, address=%1").arg(m_currentGlobalSettings.aardvarkI2cSpi.i2cSlaveAddress);
                baudrateSetValue = m_currentGlobalSettings.aardvarkI2cSpi.i2cBaudrate;
            }
            else if(m_currentGlobalSettings.aardvarkI2cSpi.deviceMode == AARDVARK_I2C_SPI_DEVICE_MODE_SPI_MASTER)
            {
                mode = "spi master";
                baudrateSetValue = m_currentGlobalSettings.aardvarkI2cSpi.spiBaudrate;
            }
            else if(m_currentGlobalSettings.aardvarkI2cSpi.deviceMode == AARDVARK_I2C_SPI_DEVICE_MODE_SPI_SLAVE)
            {
                mode = "spi slave";
                baudrateSetValue = m_currentGlobalSettings.aardvarkI2cSpi.spiBaudrate;
            }
            else
            {
                mode = "gpio";
            }

            if(m_isConnected)
            {
                QString message = QString("Connected to aardvark interface: port=%1, mode=%2").arg(m_currentGlobalSettings.aardvarkI2cSpi.devicePort).arg(mode);
                if(m_currentGlobalSettings.aardvarkI2cSpi.deviceMode != AARDVARK_I2C_SPI_DEVICE_MODE_GPIO)
                {
                    message += QString(", baudrate=%1 (kHz)").arg(deviceBitrate);
                }
                emit dataConnectionStatusSignal(true, message, false);

            }
            else
            {
                QString message = QString("could not open aardvark device: port=%1, mode=%2").arg(m_currentGlobalSettings.aardvarkI2cSpi.devicePort).arg(mode);
                if(m_currentGlobalSettings.aardvarkI2cSpi.deviceMode != AARDVARK_I2C_SPI_DEVICE_MODE_GPIO)
                {
                    message += QString(", baudrate=%1 (kHz)").arg(baudrateSetValue);
                }

                showMessageBox(QMessageBox::Critical, "aardvark error",message);
                m_isConnected = false;
                emit dataConnectionStatusSignal(false, "open error", false);
                emit showAdditionalConnectionInformationSignal("");
            }

        }
        else if(m_currentGlobalSettings.connectionType == CONNECTION_TYPE_PCAN)
        {

            bool isOk;
            m_isConnected = m_pcanInterface->open(m_currentGlobalSettings.pcanInterface.channel, m_currentGlobalSettings.pcanInterface.baudRate,
                                                  m_currentGlobalSettings.pcanInterface.busOffAutoReset, m_currentGlobalSettings.pcanInterface.powerSupply);

            if(m_isConnected)
            {
                if(!m_pcanInterface->setFilter(m_currentGlobalSettings.pcanInterface.filterExtended, m_currentGlobalSettings.pcanInterface.filterFrom.toUInt(&isOk, 16),
                                           m_currentGlobalSettings.pcanInterface.filterTo.toUInt(&isOk, 16)))
                {
                    m_pcanInterface->close();
                    m_isConnected = false;
                }
            }
            if(m_isConnected)
            {
                emit dataConnectionStatusSignal(true, tr("Connected to pcan %1: baud.=%2 (kHz), 5V=%3, reset=%4, filter=%5 %6-%7")
                                                .arg(m_currentGlobalSettings.pcanInterface.channel)
                                                .arg(SettingsDialog::convertPcanBaudrate(m_currentGlobalSettings.pcanInterface.baudRate))
                                                .arg(m_currentGlobalSettings.pcanInterface.powerSupply)
                                                .arg(m_currentGlobalSettings.pcanInterface.busOffAutoReset)
                                                .arg(m_currentGlobalSettings.pcanInterface.filterExtended ? "ext" : "std")
                                                .arg(m_currentGlobalSettings.pcanInterface.filterFrom)
                                                .arg(m_currentGlobalSettings.pcanInterface.filterTo), false);

            }
            else
            {
 #if defined(WIN32) || defined(_WIN32)

                if(m_currentGlobalSettings.pcanInterface.channel != 0)
                {
                    showMessageBox(QMessageBox::Critical, tr("pcan error"),
                                              tr("could not open pcan device: channel=%1, baudarte=%2 (kHz), power supply=%3, auto reset=%4")
                                              .arg(m_currentGlobalSettings.pcanInterface.channel)
                                              .arg(SettingsDialog::convertPcanBaudrate(m_currentGlobalSettings.pcanInterface.baudRate))
                                              .arg(m_currentGlobalSettings.pcanInterface.powerSupply)
                                              .arg(m_currentGlobalSettings.pcanInterface.busOffAutoReset));
                }
                else
                {
                    showMessageBox(QMessageBox::Critical, tr("pcan open error"), "invalid pcan channel");
                }
#else
                showMessageBox(QMessageBox::Critical, tr("pcan open error"), "pcan is only available on windows");
#endif
                m_isConnected = false;
                emit dataConnectionStatusSignal(false, tr("open error"), false);
                emit showAdditionalConnectionInformationSignal("");
            }

        }
        else
        {
            m_isConnected = false;
            emit dataConnectionStatusSignal(false, tr("Disconnected"), false);
            emit showAdditionalConnectionInformationSignal("");
            showMessageBox(QMessageBox::Critical, tr("Error"), "invalid connection type");
        }
    }//if(connect)
    else
    {
        m_isConnected = false;
        emit dataConnectionStatusSignal(false, tr("Disconnected"), false);

        if(m_currentGlobalSettings.connectionType == CONNECTION_TYPE_SERIAL_PORT)
        {
            emit showAdditionalConnectionInformationSignal("CTS=0, DSR=0, DCD=0, RI=0");
        }
        else
        {
            emit showAdditionalConnectionInformationSignal("");
        }

        if((m_currentGlobalSettings.connectionType == CONNECTION_TYPE_TCP_CLIENT) ||
                (m_currentGlobalSettings.connectionType == CONNECTION_TYPE_TCP_SERVER) ||
                (m_currentGlobalSettings.connectionType == CONNECTION_TYPE_UDP_SOCKET))
        {
            QNetworkProxyFactory::setUseSystemConfiguration(false);
            QNetworkProxy::setApplicationProxy(QNetworkProxy::NoProxy);


        }
    }


}

/**
 * Shows a message box.
 * (emits the showMessageBoxSignal signal).
 * @param icon
 *      The icon if the message box.
 * @param title
 *      The title of the message box.
 * @param text
 *      The text of the message box.
 */
void MainInterfaceThread::showMessageBox(QMessageBox::Icon icon, QString title, QString text)
{

    if(m_showMessageBoxOnError)
    {
        emit disableMouseEventsSignal();
        emit enableMouseEventsSignal();

        emit showMessageBoxSignal(icon, title, text, QMessageBox::Ok);
    }
}

/**
 * Sends data with the main interface.
 * @param data
 *      The data.
 * @param waitForSendingFinished
 *      If true, the this function blocks until the sending has been finished.
 * param serialPortSignalBlocked
 *      True if during waitForBytesWritten the serial port receive signal has been blocked.
 * @return
 *      True for success.
 */
bool MainInterfaceThread::sendDataWithTheMainInterface(const QByteArray &data, QByteArray& receivedData,
                bool waitForSendingFinished, bool* serialPortSignalBlocked)
{
    bool success = true;
    *serialPortSignalBlocked = false;


    if(m_isConnected)
    {
        if(m_currentGlobalSettings.connectionType == CONNECTION_TYPE_SERIAL_PORT)
        {
            for(int i = 0; i < data.length(); i += 512)
            {
                QByteArray subArray = data.mid(i, 512);

                if(subArray.size() != m_serial->write(subArray))
                {
                    success = false;
                }
                else
                {
                    if(waitForSendingFinished)
                    {
                        //Block the signals (to prevent that serialPortReceivedDataSlot
                        //is called before dataHasBeenSendSignal).
                        m_serial->blockSignals(true);
                        *serialPortSignalBlocked = true;

                        if(!m_serial->waitForBytesWritten(SEND_TIMEOUT))
                        {
                            success = false;
                        }

                        m_serial->blockSignals(false);
                    }
                }

                if(!success)
                {
                    break;
                }
            }
        }
        else if(m_currentGlobalSettings.connectionType == CONNECTION_TYPE_TCP_CLIENT)
        {
            for(int i = 0; i < data.length(); i += 50000)
            {
                QByteArray subArray = data.mid(i, 50000);
                if(subArray.size() != m_tcpClientSocket->write(subArray))
                {
                    success = false;
                    break;
                }
                (void)m_tcpClientSocket->waitForBytesWritten();
            }
        }
        else if(m_currentGlobalSettings.connectionType == CONNECTION_TYPE_TCP_SERVER)
        {
            for(int i = 0; i < data.length(); i += 50000)
            {
                QByteArray subArray = data.mid(i, 50000);
                for(int i = 0; i < m_tcpServerSockets.length(); i++)
                {
                    if(subArray.size() != m_tcpServerSockets[i]->write(subArray))
                    {
                        success = false;
                        break;
                    }
                    (void)m_tcpServerSockets[i]->waitForBytesWritten();
                }
            }

        }//
        else if(m_currentGlobalSettings.connectionType == CONNECTION_TYPE_UDP_SOCKET)
        {
            const char* constData = (const char *)data.constData();
            for(int i = 0; i < data.length(); i += MainInterfaceThread::UDP_MAX_SEND_SIZE)
            {
                qint64 bytesToWrite = ((i + MainInterfaceThread::UDP_MAX_SEND_SIZE) < data.length()) ? MainInterfaceThread::UDP_MAX_SEND_SIZE :
                                                                                                       data.length() - i;

                qint64 tmp = m_udpServerSocket->writeDatagram(&constData[i], bytesToWrite, QHostAddress(m_currentGlobalSettings.socketSettings.destinationIpAddress),
                                                              m_currentGlobalSettings.socketSettings.destinationPort);
                if(bytesToWrite != tmp)
                {
                    success = false;
                    break;
                }
            }
        }
        else if(m_currentGlobalSettings.connectionType == CONNECTION_TYPE_AARDVARK)
        {
            if(!m_aardvarkI2cSpi->sendReceiveData(data,&receivedData))
            {
                success = false;
                receivedData.clear();
            }
        }
        else if(m_currentGlobalSettings.connectionType == CONNECTION_TYPE_PCAN)
        {
            success = m_pcanInterface->sendData(data);

            if(!success)
            {
                m_pcanInterface->close();
                connectDataConnectionSlot(m_currentGlobalSettings, true, true);
                emit showAdditionalConnectionInformationSignal("Bus off event occurred (interface has been restartet)");
            }
        }
        else
        {
            success = false;
        }

    }
    else
    {
        success = false;
    }

    return success;
}

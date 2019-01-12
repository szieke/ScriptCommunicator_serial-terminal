#include "scriptInf.h"
#include "scriptAardvarkI2cSpi.h"
#include "scriptThread.h"
#include "scriptwindow.h"
#include "scriptSerialPort.h"
#include "scriptTcpClient.h"
#include "scriptTcpServer.h"
#include "scriptUdpSocket.h"
#include <QSerialPortInfo>
#include "scriptPcan.h"
#include <QNetworkInterface>



ScriptInf::ScriptInf(ScriptThread *scriptThread, SettingsDialog *settingsDialog) : QObject(scriptThread), m_scriptThread(scriptThread),
    m_sendingSucceeded(false), m_isConnected(false), m_isConnectedWithCan(false),
    m_isConnectedWithI2c(false), m_isConnectedWithI2cMaster(false), m_settingsDialog(settingsDialog),
    m_debugReceiveTimer(), m_savedReceivedData()
{
    //Get the connection state of the main interface.
    m_isConnected = m_scriptThread->getScriptWindow()->getMainInterfaceThread()->isConnected();
    m_isConnectedWithCan = m_scriptThread->getScriptWindow()->getMainInterfaceThread()->isConnectedWithCan();
    m_isConnectedWithI2c = m_scriptThread->getScriptWindow()->getMainInterfaceThread()->isConnectedWithI2c();
    m_isConnectedWithI2cMaster = m_scriptThread->getScriptWindow()->getMainInterfaceThread()->isConnectedWithI2cMaster();
}


void ScriptInf::disconnectDataSignals(void)
{
    //Disconnect all signals which are routed to the current script.
    QObject::disconnect(m_scriptThread->getScriptWindow()->getMainInterfaceThread(), SIGNAL(dataReceivedSignal(QByteArray)),
                    this, SLOT(dataReceivedSlot(QByteArray)));
    QObject::disconnect(m_scriptThread->getScriptWindow()->getMainInterfaceThread(), SIGNAL(canMessagesReceivedSignal(QVector<QByteArray>)),
                    this, SLOT(canMessagesReceivedSlot(QVector<QByteArray>)));
    QObject::disconnect(m_scriptThread->getScriptWindow()->getMainInterfaceThread(), SIGNAL(dataConnectionStatusSignal(bool, QString)),
                    this, SLOT(dataConnectionStatusSlot(bool, QString)));
    QObject::disconnect(m_scriptThread->getScriptWindow()->getMainInterfaceThread(), SIGNAL(sendingFinishedSignal(bool,uint)),
                    this, SLOT(sendingFinishedSlot(bool,uint)));
}

/**
 * Connects all signals.
 * @param scriptWindow
 *      Pointer to the script window.
 */
void ScriptInf::intSignals(bool runsInDebugger)
{
    Qt::ConnectionType directConnectionType = runsInDebugger ? Qt::DirectConnection : Qt::BlockingQueuedConnection;


    connect(this, SIGNAL(addDataToMainWindowSendHistorySignal(QByteArray)),
            m_scriptThread->getScriptWindow()->getMainWindow(), SLOT(addDataToMainWindowSendHistorySlot(QByteArray)), Qt::QueuedConnection);

    connect(this, SIGNAL(sendDataSignal(const QByteArray, uint)),
            m_scriptThread->getScriptWindow()->getMainInterfaceThread(), SLOT(sendDataSlot(const QByteArray, uint)), Qt::BlockingQueuedConnection);

    connect(this, SIGNAL(ardvarkI2cSpiReadAllInputsSignal(QVector<bool>&)),
            m_scriptThread->getScriptWindow()->getMainInterfaceThread()->m_aardvarkI2cSpi, SLOT(readAllInputs(QVector<bool>&)), Qt::BlockingQueuedConnection);

    connect(this, SIGNAL(i2cMasterFreeBusSignal()),
            m_scriptThread->getScriptWindow()->getMainInterfaceThread()->m_aardvarkI2cSpi, SLOT(freeI2cBusSlot()), Qt::QueuedConnection);

    connect(m_scriptThread->getScriptWindow()->getMainInterfaceThread(), SIGNAL(dataConnectionStatusSignal(bool, QString, bool)),
            this, SLOT(dataConnectionStatusSlot(bool,QString,bool)), Qt::DirectConnection);

    connect(this, SIGNAL(connectDataConnectionSignal(Settings, bool)),m_scriptThread->getScriptWindow()->getMainInterfaceThread(),
            SLOT(connectDataConnectionSlot(Settings, bool)), Qt::BlockingQueuedConnection);

    connect(this, SIGNAL(setSerialPortPinsSignal(bool,bool)),
            m_scriptThread->getScriptWindow()->getMainWindow(), SLOT(setSerialPortPinsSlot(bool,bool)), directConnectionType);

    connect(this, SIGNAL(getSerialPortSignalsSignal(uint32_t*)),
            m_scriptThread->getScriptWindow()->getMainInterfaceThread(), SLOT(getSerialPortSignals(uint32_t*)), directConnectionType);

    connect(m_scriptThread->getScriptWindow()->getMainInterfaceThread()->m_aardvarkI2cSpi, SIGNAL(inputStatesChangedSignal(QVector<bool>)),
            this, SLOT(aardvarkI2cSpiInputStatesChangedSlot(QVector<bool>)), Qt::QueuedConnection);

    connect(m_scriptThread->getScriptWindow()->getMainInterfaceThread(), SIGNAL(sendDataWithWorkerScriptsSignal(QByteArray)),
            this, SLOT(sendDataFromMainInterfaceSlot(QByteArray)), Qt::QueuedConnection);

    connect(m_scriptThread->getScriptWindow()->getMainInterfaceThread(), SIGNAL(dataReceivedSignal(QByteArray)),
            this, SLOT(dataReceivedSlot(QByteArray)), Qt::QueuedConnection);

    connect(m_scriptThread->getScriptWindow()->getMainInterfaceThread(), SIGNAL(canMessagesReceivedSignal(QVector<QByteArray>)),
            this, SLOT(canMessagesReceivedSlot(QVector<QByteArray>)), Qt::QueuedConnection);

    connect(m_scriptThread->getScriptWindow()->getMainInterfaceThread(), SIGNAL(sendingFinishedSignal(bool,uint)),
            this, SLOT(sendingFinishedSlot(bool,uint)), Qt::DirectConnection);

    connect(this, SIGNAL(setAllSettingsSignal(Settings&,bool)),
            m_settingsDialog, SLOT(setAllSettingsSlot(Settings&,bool)), directConnectionType);

    connect(this, SIGNAL(setAardvarkI2cSpiOutputSignal(AardvarkI2cSpiSettings)),
            m_scriptThread->getScriptWindow()->getMainInterfaceThread()->m_aardvarkI2cSpi, SLOT(outputValueChangedSlot(AardvarkI2cSpiSettings)), Qt::QueuedConnection);

    connect(this, SIGNAL(changeAardvarkI2cSpiPinConfigurationSignal(AardvarkI2cSpiSettings)),
            m_scriptThread->getScriptWindow()->getMainInterfaceThread()->m_aardvarkI2cSpi, SLOT(pinConfigChangedSlot(AardvarkI2cSpiSettings)), Qt::QueuedConnection);

    connect(m_scriptThread->getScriptWindow()->getMainInterfaceThread(), SIGNAL(slaveDataSentSignal(QByteArray)),
            this, SLOT(slaveDataSentSlot(QByteArray)), Qt::QueuedConnection);

    if(m_scriptThread->runsInDebugger())
    {
        connect(&m_debugReceiveTimer, SIGNAL(timeout()),this, SLOT(debugReceiveTimerSlot()));
        m_debugReceiveTimer.setInterval(20);
    }
}


/**
 * Creates a TCP socket.
 * @return
 *      The created socket.
 */
QScriptValue ScriptInf::createTcpClient(void)
{
    ScriptTcpClient* socket =  new ScriptTcpClient(new QTcpSocket(m_scriptThread), m_scriptThread, m_scriptThread->getScriptWindow()->getMainInterfaceThread());
    return m_scriptThread->getScriptEngine()->newQObject(socket, QScriptEngine::ScriptOwnership);
}

/**
 * Creates an UDP socket.
 * @return
 *      The created socket.
 */
QScriptValue ScriptInf::createUdpSocket(void)
{
    ScriptUdpSocket* socket =  new ScriptUdpSocket(m_scriptThread, m_scriptThread->getScriptWindow()->getMainInterfaceThread());
    return m_scriptThread->getScriptEngine()->newQObject(socket, QScriptEngine::ScriptOwnership);
}

/**
 * Creates a TCP server.
 * @return
 *      The created server.
 */
QScriptValue ScriptInf::createTcpServer(void)
{
    ScriptTcpServer* server =  new ScriptTcpServer(m_scriptThread, m_scriptThread->getScriptWindow()->getMainInterfaceThread());
    return m_scriptThread->getScriptEngine()->newQObject(server, QScriptEngine::ScriptOwnership);
}

/**
 * Creates a serial port.
 * @return
 *      The created serial port.
 */
QScriptValue ScriptInf::createSerialPort(void)
{
    ScriptSerialPort* serialPort =  new ScriptSerialPort(m_scriptThread, m_scriptThread->getScriptWindow()->getMainInterfaceThread());
    return m_scriptThread->getScriptEngine()->newQObject(serialPort, QScriptEngine::ScriptOwnership);
}


/**
 * Creates an Aardvark I2c/SPI interface.
 * @return
 *      The created interface.
 */
QScriptValue ScriptInf::aardvarkI2cSpiCreateInterface(void)
{
    ScriptAardvarkI2cSpi* aardvarkInterface = new ScriptAardvarkI2cSpi(m_scriptThread);
    return m_scriptThread->getScriptEngine()->newQObject(aardvarkInterface, QScriptEngine::ScriptOwnership);
}

/**
 * Creates a pcan interface.
 * @return
 *      The created pcan interface.
 */
QScriptValue ScriptInf::createPcanInterface(void)
{
    ScriptPcan* pcan = new ScriptPcan(m_scriptThread);
    return m_scriptThread->getScriptEngine()->newQObject(pcan, QScriptEngine::ScriptOwnership);

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
bool ScriptInf::sendByteArray(QByteArray byteArray, int repetitionCount, int pause, bool addToMainWindowSendHistory)
{

    bool hasSucceeded = false;

    for(qint32 i = 0; i <= repetitionCount; i++)
    {
        m_sendingSucceeded = false;

        if(!m_isConnected || m_scriptThread->scriptShallExit())
        {//The main interface is not connected or the script thread shall exit.
            break;
        }

        //send the data.
        emit sendDataSignal(byteArray, m_scriptThread->getSendId());
        hasSucceeded =  m_sendingSucceeded;

        if(!hasSucceeded)
        {
            break;
        }

        if(repetitionCount > 0)
        {
            QCoreApplication::processEvents();
            if(m_scriptThread->getShallPause())
            {
                m_scriptThread->pauseTimerSlot();
            }
            QThread::msleep(pause);
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
bool ScriptInf::sendDataArray(QVector<unsigned char> data, int repetitionCount, int pause, bool addToMainWindowSendHistory)
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
bool ScriptInf::sendCanMessage(quint8 type, quint32 canId, QVector<unsigned char> data, int repetitionCount, int pause, bool addToMainWindowSendHistory)
{
    bool result = false;

    if(m_isConnectedWithCan)
    {
        QByteArray byteArray;
        byteArray.push_back(type);

        byteArray.push_back((canId >> 24) & 0xff);
        byteArray.push_back((canId >> 16) & 0xff);
        byteArray.push_back((canId >> 8) & 0xff);
        byteArray.push_back(canId & 0xff);

        byteArray.append(QByteArray(reinterpret_cast<const char*>(data.constData()), data.size()));

        result = sendByteArray(byteArray, repetitionCount, pause, addToMainWindowSendHistory);
    }

    return result;
}

/**
 * Accesses the I2C bus (write/read).
 * Note: This functions works only if the main interface is an I2C master.
 * To receive data in master mode the signal i2cMasterDataReceivedSignal must be used.
 *
 * @param flags
 *      The I2C flags:
 *      - 0x00= no flags,
 *      - 0x01= 10bit address
 *      - 0x02=combined FMT
 *      - 0x04= no stop condition (use i2cMasterFreeBus to generate the stop condition later)
 * @param slaveAddress
 *      The slave address.
 * @param numberOfBytesToRead
 *      The number of bytes wich shall be read.
 * @param dataToSend
 *      The bytes which shall be send/written (if this array is empty only a read is performed).
 *  @param repetitionCount
 *      The I2C access is repeated until the number has been reached.
 *  @param pause
 *      The pause (ms) between two repetitions.
 *  @param addToMainWindowSendHistory
 *      True if the data shall be added to the send history in the main window.
 * @return
 *      True for success.
 */
bool ScriptInf::i2cMasterReadWrite(quint8 flags, quint16 slaveAddress, quint16 numberOfBytesToRead, QVector<unsigned char> dataToSend,
                                   int repetitionCount, int pause, bool addToMainWindowSendHistory)
{
    bool result = false;

    if(m_isConnectedWithI2cMaster)
    {
        QByteArray byteArray;
        byteArray.push_back(flags);

        byteArray.push_back((slaveAddress >> 8) & 0xff);
        byteArray.push_back(slaveAddress & 0xff);

        byteArray.push_back((numberOfBytesToRead >> 8) & 0xff);
        byteArray.push_back(numberOfBytesToRead & 0xff);

        if(!dataToSend.isEmpty())
        {
            byteArray.append(QByteArray(reinterpret_cast<const char*>(dataToSend.constData()), dataToSend.size()));
        }

        result =  sendByteArray(byteArray, repetitionCount, pause, addToMainWindowSendHistory);
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
bool ScriptInf::sendString(QString string, int repetitionCount, int pause, bool addToMainWindowSendHistory)
{
    return sendByteArray(string.toLocal8Bit(), repetitionCount, pause, addToMainWindowSendHistory);
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
bool ScriptInf::connectPcan(quint8 channel, quint32 baudrate, quint32 connectTimeout, bool busOffAutoReset, bool powerSupply,
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
 * Connects the main interface (Aardvark I2C/SPI).
 *
 * @param aardvarkI2cSpiSettings
 *      The new settings (AardvarkI2cSpiSettings structure).
 * @param connectTimeout
 *      Connect timeout(ms)
 * @return
 *      True on success.
 */
bool ScriptInf::aardvarkI2cSpiConnect(QScriptValue aardvarkI2cSpiSettings, quint32 connectTimeout)
{
    bool succeeded = false;

    m_settingsDialog->updateSettings();
    Settings oldSettings = *m_settingsDialog->settings();
    Settings settings = *m_settingsDialog->settings();
    settings.connectionType = CONNECTION_TYPE_AARDVARK;


    succeeded = ScriptAardvarkI2cSpi::scriptValueToConfig(aardvarkI2cSpiSettings, settings.aardvarkI2cSpi,
                                                            m_scriptThread, "aardvarkI2cSpiConnect");

    if(succeeded)
    {
        emit setAllSettingsSignal(settings, false);
        emit connectDataConnectionSignal(settings, true);

        waitForMainInterfaceToConnect(connectTimeout);

        if(!m_isConnected)
        {
            emit setAllSettingsSignal(oldSettings, false);
            emit connectDataConnectionSignal(oldSettings, false);
        }

        succeeded = m_isConnected;
    }
    return succeeded;


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
bool ScriptInf::connectSerialPort(QString name, qint32 baudRate, quint32 connectTimeout, quint32 dataBits, QString parity, QString stopBits, QString flowControl)
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
bool ScriptInf::connectSocket(bool isTcp, bool isServer, QString ip, quint32 destinationPort, quint32 ownPort, quint32 connectTimeout)
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
 * Sets the value of an output pin (Aardvark I2C/SPI device).
 *
 * @param pinIndex
 *      The index of the pin:
 *      - 0=Pin1/SCL
 *      - 1=Pin3/SDA
 *      - 2=Pin5/MISO
 *      - 3=Pin7/SCK
 *      - 4=Pin8/MOSI
 *      - 5=Pin9/SS0
 * @param high
 *      True for 1 and false for 0.
 * @param updateSettingsDialog
 *      True if the new output value should be displayed in the the settings dialog.
 * return
 *      True on success.
 */
bool ScriptInf::aardvarkI2cSpiSetOutput(quint8 pinIndex, bool high, bool updateSettingsDialog)
{
    bool result = false;
    Settings settings = *m_settingsDialog->settings();

    if(pinIndex < AARDVARK_I2C_SPI_GPIO_COUNT)
    {
        result = true;
        settings.aardvarkI2cSpi.pinConfigs[pinIndex].outValue = high;
        emit setAardvarkI2cSpiOutputSignal(settings.aardvarkI2cSpi);

        if(updateSettingsDialog)
        {
            emit setAllSettingsSignal(settings, false);
        }
    }

    return result;
}

/**
 * Changes the configuration of a pin (Aardvark I2C/SPI device).
 *
 * @param pinIndex
 *      The index of the pin:
 *      - 0=Pin1/SCL
 *      - 1=Pin3/SDA
 *      - 2=Pin5/MISO
 *      - 3=Pin7/SCK
 *      - 4=Pin8/MOSI
 *      - 5=Pin9/SS0
 *
 * @param isInput
 *      True if the pin shall be configured as input.
 * @param withPullups
 *      True of the input pin shall have a pullup (not possible with output).
 * return
 *      True on success.
 */
bool ScriptInf::aardvarkI2cSpiChangePinConfiguration(quint8 pinIndex, bool isInput, bool withPullups)
{
    bool result = false;
    Settings settings = *m_settingsDialog->settings();

    if(pinIndex < AARDVARK_I2C_SPI_GPIO_COUNT)
    {
        result = true;
        settings.aardvarkI2cSpi.pinConfigs[pinIndex].outValue = 0;
        settings.aardvarkI2cSpi.pinConfigs[pinIndex].isInput = isInput;
        settings.aardvarkI2cSpi.pinConfigs[pinIndex].withPullups = withPullups;
        emit changeAardvarkI2cSpiPinConfigurationSignal(settings.aardvarkI2cSpi);
        emit setAllSettingsSignal(settings, false);
    }

    return result;
}


/**
 * Reads all inputs of the Aardvark I2C/SPI device.
 *
 * @return
 *      The read values (true=1, false= 0). The indexes of the result array are:
 *      - 0=Pin1/SCL
 *      - 1=Pin3/SDA
 *      - 2=Pin5/MISO
 *      - 3=Pin7/SCK
 *      - 4=Pin8/MOSI
 *      - 5=Pin9/SS0
 */
QVector<bool> ScriptInf::aardvarkI2cSpiReadAllInputs(void)
{
    QVector<bool> readValues;
    emit ardvarkI2cSpiReadAllInputsSignal(readValues);
    return readValues;
}
/**
 * Returns the Aardvark I2C/SPI settings of the main interface.
 * @return
 *      The settings.
 */
QScriptValue ScriptInf::aardvarkI2cSpiGetMainInterfaceSettings(void)
{
    const Settings* settings = m_settingsDialog->settings();
    return ScriptAardvarkI2cSpi::convertConfigToScriptValue(&settings->aardvarkI2cSpi, m_scriptThread);
}

/**
 * Returns the serial port settings of the main interface.
 * @return
 *      The serial port settings.
 */
QScriptValue ScriptInf::getMainInterfaceSerialPortSettings(void)
{
    const Settings* settings = m_settingsDialog->settings();
    QScriptValue ret = m_scriptThread->getScriptEngine()->newObject();

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
QScriptValue ScriptInf::getMainInterfaceSocketSettings(void)
{
    const Settings* settings = m_settingsDialog->settings();
    QScriptValue ret = m_scriptThread->getScriptEngine()->newObject();

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
 * Waits until the main interface is connected or a timeout occurs.
 *
 * @param connectTimeout
 *      Connect timeout(ms).
 */
void ScriptInf::waitForMainInterfaceToConnect(quint32 connectTimeout)
{
    QDateTime startTime = QDateTime::currentDateTime();
    do
    {
        QThread::msleep(1);
        if(m_scriptThread->getShallPause())
        {
            m_scriptThread->pauseTimerSlot();
        }
        QCoreApplication::processEvents();


    }while ((startTime.msecsTo(QDateTime::currentDateTime()) < (qint64)connectTimeout)  && !m_isConnected  && !m_scriptThread->scriptShallExit());
}
/**
 * Returns all IP addresses found on the host machine.
 * @return
 *  The ip addresses.
 */
QStringList ScriptInf::getLocalIpAdress(void)
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
 * This slot is connected with MainInterfaceThread::dataConnectionStatusSignal.
 * The connected status (main interface) is reported with this signal.
 * @param isConnected
 *      True for connected.
 * @param message
 *      String with additional information.
 * @param isWaiting
 *      True if the interface is waiting for a client/connection
 */
void ScriptInf::dataConnectionStatusSlot(bool isConnected, QString message, bool isWaiting)
{
    (void)message;
    (void)isWaiting;
    (void)isConnected;
    m_isConnected = m_scriptThread->getScriptWindow()->getMainInterfaceThread()->isConnected();
    m_isConnectedWithCan = m_scriptThread->getScriptWindow()->getMainInterfaceThread()->isConnectedWithCan();
    m_isConnectedWithI2c = m_scriptThread->getScriptWindow()->getMainInterfaceThread()->isConnectedWithI2c();
    m_isConnectedWithI2cMaster = m_scriptThread->getScriptWindow()->getMainInterfaceThread()->isConnectedWithI2cMaster();

}

/**
 * Sends the send data from the main interface.
 * @param data
 *      The data.
 */
void ScriptInf::sendDataFromMainInterfaceSlot(const QByteArray data)
{

    if(m_scriptThread->getThreadState() == RUNNING)
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
 * The slot is called if the main interface thread has received data.
 * This slot is connected to the MainInterfaceThread::dataReceivedSignal signal.
 * @param data
 *      The received data.
 */
void ScriptInf::canMessagesReceivedSlot(QVector<QByteArray> messages)
{

    if(!messages.isEmpty() && (m_scriptThread->getThreadState() == RUNNING))
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
 * Is called, if data from the main interface (MainInterfaceThread) has been received.
 * It converts the received QByteArray into a QVector and emits the dataReceivedSignal.
 * The script can connect to this signal.
 * @param data
 *      The received data.
 */
void ScriptInf::dataReceivedSlot(QByteArray data)
{
    if(m_scriptThread->getThreadState()  == RUNNING)
    {
        if(!m_isConnectedWithI2cMaster)
        {
            if((QObject::receivers(SIGNAL(dataReceivedSignal(QVector<unsigned char>))) > 0) ||
               (m_scriptThread->dataReceivedSignalIsConnected()) )
            {
                QVector<unsigned char> dataVector;
                for(auto val : data)
                {
                    dataVector.push_back((unsigned char) val);
                }
                if(m_scriptThread->runsInDebugger())
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
                    emit m_scriptThread->dataReceivedSignal(dataVector);
                }
            }
        }
        else
        {//Connected with an I2C master interface.

            if(QObject::receivers(SIGNAL(i2cMasterDataReceivedSignal(quint8, quint16, QVector<unsigned char>))) > 0)
            {
                quint8 flags = (quint8)data[0];
                quint16 address = (quint16)data[2] + ((quint16)data[1] << 8);
                data.remove(0, AardvarkI2cSpi::RECEIVE_CONTROL_BYTES_COUNT);

                QVector<unsigned char> dataVector;
                for(auto val : data)
                {
                    dataVector.push_back((unsigned char) val);
                }

                emit i2cMasterDataReceivedSignal(flags, address, dataVector);
            }
        }
    }

}

/**
 * Emits the dataReceivedSignal with the saved received data (if the script is running in the script debugger).
 */
void ScriptInf::debugReceiveTimerSlot(void)
{
    m_debugReceiveTimer.stop();
    emit dataReceivedSignal(m_savedReceivedData);
    emit m_scriptThread->dataReceivedSignal(m_savedReceivedData);
    m_savedReceivedData.clear();
}

/**
 * This slot is connected with the MainInterfaceThread::sendingFinishedSignal signal.
 * The main interface thread emits this signal if the sending of data has been finished.
 * @param id
 *      The send id (identifies the sender).
 */
void ScriptInf::sendingFinishedSlot(bool success, uint id)
{
    if(id == m_scriptThread->getSendId())
    {
        m_sendingSucceeded = success;
    }
}

/**
 * Returns a list with the name of all available serial ports.
 * @return
 *      The list.
 */
QStringList ScriptInf::availableSerialPorts(void)
{
    QStringList result;
    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
    {
        result << info.portName();
    }
    return result;
}

/**
 * Is called if the main interface is a I2C or SPI slave and has sent data.
 *
 * @param data
 *      The sent data.
 */
void ScriptInf::slaveDataSentSlot(QByteArray data)
{
    if(QObject::receivers(SIGNAL(slaveDataSentSignal(QVector<unsigned char>))) > 0)
    {
        QVector<unsigned char> dataVector;
        for(auto val : data)
        {
            dataVector.push_back((unsigned char) val);
        }

        emit slaveDataSentSignal(dataVector);
    }
}


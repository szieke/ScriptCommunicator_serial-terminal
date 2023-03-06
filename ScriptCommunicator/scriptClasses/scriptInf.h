#ifndef SCRIPT_INF_H
#define SCRIPT_INF_H

#include <QObject>
#include "scriptObject.h"
#include "settingsdialog.h"


class ScriptThread;


class ScriptInf : public QObject, public ScriptObject
{
    Q_OBJECT

    ///Returns a semicolon separated list with all public functions, signals and properties.
    Q_PROPERTY(QString publicScriptElements READ getPublicScriptElements)

public:
    ScriptInf(ScriptThread* scriptThread, SettingsDialog *settingsDialog);

    ///Connects all signals.
    void intSignals(bool runsInDebugger);

    void disconnectDataSignals(void);

    ///Returns a semicolon separated list with all public functions, signals and properties.
    virtual QString getPublicScriptElements(void){return MainWindow::parseApiFile("scriptInf.api");}

    ///Creates an UDP socket.
    Q_INVOKABLE QJSValue createUdpSocket(void);

    ///Creates a TCP server.
    Q_INVOKABLE QJSValue createTcpServer(void);

    ///Creates a TCP socket.
    Q_INVOKABLE QJSValue createTcpClient(void);

    ///Creates a serial port.
    Q_INVOKABLE QJSValue createSerialPort(void);

    ///Creates an Aardvark I2c/SPI interface.
    Q_INVOKABLE QJSValue aardvarkI2cSpiCreateInterface(void);

    ///Creates a pcan interface.
    Q_INVOKABLE QJSValue createPcanInterface(void);

    ///Sends a data array (QVector) with the main interface (in MainInterfaceThread).
    Q_INVOKABLE bool sendDataArray(QVector<unsigned char> data, int repetitionCount=0, int pause=0, bool addToMainWindowSendHistory=false);

    ///Sends a can message with the main interface (in MainInterfaceThread).
    ///If more then 8 data bytes are given several can messages with the same can id will be sent.
    Q_INVOKABLE bool sendCanMessage(quint8 type, quint32 canId, QVector<unsigned char> data, int repetitionCount=0, int pause=0, bool addToMainWindowSendHistory=false);

    ///Accesses the I2C bus (write/read).
    ///Note: This functions works only if the main interface is an I2C bus (master mode).
    Q_INVOKABLE bool i2cMasterReadWrite(quint8 flags, quint16 slaveAddress, quint16 numberOfBytesToRead, QVector<unsigned char> dataToSend = QVector<unsigned char>(),
                                     int repetitionCount=0, int pause=0, bool addToMainWindowSendHistory=false);

    ///Frees the main interface I2C bus (this function can be used if the no stop condition was created
    ///during the last i2cMasterReadWrite call)
    Q_INVOKABLE void i2cMasterFreeBus(void){emit i2cMasterFreeBusSignal();}

    ///Sends a string (QString) with the main interface (in MainInterfaceThread).
    Q_INVOKABLE bool sendString(QString string, int repetitionCount=0, int pause=0, bool addToMainWindowSendHistory=false);

    ///Returns true if the main interface is connected.
    Q_INVOKABLE bool isConnected(void){return m_isConnected;}

    ///Returns true if the main interface is a CAN interface (and is connected).
    Q_INVOKABLE bool isConnectedWithCan(void){return m_isConnectedWithCan;}

    ///Returns true if the main interface is a I2C interface (and is connected).
    Q_INVOKABLE bool isConnectedWithI2c(void){return m_isConnectedWithI2c;}

    ///Disconnects the main interface.
    Q_INVOKABLE void disconnect(void){emit connectDataConnectionSignal(*m_settingsDialog->settings(), false, false);}

    ///Connects the main interface (PCAN).
    ///Note: A successful call will modify the corresponding settings in the settings dialog.
    Q_INVOKABLE bool connectPcan(quint8 channel, quint32 baudrate, quint32 connectTimeout = 2000, bool busOffAutoReset = true, bool powerSupply = false,
                                 bool filterExtended = true, quint32 filterFrom = 0, quint32 filterTo = 0x1fffffff);

    ///Sets the serial port (main interface) RTS and DTR pins.
    Q_INVOKABLE void setSerialPortPins(bool setRTS, bool setDTR){emit setSerialPortPinsSignal(setRTS, setDTR);}


    ///Returns the state of the serial port signals (pins).
    ///The signals are bit coded:
    ///NoSignal = 0x00,
    ///DataTerminalReadySignal = 0x04,
    ///DataCarrierDetectSignal = 0x08,
    ///DataSetReadySignal = 0x10,
    ///RingIndicatorSignal = 0x20,
    ///RequestToSendSignal = 0x40,
    ///ClearToSendSignal = 0x80,
    Q_INVOKABLE quint32 getSerialPortSignals(void){uint32_t bits;emit getSerialPortSignalsSignal(&bits);return bits;}

    ///Returns a string which contains informations about all detected Aardvark I2C/SPI devices.
    Q_INVOKABLE QString aardvarkI2cSpiDetectDevices(void){return AardvarkI2cSpi::detectDevices();}

    ///Connects the main interface (Aardvark I2C/SPI).
    ///Note: A successful call will modify the corresponding settings in the settings dialog.
    Q_INVOKABLE bool aardvarkI2cSpiConnect(QJSValue aardvarkI2cSpiSettings, quint32 connectTimeout = 5000);

    ///Connects the main interface (serial port).
    ///Note: A successful call will modify the corresponding settings in the settings dialog.
    Q_INVOKABLE bool connectSerialPort(QString name, qint32 baudRate = 115200, quint32 connectTimeout= 1000, quint32 dataBits = 8, QString parity = "None",
                                       QString stopBits = "1", QString flowControl = "None");

    ///Connects the main interface (UDP or TCP socket).
    ///Note: A successful call will modify the corresponding settings in the settings dialog.
    Q_INVOKABLE bool connectSocket(bool isTcp, bool isServer, QString ip, quint32 destinationPort, quint32 ownPort, quint32 connectTimeout = 5000);

    ///Sets the value of an output pin (Aardvark I2C/SPI device).
    Q_INVOKABLE bool aardvarkI2cSpiSetOutput(quint8 pinIndex, bool high, bool updateSettingsDialog=false);

    ///Changes the configuration of a pin (Aardvark I2C/SPI device).
    Q_INVOKABLE bool aardvarkI2cSpiChangePinConfiguration(quint8 pinIndex, bool isInput, bool withPullups=false);

    ///Returns the Aardvark I2C/SPI settings of the main interface.
    Q_INVOKABLE QJSValue aardvarkI2cSpiGetMainInterfaceSettings(void);

    ///Reads all inputs of the Aardvark I2C/SPI device.
    Q_INVOKABLE QVector<bool> aardvarkI2cSpiReadAllInputs(void);

    ///Returns the serial port settings of the main interface.
    Q_INVOKABLE QJSValue getMainInterfaceSerialPortSettings(void);

    ///Returns the socket (UDP, TCP client/server) settings of the main interface.
    Q_INVOKABLE QJSValue getMainInterfaceSocketSettings(void);

    ///Returns all IP addresses found on the host machine.
    Q_INVOKABLE QStringList getLocalIpAdress(void);

    ///Returns a list with the name of all available serial ports.
    Q_INVOKABLE QStringList availableSerialPorts(void);

    ///Returns a list with the information of all available serial ports.
    Q_INVOKABLE QJSValue availableSerialPortsExt(void);


signals:


    ///This signal is emitted if data has been received with the main interface (only if the main interface is not a CAN or I2C master interface,
    ///use canMessagesReceivedSignal if the main interface is a can interface and i2cMasterDataReceivedSignal if the main interface is
    ///an I2C interface).
    ///Scripts can connect a function to this signal.
    void dataReceivedSignal(QVector<unsigned char> data);

    ///This signal is emitted if data has been received with the main interface and the main interface is an I2C master.
    ///Scripts can connect a function to this signal.
    void i2cMasterDataReceivedSignal(quint8 flags, quint16 address, QVector<unsigned char> data);

    ///This signal is emitted if a can message (or several) has been received with the main interface.
    ///Scripts can connect a function to this signal.
    void canMessagesReceivedSignal(QVector<quint8> types, QVector<quint32> messageIds, QVector<quint32> timestamps,
                                   QVector<QVector<unsigned char>>  data);

    ///Is emitted if the main interface shall send data.
    ///Scripts can use this signal to send the data with an additional interface.
    ///Scripts can connect a function to this signal.
    void sendDataFromMainInterfaceSignal(QVector<unsigned char> data);

    ///Is emitted if the input states (true=1. false=0) of the Ardvard I2C/SPI device (main interface) have been changed.
    ///Elements of state:0=Pin1/SCL, 1=Pin3/SDA, 2=Pin5/MISO, 3=Pin7/SCK, 4=Pin8/MOSI, 5=Pin9/SS0.
    ///Scripts can connect a function to this signal.
    void aardvarkI2cSpiInputStatesChangedSignal(QVector<bool> states);

    ///Is emitted if the main interface is a I2C or SPI slave and has sent data.
    void slaveDataSentSignal(QVector<unsigned char> data);

    /**********************private Signals*********************************************/

    ///Is connected with MainInterfaceThread::sendData (sends data with the main interface).
    ///This signal must not be used from script.
    void sendDataSignal(const QByteArray data, uint id);

    ///Adds data to the main window send history.
    ///This signal must not be used from script.
    void addDataToMainWindowSendHistorySignal(QByteArray data);

    ///Is emitted in i2cMasterFreeBus.
    ///This signal must not be used from script.
    void i2cMasterFreeBusSignal(void);

    ///With this signal the script thread requests the main interface thread to connect with the man interface interface.
    ///This signal is connected to the MainInterfaceThread::connectDataConnectionSlot slot.
    ///This signal must not be used from script.
    void connectDataConnectionSignal(Settings settings, bool connect, bool showMessageBoxOnError);

    ///With this signal scripts can change the current settings.
    ///This signal must not be used from script.
    void setAllSettingsSignal(Settings& settings, bool setTabIndex);

    ///This event is emitted in setSerialPortPins.
    ///This signal must not be used from script.
    void setSerialPortPinsSignal(bool setRTS, bool setDTR);

    ///Returns the state of the serial port signals (pins).
    ///This signal must not be used from script.
    void getSerialPortSignalsSignal(uint32_t* bits);

    ///Is emitted in setAardvarkI2cSpiOutput.
    ///This signal must not be used from script.
    void setAardvarkI2cSpiOutputSignal(AardvarkI2cSpiSettings settings);

    ///Is emitted in changeAardvarkI2cSpiPinConfiguration.
    ///This signal must not be used from script.
    void changeAardvarkI2cSpiPinConfigurationSignal(AardvarkI2cSpiSettings settings);

    ///Is emitted in aardvarkI2cSpiReadAllInputs.
    ///This signal must not be used from script.
    void ardvarkI2cSpiReadAllInputsSignal(QVector<bool>& inputStates);


public slots:

    ///This slot is connected with MainInterfaceThread::dataConnectionStatusSignal.
    ///The connected status (main interface) is reported with this signal.
    void dataConnectionStatusSlot(bool isConnected, QString message, bool isWaiting);

    ///Is called if the input states of the Ardvard I2C/SPI device (main interface) have been changed.
    ///Elements of state:0=Pin1/SCL, 1=Pin3/SDA, 2=Pin5/MISO, 3=Pin7/SCK, 4=Pin8/MOSI, 5=Pin9/SS0.
    void aardvarkI2cSpiInputStatesChangedSlot(QVector<bool> states){emit aardvarkI2cSpiInputStatesChangedSignal(states);}

    ///Sends the send data from the main interface.
    void sendDataFromMainInterfaceSlot(const QByteArray data);

    ///Is called, if data from the main interface (MainInterfaceThread) has been received.
    ///It converts the received QByteArray into a QVector and emits the dataReceivedSignal.
    void dataReceivedSlot(QByteArray data);

    ///The slot is called if the main interface thread has received data.
    ///This slot is connected to the MainInterfaceThread::dataReceivedSignal signal.
    void canMessagesReceivedSlot(QVector<QByteArray> messages);

    ///This slot is connected with the MainInterfaceThread::sendingFinishedSignal signal.
    ///The main interface thread emits this signal if the sending of data has been finished.
    void sendingFinishedSlot(bool success, uint id);

    ///Emits the dataReceivedSignal with the saved received data (if the script is running in the script debugger).
    void debugReceiveTimerSlot(void);

    ///Is called if the main interface is a I2C or SPI slave and has sent data.
    void slaveDataSentSlot(QByteArray data);

private:


    ///Waits until the main interface is connected or a timeout occurs.
    void waitForMainInterfaceToConnect(quint32 connectTimeout);

    ///Sends a byte array (QByteArray) with the main interface (in MainInterfaceThread).
    bool sendByteArray(QByteArray byteArray, int repetitionCount, int pause, bool addToMainWindowSendHistory);

    ///Pointer to the script thread.
    ScriptThread* m_scriptThread;

    ///True, if the last sending of data has successfully finished.
    bool m_sendingSucceeded;

    ///True, if the main interface is connected.
    bool m_isConnected;

    ///True, if the main interface is a CAN interface (and is connected).
    bool m_isConnectedWithCan;

    ///True, if the main interface is a I2C interface (and is connected).
    bool m_isConnectedWithI2c;

    ///True, if the main interface is a I2C master interface (and is connected).
    bool m_isConnectedWithI2cMaster;

    ///Pointer to the settings dialog.
    SettingsDialog *m_settingsDialog;

    ///This timer is used to emit the dataReceivedSignal in debugReceiveTimerSlot (if the script is running in the script debugger).
    QTimer m_debugReceiveTimer;

    ///Contains the saved received data (dataReceivedSlot) if the script is running in the script debugger.
    QVector<unsigned char> m_savedReceivedData;


};

#endif // SCRIPT_INF_H

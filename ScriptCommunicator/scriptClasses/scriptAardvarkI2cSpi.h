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

#ifndef SCRIPT_AARDVARK_I2C_SPI_H
#define SCRIPT_AARDVARK_I2C_SPI_H

#include "scriptObject.h"
#include "scriptThread.h"


///This wrapper class is used to access a AardvarkI2cSpi object from a script.
class ScriptAardvarkI2cSpi : public QObject, public ScriptObject
{
    Q_OBJECT

    ///Returns a semicolon separated list with all public functions, signals and properties.
    Q_PROPERTY(QString publicScriptElements READ getPublicScriptElements)
public:
    ScriptAardvarkI2cSpi(ScriptThread* scriptThread) : QObject(scriptThread), m_masterSpiLastReceivedData(), m_masterI2cLastReceivedData(),
        m_interface(0), m_scriptThread(scriptThread)
    {
        memset(&m_currentSettings, 0, sizeof(m_currentSettings));

        m_interface = new AardvarkI2cSpi(this);

        connect(m_interface, SIGNAL(inputStatesChangedSignal(QVector<bool>)),
                this, SLOT(inputStatesChangedSlot(QVector<bool>)), Qt::QueuedConnection);

        connect(m_interface, SIGNAL(readyRead()),this, SLOT(slaveDataReceivedSlot()));

        connect(this, SIGNAL(readAllInputsSignal(QVector<bool>&)),
                m_interface, SLOT(readAllInputs(QVector<bool>&)), Qt::BlockingQueuedConnection);
    }

    ~ScriptAardvarkI2cSpi()
    {
        disconnect();
    }

    ///Returns a semicolon separated list with all public functions, signals and properties.
    virtual QString getPublicScriptElements(void)
    {
        return MainWindow::parseApiFile("ScriptAardvarkI2cSpi.api");
    }

    ///Coverts a script value to a AardvarkI2cSpiSettings struct.
    static bool scriptValueToConfig(QJSValue& aardvarkI2cSpiSettings, AardvarkI2cSpiSettings& convertedSettings,
                                      ScriptThread* scriptThread, QString callerName)
    {
        bool succeeded = false;

        if(aardvarkI2cSpiSettings.hasProperty("devicePort") &&
           aardvarkI2cSpiSettings.hasProperty("deviceMode") &&
           aardvarkI2cSpiSettings.hasProperty("device5VIsOn") &&
           aardvarkI2cSpiSettings.hasProperty("i2cBaudrate") &&
           aardvarkI2cSpiSettings.hasProperty("i2cSlaveAddress") &&
           aardvarkI2cSpiSettings.hasProperty("i2cPullupsOn") &&
           aardvarkI2cSpiSettings.hasProperty("spiPolarity") &&
           aardvarkI2cSpiSettings.hasProperty("spiSSPolarity") &&
           aardvarkI2cSpiSettings.hasProperty("spiBitorder") &&
           aardvarkI2cSpiSettings.hasProperty("spiPhase") &&
           aardvarkI2cSpiSettings.hasProperty("spiBaudrate") &&
           aardvarkI2cSpiSettings.hasProperty("pinConfigs"))
        {
            succeeded = true;
            convertedSettings.devicePort = aardvarkI2cSpiSettings.property("devicePort").toUInt();
            convertedSettings.deviceMode = (AardvarkI2cSpiDeviceMode)aardvarkI2cSpiSettings.property("deviceMode").toUInt();
            convertedSettings.device5VIsOn = aardvarkI2cSpiSettings.property("device5VIsOn").toBool();
            convertedSettings.i2cBaudrate = aardvarkI2cSpiSettings.property("i2cBaudrate").toUInt();
            convertedSettings.i2cSlaveAddress = aardvarkI2cSpiSettings.property("i2cSlaveAddress").toUInt();
            convertedSettings.i2cPullupsOn = aardvarkI2cSpiSettings.property("i2cPullupsOn").toBool();
            convertedSettings.spiSSPolarity = (AardvarkSpiSSPolarity)aardvarkI2cSpiSettings.property("spiSSPolarity").toUInt();
            convertedSettings.spiBitorder = (AardvarkSpiBitorder)aardvarkI2cSpiSettings.property("spiBitorder").toUInt();
            convertedSettings.spiPhase = (AardvarkSpiPhase)aardvarkI2cSpiSettings.property("spiPhase").toUInt();
            convertedSettings.spiBaudrate = aardvarkI2cSpiSettings.property("spiBaudrate").toUInt();

            QJSValue pinConfigs = aardvarkI2cSpiSettings.property("pinConfigs");
            for(int i = 0; i < AARDVARK_I2C_SPI_GPIO_COUNT; i++)
            {
                if(pinConfigs.property(i).hasProperty("isInput") &&
                   pinConfigs.property(i).hasProperty("withPullups") &&
                   pinConfigs.property(i).hasProperty("outValue"))
                {
                    convertedSettings.pinConfigs[i].isInput = pinConfigs.property(i).property("isInput").toBool();
                    convertedSettings.pinConfigs[i].withPullups = pinConfigs.property(i).property("withPullups").toBool();
                    convertedSettings.pinConfigs[i].outValue = pinConfigs.property(i).property("outValue").toBool();
                }
                else
                {
                    succeeded = false;
                    scriptThread->messageBox("Critical", scriptThread->getScriptFileName(), "The AardvarkI2cSpiSettings structure in " + callerName + " is incomplete.");

                    break;
                }
            }

        }
        else
        {
            scriptThread->messageBox("Critical", scriptThread->getScriptFileName(), "The AardvarkI2cSpiSettings structure in " + callerName + " is incomplete.");
        }

        return succeeded;

    }

    ///Converts a AardvarkI2cSpiSettings struct to a script value.
    static QJSValue convertConfigToScriptValue(const AardvarkI2cSpiSettings* config, ScriptThread* scriptThread)
    {
        QJSValue ret = scriptThread->getScriptEngine()->newObject();

        ret.setProperty("devicePort", config->devicePort);
        ret.setProperty("deviceMode", config->deviceMode);
        ret.setProperty("device5VIsOn", config->device5VIsOn);
        ret.setProperty("i2cBaudrate", config->i2cBaudrate);
        ret.setProperty("i2cSlaveAddress", config->i2cSlaveAddress);
        ret.setProperty("i2cPullupsOn", config->i2cPullupsOn);
        ret.setProperty("spiPolarity", config->spiPolarity);
        ret.setProperty("spiSSPolarity", config->spiSSPolarity);
        ret.setProperty("spiBitorder", config->spiBitorder);
        ret.setProperty("spiPhase", config->spiPhase);
        ret.setProperty("spiBaudrate", config->spiBaudrate);

        QJSValue pinConfigs = scriptThread->getScriptEngine()->newArray(AARDVARK_I2C_SPI_GPIO_COUNT);
        for(int i = 0; i < AARDVARK_I2C_SPI_GPIO_COUNT; i++)
        {
            QJSValue el = scriptThread->getScriptEngine()->newObject();
            el.setProperty("isInput", config->pinConfigs[i].isInput);
            el.setProperty("withPullups", config->pinConfigs[i].withPullups);
            el.setProperty("outValue", config->pinConfigs[i].outValue);
            pinConfigs.setProperty(i, el);
        }
        ret.setProperty("pinConfigs", pinConfigs);

        return ret;
    }

    ///Returns the current interface settings.
    Q_INVOKABLE QJSValue getInterfaceSettings(void)
    {
        return convertConfigToScriptValue(&m_currentSettings, m_scriptThread);
    }

    ///Frees the main interface I2C bus (this function can be used if the no stop condition was created
    ///during the last i2cMasterReadWrite call)
    Q_INVOKABLE void i2cMasterFreeBus(void){m_interface->freeI2cBusSlot();}

    ///Returns a string which contains informations about all detected devices.
    Q_INVOKABLE QString detectDevices(void){return AardvarkI2cSpi::detectDevices();}

    ///Connects to the Aardvark I2C/SPI device/interface.
    Q_INVOKABLE bool connectToDevice(QJSValue aardvarkI2cSpiSettings)
    {

        AardvarkI2cSpiSettings settings;
        bool succeeded = ScriptAardvarkI2cSpi::scriptValueToConfig(aardvarkI2cSpiSettings, settings,
                                                                m_scriptThread, "ScriptAardvarkI2cSpi::connectToDevice");
        if(succeeded)
        {
            int deviceBitrate;
            m_currentSettings = settings;
            succeeded = m_interface->connectToDevice(m_currentSettings, deviceBitrate);
        }
        return succeeded;
    }

    ///Disconnects from the interface.
    Q_INVOKABLE void disconnect(void){m_interface->disconnect();}


    ///Accesses the I2C bus (write/read).
    ///The received data can be read with i2cMasterReadLastReceivedData.
    Q_INVOKABLE bool i2cMasterReadWrite(quint8 flags, quint16 slaveAddress, quint16 numberOfBytesToRead,
                                              QVector<unsigned char> dataToSend = QVector<unsigned char>())
    {
        bool succeeded = false;

        if(m_currentSettings.deviceMode == AARDVARK_I2C_SPI_DEVICE_MODE_I2C_MASTER)
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

            succeeded = m_interface->sendReceiveData(byteArray, &m_masterI2cLastReceivedData);

            if(!m_masterI2cLastReceivedData.isEmpty())
            {
                //Remove the meta data.
                m_masterI2cLastReceivedData.remove(0, AardvarkI2cSpi::RECEIVE_CONTROL_BYTES_COUNT);
            }
        }

        return succeeded;

    }

    ///Returns last received data from the I2C interface (master mode).
    Q_INVOKABLE QVector<unsigned char> i2cMasterReadLastReceivedData(void)
    {
        QVector<unsigned char> dataVector;
        for(auto val : m_masterI2cLastReceivedData)
        {
            dataVector.push_back((unsigned char) val);
        }
        m_masterI2cLastReceivedData.clear();
        return dataVector;
    }

    ///Sends and receive data with the SPI interface (master mode)
    ///(the received data must be read with spiMasterReadLastReceivedData).
    Q_INVOKABLE bool spiMasterSendReceiveData(QVector<unsigned char> dataToSend)
    {
        bool succeeded = false;

        if(m_currentSettings.deviceMode == AARDVARK_I2C_SPI_DEVICE_MODE_SPI_MASTER)
        {
            QByteArray byteArray;
            byteArray.append(QByteArray(reinterpret_cast<const char*>(dataToSend.constData()), dataToSend.size()));
            succeeded = m_interface->sendReceiveData(byteArray, &m_masterSpiLastReceivedData);
        }

        return succeeded;

    }

    ///Sets the slave (I2C/SPI) response.
    ///slaveDataSentSignal can be used to receive the sent data and slaveDataReceivedSignal
    ///can be uses to read the received data.
    Q_INVOKABLE bool slaveSetResponse(QVector<unsigned char> response)
    {
        bool succeeded = false;

        if((m_currentSettings.deviceMode == AARDVARK_I2C_SPI_DEVICE_MODE_SPI_SLAVE) ||
           (m_currentSettings.deviceMode == AARDVARK_I2C_SPI_DEVICE_MODE_I2C_SLAVE))
        {
            QByteArray dummyArray;
            QByteArray byteArray;
            byteArray.append(QByteArray(reinterpret_cast<const char*>(response.constData()), response.size()));
            succeeded = m_interface->sendReceiveData(byteArray, &dummyArray);
        }

        return succeeded;

    }

    ///Returns last received data from the SPI interface (master mode).
    Q_INVOKABLE QVector<unsigned char> spiMasterReadLastReceivedData(void)
    {
        QVector<unsigned char> dataVector;
        for(auto val : m_masterSpiLastReceivedData)
        {
            dataVector.push_back((unsigned char) val);
        }
        m_masterSpiLastReceivedData.clear();
        return dataVector;
    }

    ///Sets the value of an output pin.
    ///Possible pin indexes:0=Pin1/SCL, 1=Pin3/SDA, 2=Pin5/MISO, 3=Pin7/SCK, 4=Pin8/MOSI, 5=Pin9/SS0.
    Q_INVOKABLE bool setOutput(quint8 pinIndex, bool high)
    {
        bool result = false;

        if(pinIndex < AARDVARK_I2C_SPI_GPIO_COUNT)
        {
            result = true;
            m_currentSettings.pinConfigs[pinIndex].outValue = high;
            m_interface->outputValueChangedSlot(m_currentSettings);
        }

        return result;
    }

    ///Changes the configuration of a pin.
    ///Possible pin indexes:0=Pin1/SCL, 1=Pin3/SDA, 2=Pin5/MISO, 3=Pin7/SCK, 4=Pin8/MOSI, 5=Pin9/SS0.
    Q_INVOKABLE bool changePinConfiguration(quint8 pinIndex, bool isInput, bool withPullups=false)
    {
        bool result = false;

        if(pinIndex < AARDVARK_I2C_SPI_GPIO_COUNT)
        {
            result = true;
            m_currentSettings.pinConfigs[pinIndex].outValue = 0;
            m_currentSettings.pinConfigs[pinIndex].isInput = isInput;
            m_currentSettings.pinConfigs[pinIndex].withPullups = withPullups;
            m_interface->pinConfigChangedSlot(m_currentSettings);
        }

        return result;
    }

    ///Reads all inputs.
    ///The indexes of the result array are:0=Pin1/SCL, 1=Pin3/SDA, 2=Pin5/MISO, 3=Pin7/SCK, 4=Pin8/MOSI, 5=Pin9/SS0.
    Q_INVOKABLE QVector<bool> readAllInputs(void)
    {
        QVector<bool> readValues;
        emit readAllInputsSignal(readValues);
        return readValues;
    }

    ///Returns true if the interface is connected.
    Q_INVOKABLE bool isConnected(void){return m_interface->isConnected();}

signals:

    ///Is emitted if the input states of the Ardvard I2C/SPI device have been changed.
    ///Elements of state::0=Pin1/SCL, 1=Pin3/SDA, 2=Pin5/MISO, 3=Pin7/SCK, 4=Pin8/MOSI, 5=Pin9/SS0.
    ///Scripts can connect a function to this signal.
    void inputStatesChangedSignal(QVector<bool> states);

     ///Is called if the interface is a I2C or SPI slave and has sent data.
    void slaveDataSentSignal(QVector<unsigned char> data);

     ///Is called if the interface is a I2C or SPI slave and has received data.
    void slaveDataReceivedSignal(QVector<unsigned char> data);

     ///Is emitted in readAllInputs.
     ///This signal must not be used from script.
     void readAllInputsSignal(QVector<bool>& inputStates);

public slots:

    ///Is called if the input states of the Ardvard I2c/Spi device have been changed.
    ///Note: states contains AARDVARK_I2C_SPI_GPIO_COUNT elements.
    void inputStatesChangedSlot(QVector<bool> states){emit inputStatesChangedSignal(states);}

    ///This slot function is called if data has been received from the aardvard interface (I2C/SPI slave mode).
    void slaveDataReceivedSlot(void)
    {
        QVector<AardvardkI2cSpiSlaveData> transActions = m_interface->readLastSlaveData();

        for(auto el : transActions)
        {
            QVector<unsigned char> dataVector;
            for(auto val : el.data)
            {
                dataVector.push_back((unsigned char) val);
            }

            if(el.isReceiveData)
            {
                emit slaveDataReceivedSignal(dataVector);
            }
            else
            {
                emit slaveDataSentSignal(dataVector);
            }
        }
    }


private:

    ///The last received data from the SPI interface (master mode).
    QByteArray m_masterSpiLastReceivedData;

    ///The last received data from the I2C interface (master mode).
    QByteArray m_masterI2cLastReceivedData;

    ///The current interface settings.
    AardvarkI2cSpiSettings m_currentSettings;

    ///The wrapped cheetah spi interface.
    AardvarkI2cSpi* m_interface;

    ///Pointer to the script thread.
    ScriptThread* m_scriptThread;

};


#endif // SCRIPT_AARDVARK_I2C_SPI_H

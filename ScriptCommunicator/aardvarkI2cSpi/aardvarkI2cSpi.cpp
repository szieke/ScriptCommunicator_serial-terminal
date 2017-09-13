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

#include "aardvarkI2cSpi.h"
#include <QThread>


#if defined(WIN32) || defined(_WIN32)
static const QString LIBRARAY_NAME = "aardvark.dll";
#else
static const QString LIBRARAY_NAME = "aardvark.so";
#endif
AardvarkI2cSpi::AardvarkI2cSpi(QObject *parent): QObject(parent), m_handle(0),
    m_receiveInputTimer(), m_currentSlaveData()
{

    memset(&m_settings, 0, sizeof(m_settings));

    for(int i = 0; i < AARDVARK_I2C_SPI_GPIO_COUNT; i++)
    {
        m_inputStates.append(false);
    }

    connect(&m_receiveInputTimer, SIGNAL(timeout()),this, SLOT(receiveInputTimerSlot()));
}
AardvarkI2cSpi::~AardvarkI2cSpi()
{

}


/**
 * Reads all inputs of the aardvark I2c/Spi device.
 *
 * @param inputStates
 *      The array in which the read states are written to.
 */
void AardvarkI2cSpi::readAllInputs(QVector<bool>& inputStates)
{
    inputStates.clear();

    if (m_handle > 0)
    {
        quint32 result = (quint32)aa_gpio_get(m_handle);
        quint32 bitMask = 0;

        for(int i = 0; i < AARDVARK_I2C_SPI_GPIO_COUNT; i++)
        {
            bitMask = 1 << i;
            if((result & bitMask) != 0)
            {
                inputStates.append(true);
            }
            else
            {
                inputStates.append(false);
            }
            }

    }
    else
    {
        for(int i = 0; i < AARDVARK_I2C_SPI_GPIO_COUNT; i++)
        {
            inputStates.append(false);
        }
    }
}

/**
 * Checks the all inputs of the aardvark I2c/Spi device and generates
 * inputStatesChangedSignal if the inputs have been changed.
 */
void AardvarkI2cSpi::checkInputs(void)
{
    QVector<bool> inputStates;

    readAllInputs(inputStates);

    for(int i = 0; i < AARDVARK_I2C_SPI_GPIO_COUNT; i++)
    {
        if(inputStates[i] != m_inputStates[i])
        {//State changed.

            emit inputStatesChangedSignal(inputStates);
            break;
        }
    }

    //Copy the read states.
    m_inputStates = inputStates;

}


/**
 * Is called if the pin configuration has been changed (in the GUI).
 * @param settings
 *      The interface settings.
 */
void AardvarkI2cSpi::pinConfigChangedSlot(AardvarkI2cSpiSettings settings)
{
    m_settings = settings;

    if (m_handle > 0)
    {
        configurePins(false);
    }

}


/**
 * Is called if the value of an output pin has been changed (in the GUI).
 * @param settings
 *      The interface settings.
 */
void AardvarkI2cSpi::outputValueChangedSlot(AardvarkI2cSpiSettings settings)
{
    m_settings = settings;

    if (m_handle > 0)
    {
        configurePins(true);
    }
}


/**
 * Is called if the i2c bus shall be released.
 */
void AardvarkI2cSpi::freeI2cBusSlot(void)
{
    if(m_settings.deviceMode == AARDVARK_I2C_SPI_DEVICE_MODE_I2C_MASTER)
    {
        if (m_handle > 0)
        {
            (void)aa_i2c_free_bus (m_handle);
        }

    }
}

/**
 * Returns a string which contains informations about all detected devices.
 * @return
 *      The created string.
 */
QString AardvarkI2cSpi::detectDevices(void)
{
    u16  ports[16];
    u32 unique_ids[16];
    int nelem = 16;
    QString text;

    //Find all the attached devices.
    int count = aa_find_devices_ext(nelem, ports, nelem, unique_ids);
    int i;

    if(count >= 0)
    {

        text = QString("%1 device(s) found:").arg(count);

        //Append the information on each device.
        if (count > nelem)  count = nelem;
        for (i = 0; i < count; ++i)
        {
            // Determine if the device is in-use
            QString status = "available ";
            if (ports[i] & AA_PORT_NOT_FREE)
            {
                ports[i] &= ~AA_PORT_NOT_FREE;
                status = "in use";
            }


            text += QString("\nport:%1  status:%2  id:%3-%4").arg(QString::number(ports[i]), status,
                                                               QString::number(unique_ids[i]/1000000),
                                                               QString::number(unique_ids[i]%1000000));
        }
    }
    else
    {
        if(count == AA_UNABLE_TO_LOAD_LIBRARY)
        {
            text = "could not load: " + LIBRARAY_NAME;
        }
        else
        {
            text = aa_status_string(count);
        }
    }

    return text;
}

/**
 * Configures pins of the aardvark I2c/Spi device.
 *
 * @param onlySetOutputs
 *      True if only the output values shall be set.
 */
void AardvarkI2cSpi::configurePins(bool onlySetOutputs)
{
    u08 directionBitmask = 0;
    u08 pullupBitmask = 0;
    u08 setBitmask = 0;
    int startIndex = 0;
    int endIndex = 0;

    if((m_settings.deviceMode == AARDVARK_I2C_SPI_DEVICE_MODE_I2C_MASTER) ||
       (m_settings.deviceMode == AARDVARK_I2C_SPI_DEVICE_MODE_I2C_SLAVE))
    {
        startIndex = 2;
        endIndex = AARDVARK_I2C_SPI_GPIO_COUNT - 1;
    }
    else if((m_settings.deviceMode == AARDVARK_I2C_SPI_DEVICE_MODE_SPI_MASTER) ||
            (m_settings.deviceMode == AARDVARK_I2C_SPI_DEVICE_MODE_SPI_SLAVE))
    {
        startIndex = 0;
        endIndex = 1;
    }
    else
    {//AARDVARK_I2C_SPI_DEVICE_MODE_GPIO

        startIndex = 0;
        endIndex = AARDVARK_I2C_SPI_GPIO_COUNT - 1;
    }

    for(int i = startIndex; i <= endIndex; i++)
    {
        if(m_settings.pinConfigs[i].isInput)
        {
            if(m_settings.pinConfigs[i].withPullups)
            {
                pullupBitmask += 1 << i;
            }
        }
        else
        {
            directionBitmask += 1 << i;

            if(m_settings.pinConfigs[i].outValue)
            {
                setBitmask += 1 << i;
            }
        }
    }

    if(!onlySetOutputs)
    {
        (void)aa_gpio_direction(m_handle, directionBitmask);
        (void)aa_gpio_pullup (m_handle, pullupBitmask);
    }
    (void)aa_gpio_set (m_handle, setBitmask);
}

/**
 * Returns the data from the last transactions.
 */
QVector<AardvardkI2cSpiSlaveData> AardvarkI2cSpi::readLastSlaveData(void)
{
    QVector<AardvardkI2cSpiSlaveData> slaveData;
    aa_u08* inBuffer = new aa_u08[10000];

    int dataType = aa_async_poll(m_handle, 0);
    while(dataType != AA_ASYNC_NO_DATA)
    {
        int readBytes = 0;
        int writtenBytes = 0;

        if((dataType & AA_ASYNC_I2C_WRITE) != 0)
        {
            writtenBytes = aa_i2c_slave_write_stats(m_handle);
        }
        else if((dataType & AA_ASYNC_I2C_READ) != 0)
        {
            u08 address;
            readBytes = aa_i2c_slave_read(m_handle, &address, 10000, inBuffer);
            if(address != m_settings.i2cSlaveAddress)
            {
                //Ignore the data which were sent to other I2C slaves.
                readBytes = 0;
            }
        }
        else if((dataType & AA_ASYNC_SPI) != 0)
        {
            readBytes = aa_spi_slave_read(m_handle, 10000, inBuffer);
            writtenBytes = readBytes;
        }
        else if((dataType & AA_ASYNC_I2C_MONITOR) != 0)
        {
            u16 monitoreData[100];
            //Ignore this information.
            (void)aa_i2c_monitor_read(m_handle, 100, monitoreData);
        }


        if(readBytes > 0)
        {
            AardvardkI2cSpiSlaveData receivedData;
            receivedData.isReceiveData = true;
            for(int i = 0; i < readBytes; i++)
            {
               receivedData.data.append(inBuffer[i]);
            }
            slaveData.append(receivedData);
        }

        if(writtenBytes != 0)
        {
            AardvardkI2cSpiSlaveData sentData;
            sentData.isReceiveData = false;

            while(writtenBytes >= m_currentSlaveData.size())
            {
                sentData.data.append(m_currentSlaveData);
                writtenBytes -= m_currentSlaveData.size();
            }

            if(writtenBytes != 0)
            {
                sentData.data.append(m_currentSlaveData.mid(0, writtenBytes));
            }
            slaveData.append(sentData);
        }

        dataType = aa_async_poll(m_handle, 0);
    }

    delete [] inBuffer;
    return slaveData;
}

/**
 * Checks if data has been received (I2C/SPI slave mode) and the states of the inputs.
 */
void AardvarkI2cSpi::receiveInputTimerSlot()
{

    m_receiveInputTimer.stop();

    if((m_settings.deviceMode == AARDVARK_I2C_SPI_DEVICE_MODE_I2C_SLAVE) ||
       (m_settings.deviceMode == AARDVARK_I2C_SPI_DEVICE_MODE_SPI_SLAVE))
    {
        int result = aa_async_poll (m_handle, 0);
        if(result != AA_ASYNC_NO_DATA)
        {
            emit readyRead();
        }
    }

    checkInputs();

    m_receiveInputTimer.start(1);
}

/**
 * Connects to a aarvard I2C SPI interface.
 * @param settings
 *      The interface settings.
 * @param deviceBitrate
 *      The actual bitrate set.
 * @return
 *      True on success.
 */
bool AardvarkI2cSpi::connectToDevice(AardvarkI2cSpiSettings& settings, int& deviceBitrate)
{
    bool succeeded = false;
    m_handle = aa_open(settings.devicePort);
    m_settings = settings;

    //Set the default response.
    m_currentSlaveData = QByteArray();
    m_currentSlaveData.append(0xff);

    if (m_handle > 0)
    {
        if((m_settings.deviceMode == AARDVARK_I2C_SPI_DEVICE_MODE_I2C_MASTER) ||
           (m_settings.deviceMode == AARDVARK_I2C_SPI_DEVICE_MODE_I2C_SLAVE))
        {   
            if(aa_configure(m_handle,  AA_CONFIG_GPIO_I2C) >= 0)
            {
                succeeded = true;

                if(settings.i2cPullupsOn)
                {
                    //Enable the I2C bus pullup resistors (2.2k resistors).
                    (void)aa_i2c_pullup(m_handle, AA_I2C_PULLUP_BOTH);
                }
                deviceBitrate = aa_i2c_bitrate(m_handle, m_settings.i2cBaudrate);

                if(m_settings.deviceMode == AARDVARK_I2C_SPI_DEVICE_MODE_I2C_SLAVE)
                {
                    succeeded = (aa_i2c_slave_enable(m_handle, m_settings.i2cSlaveAddress, 0, 0) >= 0) ? true : false;

                    //Set the default response.
                    (void)aa_i2c_slave_set_response(m_handle, m_currentSlaveData.size(), (const aa_u08 *)m_currentSlaveData.constData());
                }
            }
        }
        else if((m_settings.deviceMode == AARDVARK_I2C_SPI_DEVICE_MODE_SPI_MASTER) ||
                (m_settings.deviceMode == AARDVARK_I2C_SPI_DEVICE_MODE_SPI_SLAVE))
        {
            if(aa_configure(m_handle,  AA_CONFIG_SPI_GPIO) >= 0)
            {
                succeeded = true;

                (void)aa_spi_configure(m_handle, m_settings.spiPolarity, m_settings.spiPhase, m_settings.spiBitorder);
                deviceBitrate = aa_spi_bitrate(m_handle, m_settings.spiBaudrate);

                (void)aa_spi_master_ss_polarity (m_handle,m_settings.spiSSPolarity);

                //Turn off the external I2C line pullups
                (void)aa_i2c_pullup(m_handle, AA_I2C_PULLUP_NONE);

                if(m_settings.deviceMode == AARDVARK_I2C_SPI_DEVICE_MODE_SPI_SLAVE)
                {
                    //Enable the slave.
                    succeeded = (aa_spi_slave_enable(m_handle) >= 0) ? true : false;

                    //Set the default response.
                    (void)aa_spi_slave_set_response(m_handle, m_currentSlaveData.size(), (const aa_u08 *)m_currentSlaveData.constData());
                }
            }

        }
        else
        {//AARDVARK_I2C_SPI_DEVICE_MODE_GPIO

            if(AA_OK == aa_configure(m_handle,  AA_CONFIG_GPIO_ONLY))
            {
                succeeded = true;

                //Turn off the external I2C line pullups
                (void)aa_i2c_pullup(m_handle, AA_I2C_PULLUP_NONE);
            }
        }

        if(succeeded)
        {
            if(settings.device5VIsOn)
            {
                (void)aa_target_power(m_handle, AA_TARGET_POWER_BOTH);
            }
            else
            {
                (void)aa_target_power(m_handle, AA_TARGET_POWER_NONE);
            }

            configurePins(false);
        }
    }

    if(succeeded)
    {
        //Read/save the input states.
        readAllInputs(m_inputStates);
        emit inputStatesChangedSignal(m_inputStates);

        m_receiveInputTimer.start(1);
    }
    else
    {
        disconnect();
    }

    return succeeded;
}

/**
 * Disconnects from the aarvard I2C SPI interface.
 */
void AardvarkI2cSpi::disconnect(void)
{
    m_receiveInputTimer.stop();

    if (m_handle > 0)
    {
        (void)aa_target_power(m_handle, AA_TARGET_POWER_NONE);//5v off.
        (void)aa_i2c_slave_disable(m_handle);
        (void)aa_spi_slave_disable(m_handle);
        (void)aa_configure(m_handle, AA_CONFIG_GPIO_ONLY);
        (void)aa_gpio_set (m_handle, 0);//Set all outputs to 0.
        (void)aa_gpio_direction(m_handle, 0);//All pins are inputs
        (void)aa_gpio_pullup (m_handle, 0);//No pullups.

        aa_close(m_handle);
        m_handle = 0;
    }

    //Read/save the input states.
    readAllInputs(m_inputStates);
    emit inputStatesChangedSignal(m_inputStates);
}

/**
 * Converts AardvarkI2cFlags to a string.
 *
 * @param flags
 *      The flags.
 * @return
 *      The created string.
 */
QString AardvarkI2cSpi::flagsToString(AardvarkI2cFlags flags)
{
    QString result;

    if(flags == AA_I2C_NO_FLAGS)
    {
        result = "0x" + QString::number(flags, 16);
    }
    else
    {
        if((flags & AA_I2C_10_BIT_ADDR) != 0)
        {
            result += "10 bit,";
        }
        if((flags & AA_I2C_COMBINED_FMT) != 0)
        {
            result += "comb. fmt,";
        }
        if((flags & AA_I2C_NO_STOP) != 0)
        {
            result += "no stop,";
        }

        //Remove the last ','.
        result.remove(result.size() - 1, 1);
        result += " (0x" + QString::number(flags, 16) + ")";
    }

    return result;
}

/**
 * Sends and receive data with the aarvard I2C SPI interface.
 * @param data
 *      The send data.
 * @param receivedData
 *      The received data.
 * @return
 *      True on success.
 */
bool AardvarkI2cSpi::sendReceiveData(const QByteArray& data, QByteArray* receivedData)
{
    bool succeeded = false;

    receivedData->clear();

    if(m_settings.deviceMode == AARDVARK_I2C_SPI_DEVICE_MODE_I2C_MASTER)
    {
        if(data.size() >= SEND_CONTROL_BYTES_COUNT)
        {
            AardvarkI2cFlags flags = (AardvarkI2cFlags)data[0];
            quint16 slaveAddress = (quint8)data[2] + ((quint16)((quint8)data[1]) << 8);
            quint16 bytesToRead = (quint8)data[4] + ((quint16)((quint8)data[3]) << 8);

            //Remove the metadata.
            QByteArray sendData = data.mid(SEND_CONTROL_BYTES_COUNT);

            if(bytesToRead == 0)
            {//Only write.

                if(!sendData.isEmpty())
                {
                    if(sendData.size() == aa_i2c_write(m_handle, slaveAddress, flags, sendData.size(), (const aa_u08 *)sendData.constData()))
                    {
                        succeeded = true;
                    }
                    else
                    {
                        succeeded = false;
                    }
                }
                else
                {
                    succeeded = false;
                }
            }
            else  if(sendData.isEmpty())
            {//Only read.

                aa_u08* inBuffer = new aa_u08[bytesToRead];
                if(bytesToRead == aa_i2c_read(m_handle, slaveAddress, flags, bytesToRead, inBuffer))
                {
                    succeeded = true;

                    receivedData->append(flags);
                    receivedData->append((slaveAddress >> 8) & 0xff);
                    receivedData->append(slaveAddress & 0xff);

                    for(int i = 0; i < bytesToRead; i++)
                    {
                        receivedData->append(inBuffer[i]);
                    }

                }
                else
                {
                    succeeded = false;
                }

                delete[] inBuffer;

            }
            else
            {//Write and read.

                aa_u16 bytesWritten = 0;
                aa_u16 bytesRead = 0;
                aa_u08* inBuffer = new aa_u08[bytesToRead];

                (void)aa_i2c_write_read(m_handle, slaveAddress, flags, sendData.size(), (const aa_u08 *)sendData.constData(), &bytesWritten,
                                             bytesToRead,inBuffer, &bytesRead);

                if((sendData.size() == bytesWritten) && (bytesToRead == bytesRead))
                {
                    succeeded = true;

                    receivedData->append(flags);
                    receivedData->append((slaveAddress >> 8) & 0xff);
                    receivedData->append(slaveAddress & 0xff);

                    for(int i = 0; i < bytesToRead; i++)
                    {
                        receivedData->append(inBuffer[i]);
                    }
                }
                else
                {
                    succeeded = false;
                }

                delete[] inBuffer;
            }

        }//if(sendData.size() >= 5)
        else
        {
            succeeded = false;
        }

    }
    else if(m_settings.deviceMode == AARDVARK_I2C_SPI_DEVICE_MODE_I2C_SLAVE)
    {
        if(aa_i2c_slave_set_response(m_handle, data.size(), (const aa_u08 *)data.constData()) == data.size())
        {
            succeeded = true;
            m_currentSlaveData = data;
        }
        else
        {
            succeeded = true;
        }
    }
    else if(m_settings.deviceMode == AARDVARK_I2C_SPI_DEVICE_MODE_SPI_MASTER)
    {
        aa_u08* inBuffer = new aa_u08[data.size()];

        if(data.size() == aa_spi_write(m_handle, data.size(), (const aa_u08 *)data.constData(), data.size(), inBuffer))
        {
            succeeded = true;

            for(int i = 0; i < data.size(); i++)
            {
                receivedData->append(inBuffer[i]);
            }
        }
        else
        {
            succeeded = false;
        }

        delete[] inBuffer;

    }
    else if(m_settings.deviceMode == AARDVARK_I2C_SPI_DEVICE_MODE_SPI_SLAVE)
    {
        if(aa_spi_slave_set_response(m_handle, data.size(), (const aa_u08 *)data.constData()) == data.size())
        {
            succeeded = true;
            m_currentSlaveData = data;
        }
        else
        {
            succeeded = true;
        }
    }
    else
    {//AARDVARK_I2C_SPI_DEVICE_MODE_GPIO

        succeeded = false;
    }

    return succeeded;
}

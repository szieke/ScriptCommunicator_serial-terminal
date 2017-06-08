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


#if defined(WIN32) || defined(_WIN32)
static const QString LIBRARAY_NAME = "aardvark.dll";
#else
static const QString LIBRARAY_NAME = "aardvark.so";
#endif
AardvarkI2cSpi::AardvarkI2cSpi(QObject *parent): QObject(parent), m_handle(-1), m_inputTimer()
{
    for(int i = 0; i < AARDVARD_I2C_SPI_GPIO_COUNT; i++)
    {
        m_inputStates[i] = false;
    }

    connect(&m_inputTimer, SIGNAL(timeout()),this, SLOT(inputTimerSlot()), Qt::QueuedConnection);
}
AardvarkI2cSpi::~AardvarkI2cSpi()
{

}

static quint32 testCounter = 0;

/**
 * Reads all inputs of the aardvard I2c/Spi device.
 *
 * @param inputStates
 *      The array in which the read states are written to.
 */
void AardvarkI2cSpi::readAllInputs(bool inputStates[AARDVARD_I2C_SPI_GPIO_COUNT])
{
    static bool dummyInputStates = false;

    //if (m_handle > 0)
    if (m_handle == -1)//ToDo: entfernen
    {
        if(0)
        {
            quint32 result = (quint32)aa_gpio_get(m_handle);
            quint32 bitMask = 0;

            for(int i = 0; i < AARDVARD_I2C_SPI_GPIO_COUNT; i++)
            {
                bitMask = 1 << i;
                if((result & bitMask) != 0)
                {
                    inputStates[i] = true;
                }
                else
                {
                    inputStates[i] = false;
                }
            }
        }
        else
        {
            testCounter++;

            if(testCounter > 10)
            {
                testCounter = 0;
                dummyInputStates = dummyInputStates ? false : true;
            }

            for(int i = 0; i < AARDVARD_I2C_SPI_GPIO_COUNT; i++)
            {
                inputStates[i] = dummyInputStates;
            }
        }
    }
    else
    {
        for(int i = 0; i < AARDVARD_I2C_SPI_GPIO_COUNT; i++)
        {
            inputStates[i] = false;
        }
    }
}

/**
 * Slot function of m_inputTimer.
 * Reads all inputs of the aardvard I2c/Spi device.
 */
void AardvarkI2cSpi::inputTimerSlot(void)
{
    bool inputStates[AARDVARD_I2C_SPI_GPIO_COUNT];

    readAllInputs(inputStates);

    for(int i = 0; i < AARDVARD_I2C_SPI_GPIO_COUNT; i++)
    {
        if(inputStates[i] != m_inputStates[i])
        {//State changed.

            emit inputStatesChangedSignal(inputStates);
            break;
        }
    }

    //Copy the read states.
    for(int i = 0; i < AARDVARD_I2C_SPI_GPIO_COUNT; i++)
    {
        m_inputStates[i] = inputStates[i];
    }
}


/**
 * Is called if the pin configuration has been changed (in the GUI).
 * @param settings
 *      The interface settings.
 */
void AardvarkI2cSpi::pinConfigChangedSlot(AardvardI2cSpiSettings settings)
{
    m_settings = settings;

    if (m_handle > 0)
    {
        configurePins(false);
    }

    testCounter = 10;

}


/**
 * Is called if the value of an output pin has been changed (in the GUI).
 * @param settings
 *      The interface settings.
 */
void AardvarkI2cSpi::outputValueChangedSlot(AardvardI2cSpiSettings settings)
{
    m_settings = settings;

    if (m_handle > 0)
    {
        configurePins(true);
    }

    testCounter = 10;
}


/**
 * Is called if the i2c bus shall be released.
 */
void AardvarkI2cSpi::freeI2cBusSlot(void)
{
    if(m_settings.deviceMode == AARDVARD_I2C_SPI_DEVICE_MODE_I2C_MASTER)
    {
        testCounter = 10;

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
 * Configures pins of the aardvard I2c/Spi device.
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

    if(m_settings.deviceMode == AARDVARD_I2C_SPI_DEVICE_MODE_I2C_MASTER)
    {
        startIndex = 2;
        endIndex = AARDVARD_I2C_SPI_GPIO_COUNT - 1;
    }
    else if(m_settings.deviceMode == AARDVARD_I2C_SPI_DEVICE_MODE_SPI_MASTER)
    {
        startIndex = 0;
        endIndex = 1;
    }
    else
    {//AARDVARD_I2C_SPI_DEVICE_MODE_GPIO

        startIndex = 0;
        endIndex = AARDVARD_I2C_SPI_GPIO_COUNT - 1;
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
 * Connects to a aarvard I2C SPI interface.
 * @param settings
 *      The interface settings.
 * @param deviceBitrate
 *      The actual bitrate set.
 * @return
 *      True on success.
 */
bool AardvarkI2cSpi::connectToDevice(AardvardI2cSpiSettings& settings, int& deviceBitrate)
{
    bool succeeded = false;
    m_handle = 0;
    m_settings = settings;


    if(0)
    {
        if(m_settings.deviceMode == AARDVARD_I2C_SPI_DEVICE_MODE_I2C_MASTER)
        {
            if(AA_CONFIG_ERROR != aa_configure(m_handle,  AA_CONFIG_GPIO_I2C))
            {
                succeeded = true;

                if(settings.i2cPullupsOn)
                {
                    //Enable the I2C bus pullup resistors (2.2k resistors).
                    (void)aa_i2c_pullup(m_handle, AA_I2C_PULLUP_BOTH);
                }
                deviceBitrate = aa_i2c_bitrate(m_handle, m_settings.i2cBaudrate);
            }
        }
        else if(m_settings.deviceMode == AARDVARD_I2C_SPI_DEVICE_MODE_SPI_MASTER)
        {
            if(AA_CONFIG_ERROR != aa_configure(m_handle,  AA_CONFIG_SPI_GPIO))
            {
                succeeded = true;

                (void)aa_spi_configure(m_handle, m_settings.spiPolarity, m_settings.spiPhase, m_settings.spiBitorder);
                deviceBitrate = aa_i2c_bitrate(m_handle, m_settings.spiBaudrate);

                (void)aa_spi_master_ss_polarity (m_handle,m_settings.spiSSPolarity);

                //Turn off the external I2C line pullups
                (void)aa_i2c_pullup(m_handle, AA_I2C_PULLUP_NONE);
            }

        }
        else
        {//AARDVARD_I2C_SPI_DEVICE_MODE_GPIO

            if(AA_CONFIG_ERROR != aa_configure(m_handle,  AA_CONFIG_GPIO_ONLY))
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

            configurePins(false);
        }

    }
    else
    {//ToDo: entfernen

        m_handle = -1;
        succeeded = true;
    }

    if(succeeded)
    {
        //Read/save the input states.
        readAllInputs(m_inputStates);
        emit inputStatesChangedSignal(m_inputStates);

        m_inputTimer.start(250);
    }

    return succeeded;
}

/**
 * Disconnects from the aarvard I2C SPI interface.
 */
void AardvarkI2cSpi::disconnect(void)
{
    m_inputTimer.stop();

    if (m_handle > 0)
    {
        aa_close(m_handle);
        m_handle = 0;
    }

    //Read/save the input states.
    readAllInputs(m_inputStates);
    emit inputStatesChangedSignal(m_inputStates);
}


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

    if(m_settings.deviceMode == AARDVARD_I2C_SPI_DEVICE_MODE_I2C_MASTER)
    {
        if(0)
        {
            if(data.size() >= SEND_CONTROL_BYTES_COUNT)
            {
                AardvarkI2cFlags flags = (AardvarkI2cFlags)data[0];
                quint16 slaveAddress = (quint16)data[2] + ((quint16)data[1] << 8);
                quint16 bytesToRead = (quint16)data[4] + ((quint16)data[3] << 8);

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
        else
        {
            if(data.size() >= SEND_CONTROL_BYTES_COUNT)
            {
                AardvarkI2cFlags flags = (AardvarkI2cFlags)data[0];
                quint16 slaveAddress = (quint16)data[2] + ((quint16)data[1] << 8);
                quint16 bytesToRead = (quint16)data[4] + ((quint16)data[3] << 8);

                //Remove the metadata.
                QByteArray sendData = data.mid(SEND_CONTROL_BYTES_COUNT);

                if(bytesToRead == 0)
                {//Only write.
                    if(!sendData.isEmpty())
                    {
                         succeeded = true;
                    }
                    else
                    {
                        succeeded = false;
                    }
                }
                else if(sendData.isEmpty())
                {//Only read.
                    receivedData->append("read");
                    receivedData->push_front(slaveAddress & 0xff);
                    receivedData->push_front((slaveAddress >> 8) & 0xff);
                    receivedData->push_front(flags);
                    succeeded = true;
                }
                else
                {//Write and read.
                    *receivedData = sendData;
                    receivedData->push_front(slaveAddress & 0xff);
                    receivedData->push_front((slaveAddress >> 8) & 0xff);
                    receivedData->push_front(flags);

                    succeeded = true;
                }

            }
            else
            {
                succeeded = false;
            }
        }
    }
    else if(m_settings.deviceMode == AARDVARD_I2C_SPI_DEVICE_MODE_SPI_MASTER)
    {

        if(0)
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
        else
        {
            *receivedData = data;
            succeeded = true;
        }
    }
    else
    {//AARDVARD_I2C_SPI_DEVICE_MODE_GPIO

        succeeded = false;
    }

    return succeeded;
}

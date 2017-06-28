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

#ifndef AARDVARK_I2C_SPI_H
#define AARDVARK_I2C_SPI_H


#include "aardvark.h"
#include <QObject>
#include <QTimer>
#include <QVector>


/*
   GPIO:
      SCL   = 0x01   HW Pin1   GUI Pin0
      SDA   = 0x02   HW Pin3   GUI Pin1
      MISO  = 0x04   HW Pin5   GUI Pin2
      SCK   = 0x08   HW Pin7   GUI Pin3
      MOSI  = 0x10   HW Pin8   GUI Pin4
      SS0   = 0x20   HW Pin9   GUI Pin5

   I2C:
      MISO  = 0x04   HW Pin5   GUI Pin2
      SCK   = 0x08   HW Pin7   GUI Pin3
      MOSI  = 0x10   HW Pin8   GUI Pin4
      SS0   = 0x20   HW Pin9   GUI Pin5

   SPI:
      SCL   = 0x01   HW Pin1   GUI Pin0
      SDA   = 0x02   HW Pin3   GUI Pin1
 */

///The number of aardvark I2C/SPI GPIOs.
#define AARDVARK_I2C_SPI_GPIO_COUNT 6

///aardvark I2C/SPI device mode.
typedef enum
{
    AARDVARK_I2C_SPI_DEVICE_MODE_I2C_MASTER = 0,
    AARDVARK_I2C_SPI_DEVICE_MODE_I2C_SLAVE,
    AARDVARK_I2C_SPI_DEVICE_MODE_SPI_MASTER,
    AARDVARK_I2C_SPI_DEVICE_MODE_SPI_SLAVE,
    AARDVARK_I2C_SPI_DEVICE_MODE_GPIO
}AardvarkI2cSpiDeviceMode;


///Aardvark I2C/SPI GPIO configuration.
typedef struct
{
    bool isInput;///True if the pin is an input.
    bool withPullups;///True if the pin shall have a pullup (works only with inputs).
    bool outValue;///The value of the output.
}AardvarkI2cSpiGpioConfig;

///Settings for an Aardvark I2C/SPI interface.
typedef struct
{
    quint16 devicePort;///The device port.
    AardvarkI2cSpiDeviceMode deviceMode;///The device mode.
    bool device5VIsOn;///True if the 5V pins shall be on.

    quint16 i2cBaudrate;///The I2C baudrate.
    bool i2cPullupsOn;///True if the I2C pullups shall be enabled.
    quint16 i2cSlaveAddress;///The I2C slave address.

    AardvarkSpiPolarity spiPolarity;///The SPI Polarity
    AardvarkSpiSSPolarity spiSSPolarity;///The SPI slave select polarity.
    AardvarkSpiBitorder spiBitorder;///The SPI bitorder.
    AardvarkSpiPhase spiPhase;///The SPI phase.
    quint16 spiBaudrate;///The SPI baudrate.

    ///The pin configuration.
    AardvarkI2cSpiGpioConfig pinConfigs[AARDVARK_I2C_SPI_GPIO_COUNT];

}AardvarkI2cSpiSettings;

///The slave data.
typedef struct
{
    QByteArray data;
    bool isReceiveData;

}AardvardkI2cSpiSlaveData;

///Class which represents a aardvark  I2c/Spi interface.
class AardvarkI2cSpi : public QObject
{
    Q_OBJECT

public:
    AardvarkI2cSpi(QObject *parent);

    virtual ~AardvarkI2cSpi();

    ///The number of control bytes for sending data.
    static const qint32 SEND_CONTROL_BYTES_COUNT = 5;

    ///The number of control bytes for receiving data.
    static const qint32 RECEIVE_CONTROL_BYTES_COUNT = 3;

    ///Returns a string which contains informations about all detected devices.
    static QString detectDevices(void);

    ///Connects to a aarvard I2C SPI interface.
    bool connectToDevice(AardvarkI2cSpiSettings& settings, int& deviceBitrate);

    ///Disconnects from the aarvard I2C SPI interface.
    void disconnect(void);

    ///Sends and receive data with the aarvard I2C SPI interface.
    bool sendReceiveData(const QByteArray& data, QByteArray* receivedData);

    ///Return true if the interface is connected.
    bool isConnected(void){return (m_handle > 0) ? true : false;}

    ///Returns the data from the last transactions.
    QVector<AardvardkI2cSpiSlaveData> readLastSlaveData(void);

    ///Converts AardvarkI2cFlags to a string.
    static QString flagsToString(AardvarkI2cFlags flags);

    ///Checks the all inputs of the aardvark I2c/Spi device and generates
    ///inputStatesChangedSignal if the inputs have been changed.
    void checkInputs(void);


signals:

    ///Is emitted if the input states of the aardvark I2c/Spi device have been changed.
    ///Note: states contains AARDVARK_I2C_SPI_GPIO_COUNT elements.
    void inputStatesChangedSignal(QVector<bool> states);

    ///Is emitted if data has been received (I2C/SPI slave mode).
    void readyRead(void);

public slots:

    ///Is called if the pin configuration has been changed (in the GUI).
    void pinConfigChangedSlot(AardvarkI2cSpiSettings settings);

    ///Is called if the value of an output pin has been changed (in the GUI).
    void outputValueChangedSlot(AardvarkI2cSpiSettings settings);

    ///Is called if the i2c bus shall be released.
    void freeI2cBusSlot(void);

    ///Checks if data has been received (I2C/SPI slave mode) and the states of the inputs.
    void receiveInputTimerSlot();

    ///Reads all inputs of the aardvark I2c/Spi device.
    void readAllInputs(QVector<bool>& inputStates);

private:


    ///Configures pins of the aardvark I2c/Spi device.
    void configurePins(bool onlySetOutputs);

    ///Handle to aardvark I2c/Spi device.
    Aardvark m_handle;

    ///The states of all inputs (aardvark I2c/Spi device).
    QVector<bool> m_inputStates;

    ///The current settings.
    AardvarkI2cSpiSettings m_settings;

    ///Checks if data has been received (I2C/SPI slave mode) and the states of the inputs.
    QTimer m_receiveInputTimer;

    ///The current slave data.
    QByteArray m_currentSlaveData;
};

#endif // AARDVARK_I2C_SPI_H

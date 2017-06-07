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

#ifndef AARDVARD_I2C_SPI_H
#define AARDVARD_I2C_SPI_H

#include "mainwindow.h"
#include "aardvark.h"
#include "settingsdialog.h"
#include <QObject>
#include <QTimer>


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

///Class which represents a aardvard  I2c/Spi interface.
class AardvarkI2cSpi : public QObject
{
    Q_OBJECT

public:
    AardvarkI2cSpi(QObject *parent);

    virtual ~AardvarkI2cSpi();

    ///Returns a string which contains informations about all detected devices.
    static QString detectDevices(void);

    ///Connects to a aarvard I2C SPI interface.
    bool connectToDevice(AardvardI2cSpiSettings& settings);

    ///Disconnects from the aarvard I2C SPI interface.
    void disconnect(void);

    ///Sends and receive data with the aarvard I2C SPI interface.
    bool sendReceiveData(const QByteArray& sendData, QByteArray* receivedData);

signals:

    ///Is emitted if the input states of the aardvard I2c/Spi device have been changed.
    void inputStatesChangedSignal(bool* states);

public slots:

    ///Slot function of m_inputTimer.
    ///Reads all inputs of the aardvard I2c/Spi device.
    void inputTimerSlot(void);

    ///Is called if the pin configuration has been changed (in the GUI).
    void pinConfigChangedSlot(AardvardI2cSpiGpioConfig config, quint8 guiPinNumber);

    ///Is called if the value of an output pin has been changed (in the GUI).
    void outputValueChangedSlot(bool state, quint8 guiPinNumber);

private:

    ///Reads all inputs of the aardvard I2c/Spi device.
    void readAllInputs(bool inputStates[]);

    ///Handle to aardvard I2c/Spi device.
    Aardvark m_handle;

    ///Timer for polling the inputs of the aardvard I2c/Spi device.
    QTimer m_inputTimer;

    ///The states of all inputs (aardvard I2c/Spi device).
    bool m_inputStates[AARDVARD_I2C_SPI_GPIO_COUNT];

    ///The current settings.
    AardvardI2cSpiSettings m_settings;
};

#endif // AARDVARD_I2C_SPI_H

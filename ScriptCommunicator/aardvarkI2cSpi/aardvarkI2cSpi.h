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
#include <QObject>

///Class which represents a aardvard  I2c/Spi interface.
class AardvarkI2cSpi : public QObject
{
    Q_OBJECT

public:
    AardvarkI2cSpi(QObject *parent);

    virtual ~AardvarkI2cSpi();

    ///Returns a string which contains informations about all detected devices.
    static QString detectDevices(void);

private:
        Aardvark m_handle;
};

#endif // AARDVARD_I2C_SPI_H

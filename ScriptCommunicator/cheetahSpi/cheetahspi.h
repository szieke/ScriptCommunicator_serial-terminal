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

#ifndef CHEETAHSPI_H
#define CHEETAHSPI_H

#include "mainwindow.h"
#include "cheetah.h"
#include <QObject>

///Class which represents a cheetah  spi interface.
class CheetahSpi : public QObject
{
    Q_OBJECT

public:
    CheetahSpi(QObject *parent);

    virtual ~CheetahSpi();

    ///Returns a string which contains informations about all detected devices..
    static QString detectDevices(void);

    ///Connects to a cheetah spi interface.
    bool connectToDevice(quint32 port, qint16 mode, quint32 baudrate);

    ///Disconnects from the cheetah spi interface.
    void disconnect(void);

    ///Sends and receive data with the cheetah spi interface.
    bool sendReceiveData(const QByteArray& sendData, QByteArray *receivedData, quint8 chipSelectBits);


private:
        Cheetah m_handle;
};

#endif // CHEETAHSPI_H

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

#ifndef SCRIPTCHEETAHSPI_H
#define SCRIPTCHEETAHSPI_H

#include <QObject>
#include "cheetahspi.h"
#include "scriptObject.h"

///This wrapper class is used to access a CheetahSpi object from a script.
class ScriptCheetahSpi : public QObject, public ScriptObject
{
    Q_OBJECT

    ///Returns a semicolon separated list with all public functions, signals and properties.
    Q_PROPERTY(QString publicScriptElements READ getPublicScriptElements)
public:
    explicit ScriptCheetahSpi(QObject *parent = 0) : QObject(parent), m_interface(this)
    {
    }

    ///Returns a semicolon separated list with all public functions, signals and properties.
    virtual QString getPublicScriptElements(void)
    {
        return MainWindow::parseApiFile("ScriptCheetahSpi.api");
    }

    ///Returns a string which contains informations about all detected devices.
    Q_INVOKABLE QString detectDevices(void){return CheetahSpi::detectDevices();}

    ///Connects to a cheetah spi interface (the baudrate is in kHz).
    Q_INVOKABLE bool connect(quint32 port, qint16 mode, quint32 baudrate){return m_interface.connectToDevice(port, mode, baudrate);}


    ///Disconnects from the cheetah spi interface.
    Q_INVOKABLE void disconnect(void){m_interface.disconnect();}

    ///Sends and receive data with the cheetah spi interface
    ///(the received data must be read with readAll).
    Q_INVOKABLE bool sendReceiveData(QVector<unsigned char> sendData, quint8 chipSelect)
    {
        m_receivedData.clear();
        QByteArray array;
        for(auto val : sendData)
        {
            array.push_back((unsigned char) val);
        }
        return m_interface.sendReceiveData(array, &m_receivedData, chipSelect);

    }

    ///Returns all received data from the last sendReceiveData call.
    Q_INVOKABLE QVector<unsigned char> readAll(void)
    {
        QVector<unsigned char> dataVector;

        for(auto val : m_receivedData)
        {
            dataVector.push_back((unsigned char) val);
        }
        m_receivedData.clear();
        return dataVector;
    }

private:
    ///The wrapped cheetah spi interface.
    CheetahSpi m_interface;

    ///The received data in the last sendReceiveData call.
    QByteArray m_receivedData;

};

#endif // SCRIPTCHEETAHSPI_H

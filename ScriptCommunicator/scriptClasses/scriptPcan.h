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

#ifndef SCRIPTPCAN_H
#define SCRIPTPCAN_H

#include <QObject>
#include "PCANBasicClass.h"
#include "scriptObject.h"

///This wrapper class is used to access a pcan interface from a script.
class ScriptPcan : public QObject, public ScriptObject
{
    Q_OBJECT

    ///Returns a semicolon separated list with all public functions, signals and properties.
    Q_PROPERTY(QString publicScriptElements READ getPublicScriptElements)
public:
    explicit ScriptPcan(QObject *parent = 0) : QObject(parent), m_pcan(this)
    {
        //connect the necessary signals with the wrapper slots (in this slots the
        //events of the wrapper class are generated, the script can connect to this
        //wrapper events)
         connect(&m_pcan, SIGNAL(readyRead()),this, SLOT(stub_readyReadSlot()));
    }

    ///Returns a semicolon separated list with all public functions, signals and properties.
    virtual QString getPublicScriptElements(void)
    {
        return MainWindow::parseApiFile("ScriptPcanInterface.api");
    }

    /**
     * Opens a pcan interface.
     * @param channel
     *      The pcan channel.
     * @param baudRate
     *      The baudrate. Possible values are:
     *      1000, 800, 500, 250, 125,100,95,83,50,47,33,20,10,5.
     * @param busOffAutoReset
     *      True if the PCAN driver shall reset automatically the CAN controller of a PCAN Channel if a bus-off state is detected.
     * @param powerSupply
     *      True if the external 5V on the D-Sub connector shall be switched on.
     * @return
     *      True on success.
     */
    Q_INVOKABLE bool open(quint8 channel, quint32 baudrate, bool busOffAutoReset, bool powerSupply)
    {
        quint16 convertedBaudrate = PCANBasicClass::convertBaudrateString(QString("%1").arg(baudrate));
        bool result = false;

        if(convertedBaudrate != 0)
        {
            result =  m_pcan.open(channel, convertedBaudrate, busOffAutoReset,powerSupply);
        }

        return result;
    }

    ///Closes the pcan interface.
    Q_INVOKABLE void close(void){m_pcan.close();}

    /**
     * Configures the reception filter.
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
    Q_INVOKABLE bool setFilter(bool filterExtended, quint32 filterFrom, quint32 filterTo){return m_pcan.setFilter(filterExtended, filterFrom,filterTo);}


    /**
     * Sends a can message. If more then 8 data bytes are given several can messages with the same can id will be sent.
     * @param type
     *  The can message type: 0=standard, 1=standard remote-transfer-request, 2=extended,
     *  3= extended remote-transfer-request
     * @param canId
     *      The can id.
     * @param data
     *      The can data.
     * @return
     *      True on success.
     */
    Q_INVOKABLE bool sendCanMessage(quint8 type, quint32 canId, QVector<unsigned char> data)
    {
        bool success = false;
        if(!data.isEmpty())
        {
            for(int i = 0; i < data.length(); i += 8)
            {
                success = m_pcan.sendCanMessage(type, canId, data.mid(i, 8));
                if(!success)
                {
                    break;
                }
            }
        }
        else
        {
            success = m_pcan.sendCanMessage(type, canId, data);
        }
        return success;
    }

    ///Returns true if connected to a pcan interface.
    Q_INVOKABLE bool isConnected(void){return m_pcan.isConnected();}

    ///Returns the current status as string.
    Q_INVOKABLE QString getStatusString(void){return m_pcan.getStatusString();}

    ///Returns the current status.
    Q_INVOKABLE quint32 getCurrentStatus(void){return m_pcan.getCurrentStatus();}

    /**
     * Reads a pcan parameter.
     *
     * @param parameter
     *  The parameter. Possible values are:
     *  - 0x01=PCAN_DEVICE_NUMBER
     *  - 0x02=PCAN_5VOLTS_POWER
     *  - 0x07=PCAN_BUSOFF_AUTORESET
     *  - 0x08=PCAN_LISTEN_ONLY
     *  - 0x0F=PCAN_RECEIVE_STATUS
     *  - 0x10=PCAN_CONTROLLER_NUMBER
     *  - 0x15=PCAN_CHANNEL_IDENTIFYING
     *
     * result:
     *  - Byte 0: status(1=success, 0=failure)
     *  - Byte 1: parameter value
     */
    Q_INVOKABLE QList<quint8> getCanParameter(quint8 parameter)
    {
        quint32 result = 0;
        QList<quint8> resultList;
        if(PCAN_ERROR_OK == m_pcan.getValue(parameter, &result, sizeof(result)))
        {
            resultList.push_back(1);
        }
        else
        {
            resultList.push_back(0);
        }
        resultList.push_back((quint8) result);
        return resultList;
    }

    /**
     * Sets a pcan parameter.
     *
     * @param parameter
     *  The parameter. Possible values are:
     *  - 0x01=PCAN_DEVICE_NUMBER
     *  - 0x02=PCAN_5VOLTS_POWER
     *  - 0x07=PCAN_BUSOFF_AUTORESET
     *  - 0x08=PCAN_LISTEN_ONLY
     *  - 0x0F=PCAN_RECEIVE_STATUS
     *  - 0x15=PCAN_CHANNEL_IDENTIFYING
     * @param data
     *      The new parameter value.
     */
    Q_INVOKABLE bool setCanParameter(quint8 parameter, quint8 data)
    {
        return (m_pcan.setValue(parameter, (void*)&data, sizeof(data)) == PCAN_ERROR_OK) ? true : false;
    }



signals:
    ///This signal is emitted if a can message (or several) has been received with the main interface.
    ///Types: 0=standard, 1=standard remote-transfer-request, 2=extended, 3= extended remote-transfer-request
    ///Scripts can connect a function to this signal.
    void canMessagesReceivedSignal(QVector<quint8> types, QVector<quint32> messageIds, QVector<quint32> timestamps,
                                   QVector<QVector<unsigned char>>  data);

private slots:

     ///This slot function is called if can message are available for reading.
    void stub_readyReadSlot()
    {
        QByteArray data = m_pcan.readLastMessage();
        QVector<QByteArray> messages;
        while(!data.isEmpty())
        {
            messages.append(data);
            data = m_pcan.readLastMessage();
        }

        if(!messages.isEmpty())
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

                quint32 messageId = ((dataVector[1] << 24) & 0xff000000) + ((dataVector[2] << 16) & 0xff0000) +
                        ((dataVector[3] << 8) & 0xff00) + (dataVector[4] & 0xff);
                quint32 timeStamp = ((dataVector[5] << 24) & 0xff000000) + ((dataVector[6] << 16) & 0xff0000) +
                        ((dataVector[7] << 8) & 0xff00) + (dataVector[8] & 0xff);

                types.push_back(type);
                messageIds.push_back(messageId);
                timestamps.push_back(timeStamp);
                data.push_back(dataVector.mid(9));

            }

            emit canMessagesReceivedSignal(types, messageIds, timestamps, data);
        }
    }

private:
    ///The wrapped pcan interface.
    PCANBasicClass m_pcan;

};

#endif // SCRIPTPCAN_H

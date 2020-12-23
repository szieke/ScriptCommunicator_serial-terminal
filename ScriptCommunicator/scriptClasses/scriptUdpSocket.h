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

#ifndef SCRIPTUDPSOCKET_H
#define SCRIPTUDPSOCKET_H

#include <QUdpSocket>
#include <mainInterfaceThread.h>
#include "scriptTcpClient.h"
#include <QNetworkProxy>
#include <scriptSerialPort.h>
#include "scriptObject.h"

///This wrapper class is used to access a QUdpSocket object from a script.
class ScriptUdpSocket: public QObject, public ScriptObject
{
    Q_OBJECT

    ///Returns a semicolon separated list with all public functions, signals and properties.
    Q_PROPERTY(QString publicScriptElements READ getPublicScriptElements)

public:
    ScriptUdpSocket(QObject* parent, MainInterfaceThread* interfaceThread) : QObject(parent),
        m_mainInterfaceThread(interfaceThread), m_interfaceIsPaused(false)
    {

       m_socket.setProxy(QNetworkProxy::NoProxy);

        //connect the necessary signals with the wrapper slots (in this slots the
        //events of the wrapper class are generated, the script can connect to this
        //wrapper events)
        connect(&m_socket, SIGNAL(readyRead()),this, SLOT(stub_readyReadSlot()));
        connect(this, SIGNAL(sendDataWithMainInterfaceSignal(const QByteArray, uint)),
                m_mainInterfaceThread, SLOT(sendDataSlot(const QByteArray, uint)));

        connect(parent, SIGNAL(pauseAllCreatedInterfaces(bool)),this, SLOT(pauseInterfaceSlot(bool)));
    }

    ///Returns a semicolon separated list with all public functions, signals and properties.
    virtual QString getPublicScriptElements(void)
    {
        return MainWindow::parseApiFile("ScriptUdpSocket.api");
    }

    ///Binds the socket to the port.
    Q_INVOKABLE bool bind(quint16 port){return m_socket.bind(QHostAddress::Any, port);}

    ///Closes the socket.
    Q_INVOKABLE void close(void){m_socket.close();}

    ///Returns true if a received datagram can be read from the socket.
    Q_INVOKABLE bool hasPendingDatagrams(void){return m_socket.hasPendingDatagrams();}

    ///Returns the data from one received datagram.
    Q_INVOKABLE QVector<unsigned char> readDatagram(void)
    {
        QVector<unsigned char> datagram;

        if(m_socket.hasPendingDatagrams())
        {
            datagram.resize(m_socket.pendingDatagramSize());
            QHostAddress sender;
            quint16 senderPort;
            m_socket.readDatagram((char*)datagram.data(), datagram.size(),
                                  &sender, &senderPort);
        }
        return datagram;
    }

    ///Reads all received datagrams (the data from the single datagrams are
    ///inserted in one unsigned char vector)
    Q_INVOKABLE QVector<unsigned char> readAll(void)
    {
        QVector<unsigned char> data;

        while(m_socket.hasPendingDatagrams())
        {
            QVector<unsigned char> datagram;
            datagram.resize(m_socket.pendingDatagramSize());
            QHostAddress sender;
            quint16 senderPort;
            m_socket.readDatagram((char*)datagram.data(), datagram.size(),
                                  &sender, &senderPort);

            data += datagram;
            datagram.clear();
        }
        return data;
    }

    ///Writes data to the socket. Returns the number of written bytes.
    Q_INVOKABLE quint64 write(QVector<unsigned char> data, QString hostAdress,
                                      quint16 hostPort)
    {
        quint64 writtenBytes = 0;
        const char* constData = (const char *)data.constData();
        for(int i = 0; i < data.length(); i += MainInterfaceThread::UDP_MAX_SEND_SIZE)
        {
            int bytesToWrite = ((i + MainInterfaceThread::UDP_MAX_SEND_SIZE) < data.length()) ? MainInterfaceThread::UDP_MAX_SEND_SIZE :
                                                                                              data.length() - i;

            if(bytesToWrite != m_socket.writeDatagram(&constData[i], bytesToWrite, QHostAddress(hostAdress), hostPort))
            {
                break;
            }
            writtenBytes += bytesToWrite;
        }

        return writtenBytes;
    }

    ///Writes a string to the socket. Returns the number of written bytes.
    Q_INVOKABLE qint64 writeString(QString string, QString hostAdress,
                                   quint16 hostPort)
    {
        QVector<unsigned char> dataVector;
        QByteArray data = string.toUtf8();
        for(auto val : data)
        {
            dataVector.push_back((unsigned char) val);
        }
         return write(dataVector, hostAdress, hostPort);
    }

    ///Returns true if the UDP socket is open/listening.
    Q_INVOKABLE bool isOpen(void){return m_socket.isOpen();}

    ///Enables the main interface routing (all data from the main interface is send with this socket and
    ///all received (with this socket) data is sent with the main interace).
    Q_INVOKABLE void enableMainInterfaceRouting(QString routingHostAddress, quint16 routingHostPort)
    {
        m_routingHostAddress = QHostAddress(routingHostAddress);
        m_routingHostPort = routingHostPort;

        connect(&m_socket, SIGNAL(readyRead()),this, SLOT(udpSocketOnReadyReadSlot()));
        connect(m_mainInterfaceThread, SIGNAL(dataReceivedSignal(QByteArray)),
                this, SLOT(mainInterfaceReceivedSlot(QByteArray)), Qt::QueuedConnection);

    }

    ///Disables the main interface routing.
    Q_INVOKABLE void disableMainInterfaceRouting()
    {
        disconnect(&m_socket, SIGNAL(readyRead()),this, SLOT(udpSocketOnReadyReadSlot()));
        disconnect(m_mainInterfaceThread, SIGNAL(dataReceivedSignal(QByteArray)),
                   this, SLOT(mainInterfaceReceivedSlot(QByteArray)));
    }

    ///This function checks if a data line (ends with EOL ('\n')) is ready to be read.
    Q_INVOKABLE bool canReadLine(void){return m_socket.canReadLine();}

    ///This function reads a line (a line ends with a '\n') of ASCII characters.
    ///If removeNewLine is true then the '\n' will not returned (is removed from the received line).
    ///If removeCarriageReturn is true then a '\r' in front of '\n' will also not returned.
    ///Note: If no new data line is ready for reading this functions returns an empty string.
    Q_INVOKABLE QString readLine(bool removeNewLine=true, bool removeCarriageReturn=true)
    {
        QString result;
        if(m_socket.canReadLine())
        {
            result = ScriptSerialPort::readLineInternally(&m_socket, removeNewLine, removeCarriageReturn);
        }
        return result;
    }

    ///This function reads all available lines (a line ends with a '\n') of ASCII characters.
    ///If removeNewLine is true then the '\n' will not returned (is removed from the received line).
    ///If removeCarriageReturn is true then a '\r' in front of '\n' will also not returned.
    ///Note: If no new data line is ready for reading this functions returns an empty list.
    Q_INVOKABLE QStringList readAllLines(bool removeNewLine=true, bool removeCarriageReturn=true)
    {
        return ScriptSerialPort::readAllLinesInternally(&m_socket, removeNewLine, removeCarriageReturn);
    }

signals:
    ///This signal is emitted if data can be read from the socket (if a datagram has been received).
    ///Scripts can connect a function to this signal.
    void readyReadSignal(void);

    ///Is connected with MainInterfaceThread::sendData (sends data with the main interface).
    ///This signal must not be used from script.
    void sendDataWithMainInterfaceSignal(const QByteArray data, uint id);

private slots:

    ///This slot function is called if data can be read from the socket (if a datagram has been received).
    void stub_readyReadSlot(void)
    {
        if(!m_interfaceIsPaused)
        {
            emit readyReadSignal();
        }
        else
        {
            readAll();
        }
    }

    /**
     * This slot function is called of the UDP socket has received data abd the routing is enabled.
     */
    void udpSocketOnReadyReadSlot()
    {
        if(!m_interfaceIsPaused)
        {
            while (m_socket.hasPendingDatagrams())
            {
                QByteArray datagram;
                datagram.resize(m_socket.pendingDatagramSize());
                QHostAddress sender;
                quint16 senderPort;

                m_socket.readDatagram(datagram.data(), datagram.size(),
                                                &sender, &senderPort);
                emit sendDataWithMainInterfaceSignal(datagram, MainInterfaceThread::SEND_ID_ROUTING);
            }
        }
        else
        {
            (void)readAll();
        }
    }

    /**
     * Is called, if data from the main interface (MainInterfaceThread) has been received.
     * @param data
     *      The received data.
     */
    void mainInterfaceReceivedSlot(QByteArray data)
    {
        if(!m_interfaceIsPaused)
        {
            for(int i = 0; i < data.length(); i += 512)
            {
                QByteArray subArray = data.mid(i, 512);
                if(subArray.size() !=m_socket.writeDatagram(subArray, m_routingHostAddress, m_routingHostPort))
                {
                    break;
                }
            }
        }
    }

    ///If pause is true, all data from this interface is dicarded.
    void pauseInterfaceSlot(bool pause){m_interfaceIsPaused = pause;}


private:
    ///The wrapped udp socket.
    QUdpSocket m_socket;

    ///Pointer to the main interface.
    MainInterfaceThread* m_mainInterfaceThread;

    ///The main interface routing host address;
    QHostAddress m_routingHostAddress;

    ///The main interface routing host port.
    quint16 m_routingHostPort;

    ///If m_interfaceIsPaused is true, all data from this interface is dicarded.
    bool m_interfaceIsPaused;
};

#endif // SCRIPTUDPSOCKET_H

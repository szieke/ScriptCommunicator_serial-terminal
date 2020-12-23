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

#ifndef SCRIPTTCPCLIENT_H
#define SCRIPTTCPCLIENT_H

#include <QTcpSocket>
#include <QHostAddress>
#include <mainInterfaceThread.h>
#include <QNetworkProxy>
#include <scriptSerialPort.h>
#include "scriptObject.h"


///This wrapper class is used to access a QTcpSocket (tcp client) object from a script.
class ScriptTcpClient: public QObject, public ScriptObject
{
    Q_OBJECT

    ///Returns a semicolon separated list with all public functions, signals and properties.
    Q_PROPERTY(QString publicScriptElements READ getPublicScriptElements)

public:
    ScriptTcpClient(QTcpSocket* socket, QObject *parent, MainInterfaceThread* interfaceThread) :
        QObject(parent), m_tcpSocket(socket), m_mainInterfaceThread(interfaceThread), m_interfaceIsPaused(false)
    {
        m_tcpSocket->setProxy(QNetworkProxy::NoProxy);

        //connect the necessary signals with the wrapper slots (in this slots the
        //events of the wrapper class are generated, the script can connect to this
        //wrapper events)
        connect(m_tcpSocket, SIGNAL(connected()),this, SIGNAL(connectedSignal()));
        connect(m_tcpSocket, SIGNAL(disconnected()),this, SIGNAL(disconnectedSignal()));
        connect(m_tcpSocket, SIGNAL(readyRead()),this, SLOT(stub_readyReadSlot()));
        connect(m_tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)),this, SLOT(stub_errorSlot(QAbstractSocket::SocketError)));

        connect(this, SIGNAL(sendDataWithMainInterfaceSignal(const QByteArray, uint)),
                m_mainInterfaceThread, SLOT(sendDataSlot(const QByteArray, uint)));

        connect(parent, SIGNAL(pauseAllCreatedInterfaces(bool)),this, SLOT(pauseInterfaceSlot(bool)));
    }
    virtual ~ScriptTcpClient()
    {
    }

    ///Returns a semicolon separated list with all public functions, signals and properties.
    virtual QString getPublicScriptElements(void)
    {
        return MainWindow::parseApiFile("ScriptTcpClient.api");
    }

    ///This function connects the socket to a tcp server.
    Q_INVOKABLE void connectToHost(QString hostAdress, quint16 port)
    {
        QHostAddress adress(hostAdress);
        m_tcpSocket->connectToHost(adress, port);
    }

    ///This function closes the socket.
    Q_INVOKABLE void close(void){m_tcpSocket->close();}

    ///Returns true if data can be read from the socket (if data has been received).
    Q_INVOKABLE bool isReadable(void){return m_tcpSocket->isReadable();}

    ///Returns the number of bytes which are available for reading.
    Q_INVOKABLE quint64 bytesAvailable(void){return m_tcpSocket->bytesAvailable();}

    ///This function returns all received bytes.
    Q_INVOKABLE QVector<unsigned char> readAll(void)
    {
        QVector<unsigned char> dataVector;
        dataVector.resize(m_tcpSocket->bytesAvailable());
        (void)m_tcpSocket->read((char*)dataVector.constData(), dataVector.size());
        return dataVector;
    }

    ///Writes data to the socket. Returns the number of written bytes.
    Q_INVOKABLE qint64 write( QVector<unsigned char> dataVector)
    {
        return m_tcpSocket->write((const char *)dataVector.constData(), dataVector.size());
    }

    ///Writes a string to the socket. Returns the number of written bytes.
    Q_INVOKABLE qint64 writeString(QString string)
    {
        return m_tcpSocket->write(string.toUtf8());
    }

    ///Returns true if the TCP client is open/connected.
    Q_INVOKABLE bool isOpen(void){return m_tcpSocket->isOpen();}

    ///Returns a human-readable description of the last error that has been occurred.
    Q_INVOKABLE QString getErrorString(void){return m_tcpSocket->errorString();}

    ///Enables the main interface routing (all data from the main interface is send with this socket and
    ///all received (with this socket) data is sent with the main interface).
    Q_INVOKABLE void enableMainInterfaceRouting()
    {
        connect(m_tcpSocket, SIGNAL(readyRead()),this, SLOT(tcpClientSocketOnReadyReadSlot()));
        connect(m_mainInterfaceThread, SIGNAL(dataReceivedSignal(QByteArray)),
                this, SLOT(mainInterfaceReceivedSlot(QByteArray)), Qt::QueuedConnection);

    }

    ///Disables the main interface routing.
    Q_INVOKABLE void disableMainInterfaceRouting()
    {
        disconnect(m_tcpSocket, SIGNAL(readyRead()),this, SLOT(tcpClientSocketOnReadyReadSlot()));
        disconnect(m_mainInterfaceThread, SIGNAL(dataReceivedSignal(QByteArray)),
                   this, SLOT(mainInterfaceReceivedSlot(QByteArray)));

    }

    ///Sets the proxy of the TCP client. Possible values for proxyType are:
    ///- NO_PROXY
    ///- SYSTEM_PROXY
    ///- CUSTOM_PROXY
    Q_INVOKABLE void setProxy(QString proxyType = "NO_PROXY", QString proxyUserName= "", QString proxyPassword = "",
                              QString proxyIpAddress = "", quint16 proxyPort = 0)
    {
        QNetworkProxy proxy = createProxy(proxyType, proxyUserName, proxyPassword, proxyIpAddress, proxyPort);
        m_tcpSocket->setProxy(proxy);

    }

    ///Creates a network proxy. Possible values for proxyType are:
    ///- NO_PROXY
    ///- SYSTEM_PROXY
    ///- CUSTOM_PROXY
    static QNetworkProxy createProxy(QString proxyType = "NO_PROXY", QString proxyUserName= "", QString proxyPassword = "",
                                     QString proxyIpAddress = "", quint16 proxyPort = 0)
    {
        QNetworkProxy proxy = QNetworkProxy::NoProxy;

        if(proxyType == "SYSTEM_PROXY")
        {
            QNetworkProxyQuery npq(QUrl("www.google.com"));
            QList<QNetworkProxy> listOfProxies = QNetworkProxyFactory::systemProxyForQuery(npq);
            if (listOfProxies.size())
            {
                if(!listOfProxies[0].hostName().isEmpty())
                {
                    proxy = QNetworkProxy(QNetworkProxy::HttpProxy, listOfProxies[0].hostName(), listOfProxies[0].port(),
                            proxyUserName, proxyPassword);
                }
                else
                {
                    proxy = QNetworkProxy::NoProxy;
                }
            }
        }
        else if(proxyType == "CUSTOM_PROXY")
        {
            proxy = QNetworkProxy(QNetworkProxy::HttpProxy, proxyIpAddress, proxyPort, proxyUserName, proxyPassword);
        }
        else
        {
            proxy = QNetworkProxy::NoProxy;

        }

        return proxy;
    }

    ///This function checks if a data line (ends with EOL ('\n')) is ready to be read.
    Q_INVOKABLE bool canReadLine(void){return m_tcpSocket->canReadLine();}

    ///This function reads a line (a line ends with a '\n') of ASCII characters.
    ///If removeNewLine is true then the '\n' will not returned (is removed from the received line).
    ///If removeCarriageReturn is true then a '\r' in front of '\n' will also not returned.
    ///Note: If no new data line is ready for reading this functions returns an empty string.
    Q_INVOKABLE QString readLine(bool removeNewLine=true, bool removeCarriageReturn=true)
    {
        QString result;
        if(m_tcpSocket->canReadLine())
        {
            result = ScriptSerialPort::readLineInternally(m_tcpSocket, removeNewLine, removeCarriageReturn);
        }
        return result;
    }

    ///This function reads all available lines (a line ends with a '\n') of ASCII characters.
    ///If removeNewLine is true then the '\n' will not returned (is removed from the received line).
    ///If removeCarriageReturn is true then a '\r' in front of '\n' will also not returned.
    ///Note: If no new data line is ready for reading this functions returns an empty list.
    Q_INVOKABLE QStringList readAllLines(bool removeNewLine=true, bool removeCarriageReturn=true)
    {
        return ScriptSerialPort::readAllLinesInternally(m_tcpSocket, removeNewLine, removeCarriageReturn);
    }

signals:
    ///This signal is emitted if the connection has been established.
    ///Scripts can connect a function to this signal.
    void connectedSignal(void);

    ///This signal is emitted if the connection has been disconnected.
    ///Scripts can connect a function to this signal.
    void disconnectedSignal(void);

    ///This signal is emitted if data can be read from the socket (if data has been received).
    ///Scripts can connect a function to this signal.
    void readyReadSignal(void);

    ///This signal is emitted after an error has been occurred.
    ///The error parameter describes the type of error that has been occurred.
    ///Scripts can connect a function to this signal.
    void errorSignal(int error);

    ///Is connected with MainInterfaceThread::sendData (sends data with the main interface).
    ///This signal must not be used from script.
    void sendDataWithMainInterfaceSignal(const QByteArray data, uint id);

private slots:

    ///This slot function is called if data can be read from the socket (if data has been received).
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

    ///This slot function is called if an error has been ocurred.
    void stub_errorSlot(QAbstractSocket::SocketError error){emit errorSignal((int)error);}

    /**
     * This slot function is called of the tcp client has received data abd the routing is enabled.
     */
    void tcpClientSocketOnReadyReadSlot()
    {
        if(m_tcpSocket->isReadable())
        {
            if(!m_interfaceIsPaused)
            {
                QByteArray data = m_tcpSocket->readAll();
                emit sendDataWithMainInterfaceSignal(data, MainInterfaceThread::SEND_ID_ROUTING);
            }
            else
            {
                (void)readAll();
            }
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
            for(int i = 0; i < data.length(); i += 50000)
            {
                QByteArray subArray = data.mid(i, 50000);
                if(subArray.size() != m_tcpSocket->write(subArray))
                {
                    break;
                }
                m_tcpSocket->waitForBytesWritten();
            }
        }
    }

    ///If pause is true, all data from this interface is dicarded.
    void pauseInterfaceSlot(bool pause){m_interfaceIsPaused = pause;}


private:
    ///The wrapped tcp socket.
    QTcpSocket* m_tcpSocket;

    ///Pointer to the main interface.
    MainInterfaceThread* m_mainInterfaceThread;

    ///If m_interfaceIsPaused is true, all data from this interface is dicarded.
    bool m_interfaceIsPaused;
};

#endif // SCRIPTTCPCLIENT_H

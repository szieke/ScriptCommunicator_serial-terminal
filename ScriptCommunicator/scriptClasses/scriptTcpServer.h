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

#ifndef SCRIPTTCPSERVER_H
#define SCRIPTTCPSERVER_H

#include <QObject>
#include <QTcpServer>
#include "scriptTcpClient.h"
#include <mainInterfaceThread.h>
#include <QNetworkProxy>
#include <QScriptable>
#include "scriptObject.h"

///This wrapper class is used to access a QTcpServer object from a script.
class ScriptTcpServer : public QObject, protected QScriptable, public ScriptObject
{
    Q_OBJECT

    ///Returns a semicolon separated list with all public functions, signals and properties.
    Q_PROPERTY(QString publicScriptElements READ getPublicScriptElements)
public:
    explicit ScriptTcpServer(QObject *parent, MainInterfaceThread* interfaceThread) : QObject(parent),
        m_mainInterfaceThread(interfaceThread), m_interfaceIsPaused(false)
    {
        m_tcpServer.setProxy(QNetworkProxy::NoProxy);

        //connect the necessary signals with the wrapper slots (in this slots the
        //events of the wrapper class are generated, the script can connect to this
        //wrapper events)
         connect(&m_tcpServer, SIGNAL(newConnection()),this, SLOT(stub_newConnectionSlot()));

         connect(parent, SIGNAL(pauseAllCreatedInterfaces(bool)),this, SLOT(pauseInterfaceSlot(bool)));
    }

    ///Returns a semicolon separated list with all public functions, signals and properties.
    virtual QString getPublicScriptElements(void)
    {
        return MainWindow::parseApiFile("ScriptTcpServer.api");
    }

    ///Call this function to start listening for new connections.
    Q_INVOKABLE bool listen(quint16 port){ return m_tcpServer.listen(QHostAddress::Any, port);}

    ///Returns true if the socket is listening for new connections.
    Q_INVOKABLE bool isListening(void){return m_tcpServer.isListening();}

    ///Set the max. pending connections.
    Q_INVOKABLE void setMaxPendingConnections(int numConnections){m_tcpServer.setMaxPendingConnections(numConnections);}

    ///Returns the max. pending connections.
    Q_INVOKABLE int maxPendingConnections(void){return m_tcpServer.maxPendingConnections();}

    ///This function closes the tcp server.
    Q_INVOKABLE void close(){m_tcpServer.close();}

    ///Returns true if the server has a pending connection; otherwise returns false.
    Q_INVOKABLE bool hasPendingConnections(void){return m_tcpServer.hasPendingConnections();}

    ///Return the next pending connection (returns a script TCP client).
    Q_INVOKABLE QScriptValue nextPendingConnection(void)
    {
        QScriptValue result;

        if( m_tcpServer.hasPendingConnections())
        {
            QTcpSocket* pendingSocket = m_tcpServer.nextPendingConnection();
            if(pendingSocket)
            {
                ScriptTcpClient * socket = new ScriptTcpClient(pendingSocket, parent(),  m_mainInterfaceThread);
                result =  engine()->newQObject(socket, QScriptEngine::ScriptOwnership);
            }
        }

        return result;
    }


signals:
    ///This signal is emitted if a new connection has been established.
    ///Scripts can connect a function to this signal.
    void newConnectionSignal(void);

private slots:
    ///This slot function is called if a new connection has been established.
    void stub_newConnectionSlot(void)
    {
        if(!m_interfaceIsPaused)
        {
            emit newConnectionSignal();
        }
        else
        {
            QTcpSocket* socket = m_tcpServer.nextPendingConnection();
            socket->close();
            socket->deleteLater();
        }
    }


    ///If pause is true, all data from this interface is dicarded.
    void pauseInterfaceSlot(bool pause){m_interfaceIsPaused = pause;}


private:
    ///The wrapped tcp server.
    QTcpServer m_tcpServer;

    ///Pointer to the main interface.
    MainInterfaceThread* m_mainInterfaceThread;

    ///If m_interfaceIsPaused is true, all data from this interface is dicarded.
    bool m_interfaceIsPaused;

};

#endif // SCRIPTTCPSERVER_H

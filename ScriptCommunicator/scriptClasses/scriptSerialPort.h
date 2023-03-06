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

#ifndef SCRIPTSERIALPORT_H
#define SCRIPTSERIALPORT_H

#include <QObject>
#include <QtSerialPort/QSerialPort>
#include <QVector>
#include <mainInterfaceThread.h>
#include "scriptObject.h"

///This wrapper class is used to access a QSerialPort object from a script.
class ScriptSerialPort : public QObject, public ScriptObject
{
    Q_OBJECT

    ///Returns a semicolon separated list with all public functions, signals and properties.
    Q_PROPERTY(QString publicScriptElements READ getPublicScriptElements)
public:
    explicit ScriptSerialPort(QObject *parent, MainInterfaceThread* interfaceThread) :
        QObject(parent), m_serialPort(), m_mainInterfaceThread(interfaceThread), m_interfaceIsPaused(false)
    {
        //connect the necessary signals with the wrapper slots (in this slots the
        //events of the wrapper class are generated, the script can connect to this
        //wrapper events)
        connect(&m_serialPort, SIGNAL(readyRead()),this, SLOT(stub_readyReadSlot()));

        connect(this, SIGNAL(sendDataWithMainInterfaceSignal(const QByteArray, uint)),
                m_mainInterfaceThread, SLOT(sendDataSlot(const QByteArray, uint)));

        connect(parent, SIGNAL(pauseAllCreatedInterfaces(bool)),this, SLOT(pauseInterfaceSlot(bool)));

        m_dtrIsSet = true;
        m_rtsIsSet = false;
    }
    virtual ~ScriptSerialPort()
    {
    }

    ///Returns a semicolon separated list with all public functions, signals and properties.
    virtual QString getPublicScriptElements(void)
    {
        return MainWindow::parseApiFile("ScriptSerialPort.api");
    }

    ///Sets the DTR pin.
    Q_INVOKABLE void setDTR(bool set)
    {
        m_dtrIsSet = set;

        if(m_serialPort.isOpen())
        {
            m_serialPort.setDataTerminalReady(m_dtrIsSet);
        }
    }

    ///Sets the RTS pin.
    Q_INVOKABLE void setRTS(bool set)
    {
        m_rtsIsSet = set;

        if(m_serialPort.isOpen())
        {
            m_serialPort.setRequestToSend(m_rtsIsSet);
        }
    }

    ///Sets the serial port name.
    Q_INVOKABLE void setPortName(const QString &name){m_serialPort.setPortName(name);}

    ///Returns the serial port name.
    Q_INVOKABLE QString portName(void){return m_serialPort.portName();}

    ///Sets the baudrate.
    Q_INVOKABLE bool setBaudRate(qint32 baudRate){return m_serialPort.setBaudRate(baudRate);}

    ///Returns the baudrate.
    Q_INVOKABLE qint32 baudRate(void){return m_serialPort.baudRate();}

    ///Sets the number of data bits.
    Q_INVOKABLE bool setDataBits(quint32 dataBits){return m_serialPort.setDataBits(static_cast<QSerialPort::DataBits>(dataBits));}

    ///Returns the number of data bits.
    Q_INVOKABLE quint32 dataBits(void){return static_cast<quint32>(m_serialPort.dataBits());}

    ///Sets the parity. Possible values are:
    ///- None
    ///- Even
    ///- Odd
    ///- Space
    ///- Mark
    Q_INVOKABLE bool setParity(QString parityString)
    {
        QSerialPort::Parity parity;

        if(parityString == "None")
        {
            parity = QSerialPort::NoParity;
        }
        else if(parityString == "Even")
        {
            parity = QSerialPort::EvenParity;
        }
        else if(parityString == "Odd")
        {
            parity = QSerialPort::OddParity;
        }
        else if(parityString == "Space")
        {
            parity = QSerialPort::SpaceParity;
        }
        else if(parityString == "Mark")
        {
            parity = QSerialPort::MarkParity;
        }
        else
        {
            parity = QSerialPort::NoParity;
        }
        return m_serialPort.setParity(parity);
    }

    ///Returns the parity. Possible values are:
    ///- None
    ///- Even
    ///- Odd
    ///- Space
    ///- Mark
    ///- Unknown
    Q_INVOKABLE QString parity(void)
    {
        QSerialPort::Parity parity = m_serialPort.parity();
        QString parityString;

        if(parity == QSerialPort::NoParity)
        {
            parityString = "None";
        }
        else if(parity == QSerialPort::EvenParity)
        {
            parityString = "Even";
        }
        else if(parity == QSerialPort::OddParity)
        {
            parityString = "Odd";
        }
        else if(parity == QSerialPort::SpaceParity)
        {
            parityString = "Space";
        }
        else if(parity == QSerialPort::MarkParity)
        {
            parityString = "Mark";
        }
        else
        {
            parityString = "Unknown";
        }
        return parityString;
    }


    ///Sets the number of stop bits. Possible values are:
    ///- 1
    ///- 1.5
    ///- 2
    Q_INVOKABLE bool setStopBits(QString stopBitsString)
    {
        QSerialPort::StopBits stopBits;
        if(stopBitsString == "1.5")
        {
            stopBits = QSerialPort::OneAndHalfStop;
        }
        else if(stopBitsString == "2")
        {
            stopBits = QSerialPort::TwoStop;
        }
        else
        {//1
            stopBits = QSerialPort::OneStop;
        }
        return m_serialPort.setStopBits(stopBits);
    }

    ///Returns the number of stop bits. Possible values are:
    ///- 1
    ///- 1.5
    ///- 2
    Q_INVOKABLE QString stopBits(void)
    {
        QSerialPort::StopBits stopBits = m_serialPort.stopBits();
        QString stopBitsString;

        if(stopBits == QSerialPort::OneAndHalfStop)
        {
            stopBitsString = "1.5";
        }
        else if(stopBits == QSerialPort::TwoStop)
        {
            stopBitsString = "2";
        }
        else
        {
            stopBitsString = "1";
        }
        return stopBitsString;
    }

    ///Sets the flow control. Possible values are:
    ///- RTS/CTS
    ///- XON/XOFF
    ///- None
    Q_INVOKABLE bool setFlowControl(QString flowString)
    {
        QSerialPort::FlowControl flow;

        if(flowString == "RTS/CTS")
        {
            flow = QSerialPort::HardwareControl;
        }
        else if(flowString == "XON/XOFF")
        {
            flow = QSerialPort::SoftwareControl;
        }
        else
        {//None
            flow = QSerialPort::NoFlowControl;
        }
        return m_serialPort.setFlowControl(flow);

    }

    ///Returns the flow control. Possible values are:
    ///- RTS/CTS
    ///- XON/XOFF
    ///- None
    Q_INVOKABLE QString flowControl(void)
    {
        QSerialPort::FlowControl flow = m_serialPort.flowControl();
        QString flowString;

        if(flow == QSerialPort::HardwareControl)
        {
            flowString = "RTS/CTS";
        }
        else if(flow == QSerialPort::SoftwareControl)
        {
            flowString = "XON/XOFF";
        }
        else
        {
            flowString = "None";
        }
        return flowString;
    }

    ///Returns the error string from the serial port (contains additional information
    ///in the case of an error).
    Q_INVOKABLE QString errorString(void){return m_serialPort.errorString();}

    ///Opens the serial port.
    Q_INVOKABLE bool open(void)
    {
        bool ret = m_serialPort.open(QIODevice::ReadWrite);
        if(ret)
        {
            m_serialPort.setDataTerminalReady(m_dtrIsSet);
        }

        return ret;
    }

    ///Closes the serial port.
    Q_INVOKABLE void close(void)
    {
        if(m_serialPort.isOpen())
        {
            m_serialPort.setDataTerminalReady(false);
            m_serialPort.setRequestToSend(false);
            m_serialPort.close();
        }
    }

    ///Returns the state of the serial port signals (pins).
    ///The signals are bit coded:
    ///NoSignal = 0x00,
    ///DataTerminalReadySignal = 0x04,
    ///DataCarrierDetectSignal = 0x08,
    ///DataSetReadySignal = 0x10,
    ///RingIndicatorSignal = 0x20,
    ///RequestToSendSignal = 0x40,
    ///ClearToSendSignal = 0x80,
    Q_INVOKABLE quint32 getSerialPortSignals(void)
    {
        return ((quint32)m_serialPort.pinoutSignals()) & 0xfc;
    }

    ///Returns true if the serial port is open.
    Q_INVOKABLE bool isOpen(void){return m_serialPort.isOpen();}

    ///Returns the number of bytes which are available for reading.
    Q_INVOKABLE qint64 bytesAvailable(void){return m_serialPort.bytesAvailable();}

    ///Returns all available received bytes.
    Q_INVOKABLE QVector<unsigned char> readAll()
    {
        QVector<unsigned char> dataVector;
        dataVector.resize(m_serialPort.bytesAvailable());
        (void)m_serialPort.read((char*)dataVector.constData(), dataVector.size());
        return dataVector;
    }

    ///Writes data to the serial port. Returns the number of written bytes.
    Q_INVOKABLE qint64 write(QVector<unsigned char>dataVector)
    {
        return m_serialPort.write((const char *)dataVector.constData(), dataVector.size());
    }

    ///Writes a string to the serial port. Returns the number of written bytes.
    Q_INVOKABLE qint64 writeString(QString string)
    {
         return m_serialPort.write(string.toUtf8());
    }

    ///Returns the number of bytes which are not written yet.
    Q_INVOKABLE qint64 bytesToWrite(void){return m_serialPort.bytesToWrite();}

    ///This function waits until all bytes have been written (sent) or the time in msec has been elapsed.
    Q_INVOKABLE bool waitForBytesWritten(int msecs){return m_serialPort.waitForBytesWritten(msecs);}

    ///Enables the main interface routing (all data from the main interface is send with this socket and
    ///all received (with this socket) data is sent with the main interace).
    Q_INVOKABLE void enableMainInterfaceRouting()
    {
        connect(&m_serialPort, SIGNAL(readyRead()),this, SLOT(serialPortOnReadyReadSlot()));
        connect(m_mainInterfaceThread, SIGNAL(dataReceivedSignal(QByteArray)),
                this, SLOT(mainInterfaceReceivedSlot(QByteArray)), Qt::QueuedConnection);

    }

    ///Disables the main interface routing.
    Q_INVOKABLE void disableMainInterfaceRouting()
    {
        disconnect(&m_serialPort, SIGNAL(readyRead()),this, SLOT(serialPortOnReadyReadSlot()));
        disconnect(m_mainInterfaceThread, SIGNAL(dataReceivedSignal(QByteArray)),
                   this, SLOT(mainInterfaceReceivedSlot(QByteArray)));

    }

    ///This function checks if a data line (ends with EOL ('\n')) is ready to be read.
    Q_INVOKABLE bool canReadLine(void){return m_serialPort.canReadLine();}

    ///This function reads a line (a line ends with a '\n') of ASCII characters.
    ///If removeNewLine is true then the '\n' will not returned (is removed from the received line).
    ///If removeCarriageReturn is true then a '\r' in front of '\n' will also not returned.
    ///Note: If no new data line is ready for reading this functions returns an empty string.
    Q_INVOKABLE QString readLine(bool removeNewLine=true, bool removeCarriageReturn=true)
    {
        QString result;
        if(m_serialPort.canReadLine())
        {
            result = readLineInternally(&m_serialPort, removeNewLine, removeCarriageReturn);
        }
        return result;
    }

    ///This function reads all available lines (a line ends with a '\n') of ASCII characters.
    ///If removeNewLine is true then the '\n' will not returned (is removed from the received line).
    ///If removeCarriageReturn is true then a '\r' in front of '\n' will also not returned.
    ///Note: If no new data line is ready for reading this functions returns an empty list.
    Q_INVOKABLE QStringList readAllLines(bool removeNewLine=true, bool removeCarriageReturn=true)
    {
        return readAllLinesInternally(&m_serialPort, removeNewLine, removeCarriageReturn);
    }

    ///This function reads a line (a line ends with a '\n') of ASCII characters from an QIODevice object.
    ///If removeNewLine is true then the '\n' will not returned (is removed from the received line).
    ///If removeCarriageReturn is true then a '\r' in front of '\n' will also not returned.
    ///Note: If no new data line is ready for reading this functions returns an empty string.
    static inline QString readLineInternally(QIODevice* ioDevice, bool removeNewLine=true, bool removeCarriageReturn=true)
    {
        QByteArray data = ioDevice->readLine();
        int size = data.size();
        if(removeCarriageReturn && (size >= 2))
        {
            if(data.at(size-2) == '\r')
            {
                //Remove '\r'.
                data.remove(size-2, 1);
                size--;
            }
        }
        if(removeNewLine)
        {
            //Remove '\n'.
            data.remove(size-1 ,1);
        }
        return data;
    }

    ///This function reads all available lines (a line ends with a '\n') of ASCII characters from an QIODevice object.
    ///If removeNewLine is true then the '\n' will not returned (is removed from the received line).
    ///If removeCarriageReturn is true then a '\r' in front of '\n' will also not returned.
    ///Note: If no new data line is ready for reading this functions returns an empty list.
    static inline QStringList readAllLinesInternally(QIODevice* ioDevice, bool removeNewLine=true, bool removeCarriageReturn=true)
    {
        QStringList result;
        while(ioDevice->canReadLine())
        {
            result.append(readLineInternally(ioDevice, removeNewLine, removeCarriageReturn));
        }
        return result;
    }


Q_SIGNALS:
    ///This signal is emitted if data is available for reading (if data has been received).
    ///Scripts can connect a function to this signal.
    void readyReadSignal(void);

    ///Is connected with MainInterfaceThread::sendData (sends data with the main interface).
    ///This signal must not be used from script.
    void sendDataWithMainInterfaceSignal(const QByteArray data, uint id);

private slots:

     ///This slot function is called if data is available for reading (if data has been received).
    void stub_readyReadSlot()
    {
        if(!m_interfaceIsPaused)
        {
            emit readyReadSignal();
        }
        else
        {
            m_serialPort.clear();
        }
    }


    /**
     * This slot function is called of the tcp client has received data abd the routing is enabled.
     */
    void serialPortOnReadyReadSlot()
    {
        if(m_serialPort.isReadable())
        {
            if(!m_interfaceIsPaused)
            {
                QByteArray data = m_serialPort.readAll();
                emit sendDataWithMainInterfaceSignal(data, MainInterfaceThread::SEND_ID_ROUTING);
            }
            else
            {
                m_serialPort.clear();
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
            for(int i = 0; i < data.length(); i += 512)
            {
                QByteArray subArray = data.mid(i, 512);
                if(subArray.size() != m_serialPort.write(subArray))
                {
                    break;
                }
                m_serialPort.waitForBytesWritten(MainInterfaceThread::SEND_TIMEOUT);
            }
        }
    }

    ///If pause is true, all data from this interface is dicarded.
    void pauseInterfaceSlot(bool pause){m_interfaceIsPaused = pause;}


private:

    ///The wrapped serial port.
    QSerialPort m_serialPort;

    ///True, if the DTR pin is set.
    bool m_dtrIsSet;


    ///True, if the RTS pin is set.
    bool m_rtsIsSet;

    ///Pointer to the main interface.
    MainInterfaceThread* m_mainInterfaceThread;

    ///If m_interfaceIsPaused is true, all data from this interface is dicarded.
    bool m_interfaceIsPaused;
};

#endif // SCRIPTSERIALPORT_H

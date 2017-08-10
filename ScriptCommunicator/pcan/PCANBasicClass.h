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
//
#ifndef __PCANBASICCLASSH_
#define __PCANBASICCLASSH_

#include <QtCore/QtGlobal>
#include <QObject>
#include <QTimer>
#include <QVector>

#include "PCANBasic.h"


#if defined(WIN32) || defined(_WIN32)
// Function pointers
//

typedef TPCANStatus (__stdcall *fpInitialize)(TPCANHandle, TPCANBaudrate, TPCANType, DWORD, WORD);
typedef TPCANStatus (__stdcall *fpOneParam)(TPCANHandle);
typedef TPCANStatus (__stdcall *fpRead)(TPCANHandle, TPCANMsg*, TPCANTimestamp*);
typedef TPCANStatus (__stdcall *fpWrite)(TPCANHandle, TPCANMsg*);
typedef TPCANStatus (__stdcall *fpFilterMessages)(TPCANHandle, DWORD, DWORD, TPCANMode);
typedef TPCANStatus (__stdcall *fpGetSetValue)(TPCANHandle, TPCANParameter, void*, DWORD);
typedef TPCANStatus (__stdcall *fpGetErrorText)(TPCANStatus, WORD, LPSTR);

// Re-define of name for better code-read
//
#define fpUninitialize fpOneParam
#define fpReset fpOneParam
#define fpGetStatus fpOneParam
#define fpGetValue fpGetSetValue
#define fpSetValue fpGetSetValue

///Class which represents a pcan interface.
class PCANBasicClass : public QObject
{
    Q_OBJECT

    public:

        PCANBasicClass(QObject *parent);
        ~PCANBasicClass();

        ///Opens a pcan interface.
        bool open(quint8 channel, quint32 baudRate, bool busOffAutoReset, bool powerSupply);

        ///Configures the reception filter.
        bool setFilter(bool filterExtended, quint32 filterFrom, quint32 filterTo);

        ///Closes the pcan interface.
        void close();

        ///Sends a can message.
        bool sendData(const QByteArray &data);

        ///Sends a can message (max. 8 bytes).
        bool sendCanMessage(quint8 type, quint32 canId, QVector<unsigned char> data);

        ///Returns true if connected to a pcan interface.
        bool isConnected(void);

        ///Converts a baudrate to the corresponding string.
        static quint16 convertBaudrateString(QString baudrate);

        ///Reads the last received messages.
        QByteArray readLastMessage(void);

        ///Returns the current status as string.
        QString getStatusString(void);

        ///Returns the current status.
        TPCANStatus getCurrentStatus(void){return m_currentStatus;}


        ///Returns a pcan parameter.
        TPCANStatus getValue(TPCANHandle channel, TPCANParameter parameter, void* buffer, DWORD bufferLength);

        ///Returns a pcan parameter.
        TPCANStatus getValue(TPCANParameter parameter, void* buffer, DWORD bufferLength);

        ///Sets a pcan parameter.
        TPCANStatus setValue(TPCANHandle channel, TPCANParameter parameter, void* buffer, DWORD bufferLength);

        ///Sets a pcan parameter.
        TPCANStatus setValue(TPCANParameter parameter, void* buffer, DWORD bufferLength);

        ///The max. number of allowed send errors for a single CAN message (after this number the sending of data fails).
        const quint32 MAX_SEND_ERROR = 10;

        ///The max. number of bytes in a single CAN message.
        static const qint32 MAX_BYTES_PER_MESSAGE = 8;

        ///The number of bytes for the CAN type.
        static const qint32 BYTES_FOR_CAN_TYPE = 1;

        ///The number of bytes for the CAN id.
        static const qint32 BYTES_FOR_CAN_ID = 4;

        ///The number of bytes for the CAN timestamp.
        static const qint32 BYTES_FOR_CAN_TIMESTAMP = 4;

        ///The number of bytes for the meta data in a send message.
        static const quint32 BYTES_METADATA_SEND = BYTES_FOR_CAN_TYPE + BYTES_FOR_CAN_ID;

        ///The number of bytes for the meta data in a received message.
        static const quint32 BYTES_METADATA_RECEIVE = BYTES_FOR_CAN_TYPE + BYTES_FOR_CAN_ID + BYTES_FOR_CAN_TIMESTAMP;

signals:
        ///Is emitted if data has been received.
        void readyRead(void);
private slots:
        ///Checks if data has been received.
        void receiveTimerSlot();

private:



        /// <summary>
        /// Initializes a PCAN Channel
        /// </summary>
        /// <param name="Channel">"The handle of a PCAN Channel"</param>
        /// <param name="Btr0Btr1">"The speed for the communication (BTR0BTR1 code)"</param>
        /// <param name="HwType">"NON PLUG&PLAY: The type of hardware and operation mode"</param>
        /// <param name="IOPort">"NON PLUG&PLAY: The I/O address for the parallel port"</param>
        /// <param name="Interrupt">"NON PLUG&PLAY: Interrupt number of the parallel port"</param>
        /// <returns>"A TPCANStatus error code"</returns>
        TPCANStatus initialize(TPCANHandle Channel, TPCANBaudrate Btr0Btr1, TPCANType HwType = 0, DWORD IOPort = 0, WORD Interrupt = 0);

        /// <summary>
        /// Uninitializes one or all PCAN Channels initialized by CAN_Initialize
        /// </summary>
        /// <remarks>Giving the TPCANHandle value "PCAN_NONEBUS",
        /// uninitialize all initialized channels</remarks>
        /// <param name="Channel">"The handle of a PCAN Channel"</param>
        /// <returns>"A TPCANStatus error code"</returns>
        TPCANStatus uninitialize(TPCANHandle Channel);

        /// <summary>
        /// Resets the receive and transmit queues of the PCAN Channel
        /// </summary>
        /// <remarks>
        /// A reset of the CAN controller is not performed.
        /// </remarks>
        /// <param name="Channel">"The handle of a PCAN Channel"</param>
        /// <returns>"A TPCANStatus error code"</returns>
        TPCANStatus reset(TPCANHandle Channel);


        /// <summary>
        /// Reads a CAN message from the receive queue of a PCAN Channel
        /// </summary>
        /// <param name="Channel">"The handle of a PCAN Channel"</param>
        /// <param name="MessageBuffer">"A TPCANMsg structure buffer to store the CAN message"</param>
        /// <param name="TimestampBuffer">"A TPCANTimestamp structure buffer to get
        /// the reception time of the message. If this value is not desired, this parameter
        /// should be passed as NULL"</param>
        /// <returns>"A TPCANStatus error code"</returns>
        TPCANStatus read(TPCANHandle Channel, TPCANMsg* MessageBuffer, TPCANTimestamp* TimestampBuffer);

        /// <summary>
        /// Transmits a CAN message
        /// </summary>
        /// <param name="Channel">"The handle of a PCAN Channel"</param>
        /// <param name="MessageBuffer">"A TPCANMsg buffer with the message to be sent"</param>
        /// <returns>"A TPCANStatus error code"</returns>
        TPCANStatus write(TPCANHandle Channel, TPCANMsg* MessageBuffer);

        /// <summary>
        /// Configures the reception filter.
        /// </summary>
        /// <remarks>The message filter will be expanded with every call to
        /// this function. If it is desired to reset the filter, please use
        /// the CAN_SetParameter function</remarks>
        /// <param name="Channel">"The handle of a PCAN Channel"</param>
        /// <param name="FromID">"The lowest CAN ID to be received"</param>
        /// <param name="ToID">"The highest CAN ID to be received"</param>
        /// <param name="Mode">"Message type, Standard (11-bit identifier) or
        /// Extended (29-bit identifier)"</param>
        /// <returns>"A TPCANStatus error code"</returns>
        TPCANStatus filterMessages(TPCANHandle Channel, DWORD FromID, DWORD ToID, TPCANMode Mode);


    //DLL PCANBasic
    HINSTANCE m_hDll;

    //Function pointers
    fpInitialize m_pInitialize;
    fpUninitialize m_pUnInitialize;
    fpReset m_pReset;
    fpGetStatus m_pGetStatus;
    fpRead m_pRead;
    fpWrite m_pWrite;
    fpFilterMessages m_pFilterMessages;
    fpGetValue m_pGetValue;
    fpSetValue m_pSetValue;
    fpGetErrorText m_pGetTextError;

    ///True if the pcan dll has already been loaded.
    bool m_pcanAlreadyLoaded;

    ///Loads the PCANBasic API.
    void loadAPI();

    ///Releases the loaded API.
    void unloadAPI();

    ///Initializes the pointers for the PCANBasic functions.
    void initializePointers();

    ///Loads the pcan DLL.
    bool loadDllHandle();

    ///Gets the address of a given function name in a loaded DLL.
    FARPROC getFunction(const char* szName);

    TPCANHandle m_currentHandle;

    ///True if data is ready for read.
    bool m_dataReadyForRead;

    ///Checks if data has been received.
    QTimer m_receiveTimer;

    TPCANMsg m_lastReadMessage;

    ///The time stamp of the first received message.
    quint64 m_timeStampFirstReceivedMessage;

    ///The time stamp of the last received message.
    TPCANTimestamp m_timeStampLastReceivedMessage;

    ///True if a first message has been received.
    bool m_firstMessageReceived;

    ///The current CAN status.
    TPCANStatus m_currentStatus;
};
#else//Linux is not supported.
class PCANBasicClass : public QObject
{
    Q_OBJECT
    public:

        PCANBasicClass(QObject *parent) : QObject(parent){}
        ~PCANBasicClass(){}
        bool open(quint8 channel, quint32 baudRate, bool busOffAutoReset, bool powerSupply){(void)channel;(void)baudRate;(void)busOffAutoReset;(void)powerSupply;return false;}
        void close(){}
        bool sendData(const QByteArray &data){(void)data;return false;}
        QByteArray readLastMessage(void){QByteArray data;return data;}
        TPCANStatus getValue(TPCANHandle Channel, TPCANParameter Parameter, void* Buffer, DWORD BufferLength){(void)Channel;(void)Parameter;(void)Buffer;(void)BufferLength;return PCAN_ERROR_INITIALIZE;}

        ///The max. number of allowed send errors for a single CAN message (after this number the sending of data fails).
        const quint32 MAX_SEND_ERROR = 1000;

        ///The max. number of bytes in a single CAN message.
        static const qint32 MAX_BYTES_PER_MESSAGE = 8;

        ///The number of bytes for the CAN type.
        static const qint32 BYTES_FOR_CAN_TYPE = 1;

        ///The number of bytes for the CAN id.
        static const qint32 BYTES_FOR_CAN_ID = 4;

        ///The number of bytes for the CAN timestamp.
        static const qint32 BYTES_FOR_CAN_TIMESTAMP = 4;

        ///The number of bytes for the meta data in a send message.
        static const quint32 BYTES_METADATA_SEND = BYTES_FOR_CAN_TYPE + BYTES_FOR_CAN_ID;

        ///The number of bytes for the meta data in a received message.
        static const quint32 BYTES_METADATA_RECEIVE = BYTES_FOR_CAN_TYPE + BYTES_FOR_CAN_ID + BYTES_FOR_CAN_TIMESTAMP;


        QString getStatusString(void){return "";}
        TPCANStatus getCurrentStatus(void){return PCAN_ERROR_OK;}
        bool isConnected(void){return false;}
        static quint16 convertBaudrateString(QString baudrate){(void)baudrate;return 0;}
        bool sendCanMessage(quint8 type, quint32 canId, QVector<unsigned char> data)
        {
            (void)type;
            (void)canId;
            (void)data;
            return true;
        }
        bool setFilter(bool filterExtended, quint32 filterFrom, quint32 filterTo){(void) filterExtended;(void)filterFrom;(void)filterTo;return true;}

        TPCANStatus getValue(TPCANParameter parameter, void* buffer, DWORD bufferLength)
        {
            (void)parameter;
            (void)buffer;
            (void)bufferLength;
            return PCAN_ERROR_OK;
        }

        TPCANStatus setValue(TPCANHandle channel, TPCANParameter parameter, void* buffer, DWORD bufferLength)
        {
            (void)channel;
            (void)parameter;
            (void)buffer;
            (void)bufferLength;
            return PCAN_ERROR_OK;
        }

        TPCANStatus setValue(TPCANParameter parameter, void* buffer, DWORD bufferLength)
        {

            (void)parameter;
            (void)buffer;
            (void)bufferLength;
            return PCAN_ERROR_OK;

        }
signals:
        void readyRead(void);
};
#endif



#endif

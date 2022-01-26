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

#if defined(WIN32) || defined(_WIN32)

#include "PCANBasicClass.h"
#include <QThread>




/**
 * Constructor.
 * @param parent
 *      The parent pointer.
 */
PCANBasicClass::PCANBasicClass(QObject *parent) : QObject(parent), m_dataReadyForRead(false)
{

    m_pcanAlreadyLoaded = false;
	m_hDll = NULL;
    m_currentHandle = PCAN_NONEBUS;

    m_lastReadMessage.ID = 0xffffffff;

    m_timeStampFirstReceivedMessage = 0;
    m_firstMessageReceived = true;

    m_currentStatus = PCAN_ERROR_OK;

     connect(&m_receiveTimer, SIGNAL(timeout()),this, SLOT(receiveTimerSlot()));

    //Load the API.
    loadAPI();
}

/**
 * Destructor.
 */
PCANBasicClass::~PCANBasicClass()
{
    close();

    //Unload the API.
    unloadAPI();
}

/**
 * Configures the reception filter.
 * @param filterExtended
 *      True if the filer message type is extended (29-bit identifier) or false if the filter message type
 *      is standard (11-bit identifier).
 * @param filterFrom
 *      The lowest CAN ID to be received
 * @param filterTo
 *      The highest CAN ID to be received.
 * @return
 *      True on success.
 */
bool PCANBasicClass::setFilter(bool filterExtended, quint32 filterFrom, quint32 filterTo)
{
    bool result = false;

    if(m_currentHandle != PCAN_NONEBUS)
    {
       TPCANStatus status = filterMessages(m_currentHandle, filterFrom, filterTo, filterExtended ? 2 : 0);
        if(status == PCAN_ERROR_OK)
        {
           result = true;
        }
    }

    return result;
}

/**
 * Opens a pcan interface.
 * @param channel
 *      The pcan channel.
 * @param baudRate
 *      The baudrate.
 * @param busOffAutoReset
 *      True if the PCAN driver shall reset automatically the CAN controller of a PCAN Channel when a bus-off state is detected.
 * @param powerSupply
 *      True if the external 5V on the D-Sub connector shall be switched on.
 * @return
 *      True on success.
 */
bool PCANBasicClass::open(quint8 channel, quint32 baudRate, bool busOffAutoReset, bool powerSupply)
{
    bool result = false;

    m_currentHandle = (PCAN_USBBUS1 + channel) - 1;
    m_firstMessageReceived = false;

    TPCANStatus status = initialize(m_currentHandle, (TPCANBaudrate)baudRate);
    if(status == PCAN_ERROR_OK)
    {
       m_dataReadyForRead = false;

       quint32 buffer = busOffAutoReset ? 1 : 0;
       (void)setValue(m_currentHandle, PCAN_BUSOFF_AUTORESET, (void*)&buffer, sizeof(buffer));

       buffer = powerSupply ? 1 : 0;
       (void)setValue(m_currentHandle, PCAN_5VOLTS_POWER, (void*)&buffer, sizeof(buffer));

       QThread::msleep(100);
       reset(m_currentHandle);
       QThread::msleep(100);

       //Discard all available messages.
       TPCANMsg message;
       TPCANTimestamp time;
       do
       {
           m_currentStatus = read(m_currentHandle, &message, &time);

       }while(!(m_currentStatus & PCAN_ERROR_QRCVEMPTY));

       result = true;
       m_receiveTimer.start(1);

    }
    else
    {
         m_currentHandle = PCAN_NONEBUS;
    }

    return result;
}

/**
 * Closes the pcan interface.
 */
void PCANBasicClass::close()
{
    if(m_currentHandle != PCAN_NONEBUS)
    {
        m_receiveTimer.stop();
        uninitialize(m_currentHandle);
        m_currentHandle = PCAN_NONEBUS;
        m_currentStatus = PCAN_ERROR_OK;
    }
}

/**
 * Returns true if connected to a pcan interface.
 */
bool PCANBasicClass::isConnected(void)
{
    return (m_currentHandle != PCAN_NONEBUS) ? true : false;
}

/**
 * Converts a baudrate to the corresponding string.
 * @param baudrate
 *      The baudrate.
 * @return
 *      The created string.
 */
quint16 PCANBasicClass::convertBaudrateString(QString baudrate)
{
    TPCANBaudrate result;

    if(baudrate == "1000")
    {
        result = PCAN_BAUD_1M;
    }
    else if(baudrate == "800")
    {
        result = PCAN_BAUD_800K;
    }
    else if(baudrate == "500")
    {
        result = PCAN_BAUD_500K;
    }
    else if(baudrate == "250")
    {
        result = PCAN_BAUD_250K;
    }
    else if(baudrate == "125")
    {
        result = PCAN_BAUD_125K;
    }
    else if(baudrate == "100")
    {
        result = PCAN_BAUD_100K;
    }
    else if(baudrate == "95")
    {
        result = PCAN_BAUD_95K;
    }
    else if(baudrate == "83")
    {
        result = PCAN_BAUD_83K;
    }
    else if(baudrate == "50")
    {
        result = PCAN_BAUD_50K;
    }
    else if(baudrate == "47")
    {
        result = PCAN_BAUD_47K;
    }
    else if(baudrate == "33")
    {
        result = PCAN_BAUD_33K;
    }
    else if(baudrate == "20")
    {
        result = PCAN_BAUD_20K;
    }
    else if(baudrate == "10")
    {
        result = PCAN_BAUD_10K;
    }
    else if(baudrate == "5")
    {
        result = PCAN_BAUD_5K;
    }
    else
    {
        result = 0;
    }

    return result;

}

/**
 * Sends a can message (max. 8 bytes).
 * @param type
 *  The can message type: 0=standard, 1=standard remote-transfer-request, 2=extended,
 *  3= extended remote-transfer-request
 * @param canId
 *      The can id.
 * @param data
 *      The can data (max. 8 Bytes).
 * @return
 *      True on success.
 */
bool PCANBasicClass::sendCanMessage(quint8 type, quint32 canId, QVector<unsigned char> data)
{
    bool result = false;
    TPCANMsg messageBuffer;


    if(data.length() <= MAX_BYTES_PER_MESSAGE)
    {
        TPCANStatus status = PCAN_ERROR_UNKNOWN;
        quint32 errorCounter = 0;

        messageBuffer.MSGTYPE = type;
        messageBuffer.ID = canId;

        messageBuffer.LEN = data.length();
        memcpy(messageBuffer.DATA, data.constData(), messageBuffer.LEN);

        do
        {
           status = write(m_currentHandle, &messageBuffer);

            if(status == PCAN_ERROR_QXMTFULL)
            {
                errorCounter++;
                QThread::msleep(1);
            }
            else
            {
                break;

            }
        }while((status != PCAN_ERROR_OK) && (errorCounter < MAX_SEND_ERROR));

        if(status == PCAN_ERROR_OK)
        {
            result = true;
        }
    }

    return result;
}

/**
 * Sends a can message.
 * @param data
 *      The can message.
 *      Byte 0=  message type (0=standard, 1=standard remote-transfer-request,
 *      2=extended, 3= extended remote-transfer-request)
 *      Byte 1-4= can id
 *      Byte 5-12= the data.
 * @return
 *      True on success.
 */
bool PCANBasicClass::sendData(const QByteArray &data)
{
    bool result = false;
    TPCANMsg messageBuffer;

     if(m_currentHandle != PCAN_NONEBUS)
     {

         TPCANStatus status = PCAN_ERROR_UNKNOWN;
         messageBuffer.MSGTYPE = data[0];

         QByteArray idArray = data.mid(1, 4);
         int length = idArray.length();
         for(int i = length; i < 4; i++)
         {
             idArray.push_front((char)0);
         }
         messageBuffer.ID  = ((quint8)idArray[0] << 24) + ((quint8)idArray[1] << 16) + ((quint8)idArray[2] << 8) + ((quint8)idArray[3] & 0xff);

         if(messageBuffer.MSGTYPE <= PCAN_MESSAGE_RTR){messageBuffer.ID = messageBuffer.ID & 0x7ff;}
         else{messageBuffer.ID = messageBuffer.ID & 0x1fffffff;}



         if(data.length() > (BYTES_FOR_CAN_TYPE + BYTES_FOR_CAN_ID) )
         {
             for(int i = 0; i < data.length() - (BYTES_FOR_CAN_TYPE + BYTES_FOR_CAN_ID) ; i+= MAX_BYTES_PER_MESSAGE)
             {
                 QByteArray tmpArray = data.mid(i + (BYTES_FOR_CAN_TYPE + BYTES_FOR_CAN_ID) , MAX_BYTES_PER_MESSAGE);
                 quint32 errorCounter = 0;

                 messageBuffer.LEN = tmpArray.length();
                 memcpy(messageBuffer.DATA, tmpArray.constData(), messageBuffer.LEN);

                 do
                 {
                    status = write(m_currentHandle, &messageBuffer);

                     if(status == PCAN_ERROR_QXMTFULL)
                     {
                         errorCounter++;
                         QThread::msleep(1);
                     }
                     else
                     {
                         break;

                     }
                 }while((status != PCAN_ERROR_OK) && (errorCounter < MAX_SEND_ERROR));

                 if(status != PCAN_ERROR_OK)
                 {
                     break;
                 }
             }
         }
         else
         {
             if(data.length() == (BYTES_FOR_CAN_TYPE + BYTES_FOR_CAN_ID))
             {//Empty message.

                messageBuffer.LEN = 0;
                status = write(m_currentHandle, &messageBuffer);
             }
             else
             {//Insufficient number of bytes.

                status = PCAN_ERROR_ILLDATA;
             }
         }

         if(status == PCAN_ERROR_OK)
         {
             result = true;
         }

     }

    return result;
}

/**
 * Returns the current status as string.
 * @return
 *      The status string.
 */
QString PCANBasicClass::getStatusString(void)
{
    QString statusString = "Bus error:";

    if(m_currentStatus & (PCAN_ERROR_BUSLIGHT | PCAN_ERROR_BUSHEAVY | PCAN_ERROR_BUSOFF))
    {

        if(m_currentStatus & PCAN_ERROR_BUSLIGHT)
        {
            statusString += " 'light' limit reached";

            if(m_currentStatus & (PCAN_ERROR_BUSHEAVY | PCAN_ERROR_BUSOFF))
            {
                statusString += ",";
            }
        }
        if(m_currentStatus & PCAN_ERROR_BUSHEAVY)
        {

            statusString += " 'heavy' limit reached";

            if(m_currentStatus & PCAN_ERROR_BUSOFF)
            {
                statusString += ",";
            }
        }
        if(m_currentStatus & PCAN_ERROR_BUSOFF)
        {
            statusString += " bus-off state";
        }
    }
    else
    {
        statusString += " no error";
    }

    return statusString;
}

/**
 * Reads the last received messages.
 * @return
 *      The received message.
 */
QByteArray PCANBasicClass::readLastMessage(void)
{

    m_dataReadyForRead = false;
    QByteArray data;

     if(m_currentHandle != PCAN_NONEBUS)
     {

        if(m_lastReadMessage.ID == 0xffffffff)
        {

            m_currentStatus = read(m_currentHandle, &m_lastReadMessage, &m_timeStampLastReceivedMessage);

            if((m_currentStatus & PCAN_ERROR_QRCVEMPTY) || (m_currentStatus & PCAN_ERROR_BUSOFF) || (m_lastReadMessage.MSGTYPE == PCAN_MESSAGE_STATUS))
            {//The message is not a CAN message.
                m_lastReadMessage.ID = 0xffffffff;

            }
        }

        if(m_lastReadMessage.ID != 0xffffffff)
        {
            if(!m_firstMessageReceived)
            {
                m_timeStampFirstReceivedMessage = (quint64)m_timeStampLastReceivedMessage.micros + (quint64)(1000 * (quint64)m_timeStampLastReceivedMessage.millis) +
                                                   (quint64)(0xFFFFFFFF * 1000 * (quint64)m_timeStampLastReceivedMessage.millis_overflow);
                m_firstMessageReceived = true;
            }


            quint64 timeStampLast = (quint64)m_timeStampLastReceivedMessage.micros + (quint64)(1000 * (quint64)m_timeStampLastReceivedMessage.millis) +
                                       (quint64)(0xFFFFFFFF * 1000 * (quint64)m_timeStampLastReceivedMessage.millis_overflow);



            quint32 timeStampDiff = (quint32)((timeStampLast - m_timeStampFirstReceivedMessage) / 1000);

            data.push_back(m_lastReadMessage.MSGTYPE);
            //CAN id
            data.push_back((m_lastReadMessage.ID >> 24) & 0xff);
            data.push_back((m_lastReadMessage.ID >> 16) & 0xff);
            data.push_back((m_lastReadMessage.ID >> 8) & 0xff);
            data.push_back(m_lastReadMessage.ID & 0xff);
            //timestamp
            data.push_back((timeStampDiff >> 24) & 0xff);
            data.push_back((timeStampDiff >> 16) & 0xff);
            data.push_back((timeStampDiff >> 8) & 0xff);
            data.push_back(timeStampDiff & 0xff);

            if(m_lastReadMessage.MSGTYPE & PCAN_MESSAGE_RTR)
            {
                m_lastReadMessage.LEN = 0;
            }

            for(int i = 0; i < m_lastReadMessage.LEN; i ++)
            {
                data.push_back(m_lastReadMessage.DATA[i]);
            }

            m_lastReadMessage.ID = 0xffffffff;
        }

     }

    return data;
}

/**
 * Checks if data has been received.
 */
void PCANBasicClass::receiveTimerSlot()
{
    if(m_currentHandle != PCAN_NONEBUS)
    {
        if(!m_dataReadyForRead)
        {
            m_receiveTimer.stop();

            m_currentStatus = read(m_currentHandle, &m_lastReadMessage, &m_timeStampLastReceivedMessage);

            if(!((m_currentStatus & PCAN_ERROR_QRCVEMPTY) || (m_currentStatus & PCAN_ERROR_BUSOFF)))
            {//message received

                if(m_lastReadMessage.MSGTYPE != PCAN_MESSAGE_STATUS)
                {
                    m_dataReadyForRead = true;
                    emit readyRead();
                }
                else
                {
                    m_lastReadMessage.ID = 0xffffffff;
                }

            }

            m_receiveTimer.start(1);
        }
    }
}

/**
 * Loads the PCANBasic API.
 */
void PCANBasicClass::loadAPI()
{
    //Initialize the pointers.
    initializePointers();

    //Load the DLL.
    if(loadDllHandle())
    {

        // Load the API functions.
        m_pInitialize = (fpInitialize)getFunction("CAN_Initialize");
        m_pUnInitialize = (fpUninitialize)getFunction("CAN_Uninitialize");
        m_pReset = (fpReset)getFunction("CAN_Reset");
        m_pGetStatus = (fpGetStatus)getFunction("CAN_GetStatus");
        m_pRead = (fpRead)getFunction("CAN_Read");
        m_pWrite = (fpWrite)getFunction("CAN_Write");
        m_pFilterMessages = (fpFilterMessages)getFunction("CAN_FilterMessages");
        m_pGetValue = (fpSetValue)getFunction("CAN_GetValue");
        m_pSetValue = (fpGetValue)getFunction("CAN_SetValue");
        m_pGetTextError = (fpGetErrorText)getFunction("CAN_GetErrorText");

        m_pcanAlreadyLoaded = m_pInitialize && m_pUnInitialize && m_pReset && m_pGetStatus && m_pRead
                && m_pWrite && m_pFilterMessages && m_pGetValue && m_pSetValue && m_pGetTextError;
    }

}

/**
 * Releases the loaded API.
 */
void PCANBasicClass::unloadAPI()
{
    // Free the loaded DLL.
	if(m_hDll != NULL)
		FreeLibrary(m_hDll);
	m_hDll = NULL;	

    //Initialize the pointers.
    initializePointers();

    m_pcanAlreadyLoaded = false;
}

/**
 * Initializes the pointers for the PCANBasic functions.
 */
void PCANBasicClass::initializePointers()
{
    // Initialize the pointers for the PCANBasic functions.
	m_pInitialize = NULL;
	m_pUnInitialize = NULL;
	m_pReset = NULL;
	m_pGetStatus = NULL;
	m_pRead = NULL;
	m_pWrite = NULL;
	m_pFilterMessages = NULL;
	m_pGetValue = NULL;
	m_pSetValue = NULL;	
	m_pGetTextError = NULL;
}

/**
 * Loads the pcan DLL.
 * @return
 *  True on success.
 */
bool PCANBasicClass::loadDllHandle()
{   
    //Was already loaded.
    if(m_pcanAlreadyLoaded)
		return true;

    //Load Dll.
    m_hDll = LoadLibraryA("PCANBasic");

    //Return true if the DLL was loaded or false otherwise.
	return (m_hDll != NULL);
}

/**
 * Gets the address of a given function name in a loaded DLL.
 * @param strName
 *  The function name.
 * @return
 *  The function pointer.
 */
FARPROC PCANBasicClass::getFunction(const char *strName)
{
    //There is no DLL loaded.
	if(m_hDll  == NULL)
		return NULL;

    //Get the address of the given function in the loaded DLL.
    return GetProcAddress(m_hDll, strName);
}

TPCANStatus PCANBasicClass::initialize(
        TPCANHandle Channel, 
        TPCANBaudrate Btr0Btr1, 
        TPCANType HwType,
		DWORD IOPort, 
		WORD Interrupt)
{
    if(!m_pcanAlreadyLoaded)
		return PCAN_ERROR_UNKNOWN;

	return (TPCANStatus)m_pInitialize(Channel, Btr0Btr1, HwType, IOPort, Interrupt); 
}

TPCANStatus PCANBasicClass::uninitialize(
        TPCANHandle Channel)
{
    if(!m_pcanAlreadyLoaded)
		return PCAN_ERROR_UNKNOWN;

	return (TPCANStatus)m_pUnInitialize(Channel);  
}

TPCANStatus PCANBasicClass::reset(
     TPCANHandle Channel)
{
    if(!m_pcanAlreadyLoaded)
        return PCAN_ERROR_UNKNOWN;

    return (TPCANStatus)m_pReset(Channel);
}



TPCANStatus PCANBasicClass::read(
        TPCANHandle Channel, 
        TPCANMsg* MessageBuffer, 
        TPCANTimestamp* TimestampBuffer)
{
    if(!m_pcanAlreadyLoaded)
		return PCAN_ERROR_UNKNOWN;

	return (TPCANStatus)m_pRead(Channel, MessageBuffer, TimestampBuffer);  
}

TPCANStatus PCANBasicClass::write(
        TPCANHandle Channel, 
        TPCANMsg* MessageBuffer)
{
    if(!m_pcanAlreadyLoaded)
		return PCAN_ERROR_UNKNOWN;

	return (TPCANStatus)m_pWrite(Channel, MessageBuffer);  
}

TPCANStatus PCANBasicClass::filterMessages(
        TPCANHandle Channel, 
        DWORD FromID, 
        DWORD ToID, 
        TPCANMode Mode)
{
    if(!m_pcanAlreadyLoaded)
		return PCAN_ERROR_UNKNOWN;

	return (TPCANStatus)m_pFilterMessages(Channel, FromID, ToID, Mode);  
}

/**
 * Returns a pcan parameter.
 * @param parameter
 *      The pcan parameter.
 * @param buffer
 *      The buffer in which the parameter will be written.
 * @param bufferLength
 *      The buffer length.
 * @return
 *      The result.
 */
TPCANStatus PCANBasicClass::getValue(TPCANParameter parameter, void* buffer, DWORD bufferLength)
{
    return getValue(m_currentHandle, parameter, buffer, bufferLength);
}

/**
 * Returns a pcan parameter.
 * @param channel
 *      The pcan channel.
 * @param parameter
 *      The pcan parameter.
 * @param buffer
 *      The buffer in which the parameter will be written.
 * @param bufferLength
 *      The buffer length.
 * @return
 *      The result.
 */
TPCANStatus PCANBasicClass::getValue(
        TPCANHandle channel,
        TPCANParameter parameter,
        void* buffer,
        DWORD bufferLength)
{
    if(!m_pcanAlreadyLoaded)
		return PCAN_ERROR_UNKNOWN;

    return (TPCANStatus)m_pGetValue(channel, parameter, buffer, bufferLength);
}

/**
 * Sets a pcan parameter.@brief PCANBasicClass::setValue
 * @param parameter
 *      The pcan parameter.
 * @param buffer
 *      The buffer with the pcan parameter.
 * @param bufferLength
 *      The buffer length.
 * @return
 *      The result.
 */
TPCANStatus PCANBasicClass::setValue(TPCANParameter parameter, void* buffer, DWORD bufferLength)
{
    return setValue(m_currentHandle, parameter, buffer, bufferLength);
}

/**
 * Sets a pcan parameter.@brief PCANBasicClass::setValue
 * @param channel
 *      The pcan channel.
 * @param parameter
 *      The pcan parameter.
 * @param buffer
 *      The buffer with the pcan parameter.
 * @param bufferLength
 *      The buffer length.
 * @return
 *      The result.
 */
TPCANStatus PCANBasicClass::setValue(
        TPCANHandle channel,
        TPCANParameter parameter,
        void* buffer,
        DWORD bufferLength)
{
    if(!m_pcanAlreadyLoaded)
		return PCAN_ERROR_UNKNOWN;

    return (TPCANStatus)m_pSetValue(channel, parameter, buffer, bufferLength);
}


#endif

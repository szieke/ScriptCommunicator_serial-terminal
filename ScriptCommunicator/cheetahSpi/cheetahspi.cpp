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

#include "cheetahspi.h"
#include "cheetah.h"

#if defined(WIN32) || defined(_WIN32)
static const QString LIBRARAY_NAME = "cheetah.dll";
#else
static const QString LIBRARAY_NAME = "cheetah.so";
#endif
CheetahSpi::CheetahSpi(QObject *parent): QObject(parent), m_handle(0)
{
}
CheetahSpi::~CheetahSpi()
{

}

/**
 * Returns a string which contains informations about all detected devices..
 * @return
 *      The created string.
 */
QString CheetahSpi::detectDevices(void)
{
    u16  ports[16];
    u32 unique_ids[16];
    int nelem = 16;
    QString text;


    //Find all the attached devices.
    int count = ch_find_devices_ext(nelem, ports, nelem, unique_ids);
    int i;

    if(count != -1)
    {

        text = QString("%1 device(s) found:").arg(count);

        //Append the information on each device.
        if (count > nelem)  count = nelem;
        for (i = 0; i < count; ++i)
        {
            // Determine if the device is in-use
            QString status = "available ";
            if (ports[i] & CH_PORT_NOT_FREE)
            {
                ports[i] &= ~CH_PORT_NOT_FREE;
                status = "in use";
            }


            text += QString("\nport:%1  status:%2  id:%3-%4").arg(QString::number(ports[i]), status,
                                                               QString::number(unique_ids[i]/1000000),
                                                               QString::number(unique_ids[i]%1000000));
        }
    }
    else
    {
        if(count == CH_UNABLE_TO_LOAD_LIBRARY)
        {
            text = "could not load: " + LIBRARAY_NAME;
        }
        else
        {
            text = ch_status_string(count);
        }
    }

    return text;
}
/**
 * Disconnects from the cheetah spi interface.
 */
void CheetahSpi::disconnect(void)
{
    if (m_handle > 0)
    {
        ch_close(m_handle);
        m_handle = 0;
    }
}

/**
 * Connects to a cheetah spi interface.
 * @param port
 *      The port if the interface.
 * @param mode
 *      The spi mode (0-3)
 * @param baudrate
 *      The baudrate of the interface (kHz).
 * @return
 *      True on success.
 */
bool CheetahSpi::connectToDevice(quint32 port, qint16 mode, quint32 baudrate)
{
    bool result = true;

    if (m_handle > 0)
    {
        ch_close(m_handle);
        m_handle = 0;
    }
   m_handle = ch_open(port);

   if (m_handle > 0)
   {
       ch_spi_configure(m_handle, (CheetahSpiPolarity)(mode >> 1), (CheetahSpiPhase)(mode & 1),
                        CH_SPI_BITORDER_MSB, 0x0);

       // Power the flash using the Cheetah adapter's power supply.
       ch_target_power(m_handle, CH_TARGET_POWER_ON);
       ch_sleep_ms(100);


       int currentBaudrate = ch_spi_bitrate(m_handle, (int)baudrate);
       if (currentBaudrate != (int)baudrate)
       {
           ch_close(m_handle);
           m_handle = 0;
       }
       else
       {
           ch_spi_queue_ss(m_handle, 0);
       }
   }


    if (m_handle > 0)
    {
        result = true;
    }
    else
    {
        result = false;
    }

    return result;
}

/**
 * Sends and receive data with the cheetah spi interface.
 * @param sendData
 *      The send data.
 * @param receivedData
 *      The received data.
 * @param chipSelect
 *      The selected chip select (0-2)
 * @return
 *      True on success.
 */
bool CheetahSpi::sendReceiveData(const QByteArray& sendData, QByteArray* receivedData, quint8 chipSelectBits)
{

    u08 *   data_in = new u08[sendData.size()];
    s32 receivedBytes = 0;
    receivedData->clear();
    bool result = true;


    ch_spi_queue_clear(m_handle);
    ch_spi_queue_oe(m_handle, 1);


    ch_spi_queue_ss(m_handle, chipSelectBits);


    for(auto val : sendData)
    {
        ch_spi_queue_byte(m_handle, 1, val);
    }

    receivedBytes = ch_spi_batch_shift(m_handle, sendData.size(), data_in);

    if(receivedBytes > 0)
    {
        for(u32 i = 0; i < (u32)receivedBytes; i++)
        {

            if(i >= (u32)sendData.size())
            {
                result = false;
                break;
            }
            receivedData->append(data_in[i]);
        }
    }
    else
    {//Error.
        result = false;
    }

    // Clear the state of the bus.
    ch_spi_queue_clear(m_handle);
    ch_spi_queue_ss(m_handle, 0);
    ch_spi_queue_oe(m_handle, 0);
    ch_spi_batch_shift(m_handle, 0, 0);

    delete[] data_in;
    return result;
}

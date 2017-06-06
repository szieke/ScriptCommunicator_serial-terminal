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

#include "aardvarkI2cSpi.h"


#if defined(WIN32) || defined(_WIN32)
static const QString LIBRARAY_NAME = "aardvark.dll";
#else
static const QString LIBRARAY_NAME = "aardvark.so";
#endif
AardvarkI2cSpi::AardvarkI2cSpi(QObject *parent): QObject(parent), m_handle(0)
{
}
AardvarkI2cSpi::~AardvarkI2cSpi()
{

}

/**
 * Returns a string which contains informations about all detected devices.
 * @return
 *      The created string.
 */
QString AardvarkI2cSpi::detectDevices(void)
{


    u16  ports[16];
    u32 unique_ids[16];
    int nelem = 16;
    QString text;


    //Find all the attached devices.
    int count = aa_find_devices_ext(nelem, ports, nelem, unique_ids);
    int i;

    if(count >= 0)
    {

        text = QString("%1 device(s) found:").arg(count);

        //Append the information on each device.
        if (count > nelem)  count = nelem;
        for (i = 0; i < count; ++i)
        {
            // Determine if the device is in-use
            QString status = "available ";
            if (ports[i] & AA_PORT_NOT_FREE)
            {
                ports[i] &= ~AA_PORT_NOT_FREE;
                status = "in use";
            }


            text += QString("\nport:%1  status:%2  id:%3-%4").arg(QString::number(ports[i]), status,
                                                               QString::number(unique_ids[i]/1000000),
                                                               QString::number(unique_ids[i]%1000000));
        }
    }
    else
    {
        if(count == AA_UNABLE_TO_LOAD_LIBRARY)
        {
            text = "could not load: " + LIBRARAY_NAME;
        }
        else
        {
            text = aa_status_string(count);
        }
    }



    return text;
}

/**
 * Connects to a aarvard I2C SPI interface.
 * @param settings
 *      The interface settings.
 * @return
 *      True on success.
 */
bool AardvarkI2cSpi::connectToDevice(AardvardI2cSpiSettings& settings)
{
    (void)settings;
    return true;
}

/**
 * Disconnects from the aarvard I2C SPI interface.
 */
void AardvarkI2cSpi::disconnect(void)
{
    if (m_handle > 0)
    {
        aa_close(m_handle);
        m_handle = 0;
    }
}

/**
 * Sends and receive data with the aarvard I2C SPI interface.
 * @param sendData
 *      The send data.
 * @param receivedData
 *      The received data.
 * @return
 *      True on success.
 */
bool AardvarkI2cSpi::sendReceiveData(const QByteArray& sendData, QByteArray* receivedData)
{
    *receivedData = sendData;
    return true;
}

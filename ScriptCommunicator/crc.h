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

#ifndef CRC_H
#define CRC_H

#include <QObject>
#include <QVector>

class CRC
{
public:
    CRC();

    ///Calculates a crc8 with a generic polynomial
    static quint8 calculateCrc8(const QVector<unsigned char> data,
                                const unsigned char polynomial, const unsigned char startValue);

    ///Calculates a crc16.
    static quint16 calculateCrc16(const QVector<unsigned char> data,
                              const quint16 polynomial, const unsigned char startValue);

    ///Calculates a crc32.
    static quint32 calculateCrc32(const QVector<unsigned char> data,
                              const quint32 polynomial, const unsigned char startValue);

    /****************Deprecated******************************************************/

    ///Calculates a crc16.
    static quint16 calculateCrc16(const QVector<unsigned char> data);

    ///Calculates a crc32.
    static quint32 calculateCrc32(const QVector<unsigned char> data);

    ///Calculates a crc8.
    static quint8 calculateCrc8(const QVector<unsigned char> data);


};

#endif // CRC_H

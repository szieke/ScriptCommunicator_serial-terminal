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

#ifndef CANTAB_H
#define CANTAB_H


#include <QTableWidget>
#include <QObject>
#include <QTimer>
#include <QTime>

class MainWindow;

///The can tab in the main window.
class CanTab : public QObject
{
    Q_OBJECT

    friend class MainWindow;
public:
    CanTab(MainWindow* mainWindow);

    ///Must be called if a can message has been received.
    void canMessageReceived(const QByteArray& data);

   ///Must be called if a can message has been transmitted.
   void canMessageTransmitted(const QByteArray &data);

   ///Clears the tables.
   void clearTables();

   ///Sets m_isActivated.
   void setActivated(bool activated){m_isActivated = activated;}

private slots:

    ///Cyclic slot function which updates the can receive table.
    void updateTableSlot(void);

    ///Deletes the selected can receive table entries.
    void deleteReceiveTableEntrySlot(void);

    ///Deletes the selected can transmit table entries.
    void deleteTransmitTableEntrySlot(void);
private:

    int getRowToInsert(QTableWidget* table, quint32 canId, quint32 type);

    ///Resizes a table.
    void resizeTable(QTableWidget* table);

    ///Update a table entry.
    void updateTableEntry(QTableWidget* table, const QByteArray &data, bool isReceived);

    ///Deletes the selected table entries.
    void deleteSelectedEntries(QTableWidget* table);

    ///Converts a can type to a type string.
    QString typeToString(quint8 type);

    ///Creates a new entry in the can receive table.
    void createNewReceiveEntry(QTableWidget *table, int row);

    ///User role value for the can id in the table.
    static const int  USER_ROLE_CAN_ID_IN_TABLE = Qt::UserRole + 1;

    ///User role value for the can type in the table.
    static const int  USER_ROLE_CAN_TYPE_IN_TABLE = Qt::UserRole + 2;

    ///User role value for the can timestamp in the table.
    static const int  USER_ROLE_CAN_TIMESTAMP_IN_TABLE = Qt::UserRole + 3;

    ///User role value for the last cycle value in the table.
    static const int  USER_ROLE_CAN_LAST_CYCLE_IN_TABLE = Qt::UserRole + 4;

    ///User role value for the second to last cycle value in the table.
    static const int  USER_ROLE_CAN_SECOND_TO_LAST_CYCLE_IN_TABLE = Qt::UserRole + 5;


    ///Id column in the receive table.
    static const int  RECEIVE_TABLE_ID_COLUMN = 0;

    ///Type column in the receive table.
    static const int  RECEIVE_TABLE_TYPE_COLUMN = 1;

    ///DLC column in the receive table.
    static const int  RECEIVE_TABLE_DLC_COLUMN = 2;

    ///Data column in the receive table.
    static const int  RECEIVE_TABLE_DATA_COLUMN = 3;

    ///Cycle column in the receive table.
    static const int  RECEIVE_TABLE_CYCLE_COLUMN = 4;

    ///Cycle column in the receive table.
    static const int  RECEIVE_TABLE_COUNT_COLUMN = 5;

    ///Finds an entry in the can receive table.
    int findEntry(quint32 canId, QTableWidget* table, quint8 type);

    ///Main window pointer.
    MainWindow* m_mainWindow;

    ///Timer which calls updateTableSlot periodically.
    QTimer m_updateTimer;

    ///The time at which this object has been created.
    QDateTime m_creationTime;

    ///Contains the revieved CAN messages.
    QVector<QByteArray> m_recievedCanMessages;

    ///True if the CAN tab is actived.
    bool m_isActivated;
};

#endif // CANTAB_H

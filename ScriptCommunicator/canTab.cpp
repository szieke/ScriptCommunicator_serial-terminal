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

#include "canTab.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"

/**
 * Constructor.
 * @param mainWindow
 *      Main window pointer.
 */
CanTab::CanTab(MainWindow *mainWindow) : QObject(mainWindow), m_mainWindow(mainWindow), m_recievedCanMessages(), m_isActivated(false)
{
    m_creationTime = QDateTime::currentDateTime();

    connect(&m_updateTimer, SIGNAL(timeout()),this, SLOT(updateTableSlot()));

    connect(m_mainWindow->m_userInterface->pcanDeleteReceiveEntryButton, SIGNAL(clicked()),this, SLOT(deleteReceiveTableEntrySlot()));
    connect(m_mainWindow->m_userInterface->pcanDeleteTransmitEntryButton, SIGNAL(clicked()),this, SLOT(deleteTransmitTableEntrySlot()));


    m_updateTimer.start(200);

    m_mainWindow->m_userInterface->canReceiveTableWidget->resizeColumnsToContents();
    m_mainWindow->m_userInterface->canTransmitTableWidget->resizeColumnsToContents();
}

/**
 * Finds an entry in a can table.
 * @param canId
 *      The can id which must be matched.
 * @param table
 *      The can table.
 * @param type
 *      The can type which must be matched.
 * @return
 *      The row of the entry (-1 if not found).
 */
int CanTab::findEntry(quint32 canId, QTableWidget* table, quint8 type)
{
    int row = -1;

    for (int i = 0; i < table->rowCount(); i++)
    {
        quint8 storedType = (quint8)table->item(i, RECEIVE_TABLE_ID_COLUMN)->data(USER_ROLE_CAN_TYPE_IN_TABLE).toUInt();
        quint32 storedCanId = table->item(i, RECEIVE_TABLE_ID_COLUMN)->data(USER_ROLE_CAN_ID_IN_TABLE).toUInt();
        if((storedCanId == canId) && (storedType == type))
        {
            row = i;
            break;
        }
    }

    return row;
}

/**
 * Clears the tables.
 */
void CanTab::clearTables()
{
    m_mainWindow->m_userInterface->canReceiveTableWidget->setRowCount(0);
    m_mainWindow->m_userInterface->canTransmitTableWidget->setRowCount(0);
}

/**
 * Creates a new entry in the can receive table.
 * @param table
 *      The table.
 */
void CanTab::createNewReceiveEntry(QTableWidget* table, int row)
{
    //table->setRowCount(table->rowCount() + 1);
    table->insertRow(row);

    QTableWidgetItem* item = new QTableWidgetItem();
    item->setFlags(item->flags() ^ Qt::ItemIsEditable);
    table->setItem(row, RECEIVE_TABLE_ID_COLUMN, item);

    item = new QTableWidgetItem();
    item->setFlags(item->flags() ^ Qt::ItemIsEditable);
    table->setItem(row, RECEIVE_TABLE_TYPE_COLUMN, item);

    item = new QTableWidgetItem();
    item->setFlags(item->flags() ^ Qt::ItemIsEditable);
    table->setItem(row, RECEIVE_TABLE_DLC_COLUMN, item);

    item = new QTableWidgetItem();
    item->setFlags(item->flags() ^ Qt::ItemIsEditable);
    table->setItem(row, RECEIVE_TABLE_DATA_COLUMN, item);

    item = new QTableWidgetItem();
    item->setFlags(item->flags() ^ Qt::ItemIsEditable);
    table->setItem(row, RECEIVE_TABLE_CYCLE_COLUMN, item);

    item = new QTableWidgetItem();
    item->setFlags(item->flags() ^ Qt::ItemIsEditable);
    table->setItem(row, RECEIVE_TABLE_COUNT_COLUMN, item);


}

/**
 * Converts a can type to a type string.
 * @param type
 *      The can type.
 * @return
 *      The created string.
 */
QString CanTab::typeToString(quint8 type)
{
    QString result;
    switch(type)
    {
        case PCAN_MESSAGE_STANDARD:
        {
            result = "std";
            break;
        }
        case PCAN_MESSAGE_RTR:
        {
            result = "std rtr";
            break;
        }
        case PCAN_MESSAGE_EXTENDED:
        {
            result = "ext";
            break;
        }
        case (PCAN_MESSAGE_RTR + PCAN_MESSAGE_EXTENDED):
        {
            result = "ext rtr";
            break;
        }
        default:
        {
            result = "unkown";
            break;

        }
    }
    return result;
}

/**
 * Deletes the selected can receive table entries.
 */
void CanTab::deleteReceiveTableEntrySlot(void)
{
    deleteSelectedEntries(m_mainWindow->m_userInterface->canReceiveTableWidget);
}

/**
 * Deletes the selected can transmit table entries.
 */
void CanTab::deleteTransmitTableEntrySlot(void)
{
    deleteSelectedEntries(m_mainWindow->m_userInterface->canTransmitTableWidget);
}

/**
 * Deletes the selected table entries.
 * @param table
 *      The table.
 */
void CanTab::deleteSelectedEntries(QTableWidget* table)
{
    QList<int> selectedRows;
    for(int i = 0; i < table->rowCount(); i++)
    {
        if(table->item(i, 0)->isSelected())
        {
            selectedRows << i;
        }
    }

    int alreadySelectedRows = 0;
    for(auto el : selectedRows)
    {
        table->removeRow(el - alreadySelectedRows);
        alreadySelectedRows++;
    }

    table->selectRow(table->rowCount() - 1);
}

int CanTab::getRowToInsert(QTableWidget* table, quint32 canId, quint32 type)
{
    int rowIndex = 0;

    for(int row = 0; row < table->rowCount(); row++)
    {
        quint32 rowId = table->item(row, RECEIVE_TABLE_ID_COLUMN)->data(USER_ROLE_CAN_ID_IN_TABLE).toUInt();
        quint32 rowType = table->item(row, RECEIVE_TABLE_ID_COLUMN)->data(USER_ROLE_CAN_TYPE_IN_TABLE).toUInt();

        if((canId < rowId) || ((canId == rowId) && (type < rowType)))
        {
            rowIndex = row;
            break;
        }
        else
        {
            rowIndex = row + 1;
        }
    }

    return rowIndex;
}

/**
 * Update a table entry.
 * @param table
 *      The table.
 * @param data
 *      The received or transmitted entry.
 */
void CanTab::updateTableEntry(QTableWidget* table, const QByteArray &data, bool isReceived)
{
    bool newItemInserted = false;
    bool firstItemAdded = (table->rowCount() == 0) ? true : false;

    quint8 type = data[0];
    quint32 canId = ((quint8)data[1] << 24) + ((quint8)data[2] << 16) + ((quint8)data[3] << 8) + ((quint8)data[4] & 0xff);

    quint32 timestamp = 0;
    table->blockSignals(true);

    if(isReceived)
    {
        timestamp = ((quint8)data[5] << 24) + ((quint8)data[6] << 16) + ((quint8)data[7] << 8) + ((quint8)data[8] &0xff);
    }
    else
    {
        timestamp = (quint32)m_creationTime.msecsTo(QDateTime::currentDateTime());
    }


    int row = findEntry(canId, table, type);

    if(row == -1)
    {//New item

        row = getRowToInsert(table, canId, type);
        newItemInserted= true;
        createNewReceiveEntry(table, row);


        QString idString = QString::number(canId, 16);
        QString leadingZeros;
        int numberOfLeadingZeros = 8;

        if((type == PCAN_MESSAGE_STANDARD) || (type == PCAN_MESSAGE_RTR))
        {
            numberOfLeadingZeros = 3;
        }
        for(int i = 0; i < (numberOfLeadingZeros - idString.size()); i++)
        {
            leadingZeros += "0";
        }
        idString = leadingZeros + idString+ "h";


        table->item(row, RECEIVE_TABLE_ID_COLUMN)->setData(USER_ROLE_CAN_ID_IN_TABLE, canId);
        table->item(row, RECEIVE_TABLE_ID_COLUMN)->setData(USER_ROLE_CAN_TYPE_IN_TABLE, (quint32)type);
        table->item(row, RECEIVE_TABLE_ID_COLUMN)->setData(USER_ROLE_CAN_TIMESTAMP_IN_TABLE, timestamp);

        table->item(row, RECEIVE_TABLE_ID_COLUMN)->setText(idString);
        table->item(row, RECEIVE_TABLE_TYPE_COLUMN)->setText(typeToString(type));

        //table->resizeColumnsToContents();
    }

    table->item(row, RECEIVE_TABLE_DLC_COLUMN)->setText(QString("%1").arg(data.length() -
                                          (isReceived ? PCANBasicClass::BYTES_METADATA_RECEIVE : PCANBasicClass::BYTES_METADATA_SEND)));
    QString dataString = MainWindow::byteArrayToNumberString(data.mid(isReceived ? PCANBasicClass::BYTES_METADATA_RECEIVE : PCANBasicClass::BYTES_METADATA_SEND),
                                                             false,  true, false, true, true);
    table->item(row, RECEIVE_TABLE_DATA_COLUMN)->setText(dataString + " ");

    quint32 storedTimeStamp = table->item(row, RECEIVE_TABLE_ID_COLUMN)->data(USER_ROLE_CAN_TIMESTAMP_IN_TABLE).toUInt();
    quint32 lastCycle = table->item(row, RECEIVE_TABLE_ID_COLUMN)->data(USER_ROLE_CAN_LAST_CYCLE_IN_TABLE).toUInt();
    quint32 secondToLastCycle = table->item(row, RECEIVE_TABLE_ID_COLUMN)->data(USER_ROLE_CAN_SECOND_TO_LAST_CYCLE_IN_TABLE).toUInt();
    quint32 newCycle = lastCycle;

    if(storedTimeStamp <= timestamp)
    {
        newCycle = timestamp - storedTimeStamp;
        quint32 shownCycle = (newCycle + secondToLastCycle + lastCycle) / 3;

        quint32 tmp = shownCycle % 5;

        if(tmp == 4)
        {//Round up
            shownCycle++;

        }
        else if(tmp == 3)
        {//Round up
            shownCycle += 2;

        }
        else if(tmp == 2)
        {//Round up
            shownCycle += 3;

        }
        else
        {
            shownCycle = shownCycle - tmp;
        }



        table->item(row, RECEIVE_TABLE_CYCLE_COLUMN)->setText(QString("%1").arg(shownCycle));
    }

    table->item(row, RECEIVE_TABLE_COUNT_COLUMN)->setText(QString("%1").arg(table->item(row, RECEIVE_TABLE_COUNT_COLUMN)->text().toUInt() + 1));
    table->item(row, RECEIVE_TABLE_ID_COLUMN)->setData(USER_ROLE_CAN_TIMESTAMP_IN_TABLE, timestamp);
    table->item(row, RECEIVE_TABLE_ID_COLUMN)->setData(USER_ROLE_CAN_SECOND_TO_LAST_CYCLE_IN_TABLE, lastCycle);
    table->item(row, RECEIVE_TABLE_ID_COLUMN)->setData(USER_ROLE_CAN_LAST_CYCLE_IN_TABLE, newCycle);

    if(newItemInserted)
    {
        if(firstItemAdded)
        {
            table->resizeColumnsToContents();
        }
    }

    table->blockSignals(false);
}

/**
 * Must be called if a can message has been received.
 * @param data
 *      The received can message.
 */
void CanTab::canMessageReceived(const QByteArray &data)
{
    if(m_isActivated)
    {
        m_recievedCanMessages.append(data);
    }
}

/**
 * Must be called if a can message has been transmitted.
 * @param data
 *      The received can message.
 */
void CanTab::canMessageTransmitted(const QByteArray &data)
{
    if(m_mainWindow->m_userInterface->pcanUpdateTransmitTableCheckBox->isChecked() && m_isActivated)
    {
        updateTableEntry(m_mainWindow->m_userInterface->canTransmitTableWidget, data, false);
    }
}

/**
 * Resizes a table.
 * @param table
 *      The table.
 */
void CanTab::resizeTable(QTableWidget* table)
{

    table->setColumnWidth(RECEIVE_TABLE_COUNT_COLUMN, table->width() -
                          (table->columnWidth(RECEIVE_TABLE_ID_COLUMN) + table->columnWidth(RECEIVE_TABLE_TYPE_COLUMN)
                           + table->columnWidth(RECEIVE_TABLE_DLC_COLUMN) + table->columnWidth(RECEIVE_TABLE_DATA_COLUMN)
                           + table->columnWidth(RECEIVE_TABLE_CYCLE_COLUMN)
                           + 2 *table->frameWidth()));

}

/**
 * Cyclic slot function which updates the can receive and the transmit table.
 */
void CanTab::updateTableSlot(void)
{

    for(auto el : m_recievedCanMessages)
    {
        updateTableEntry(m_mainWindow->m_userInterface->canReceiveTableWidget, el, true);
    }
    m_recievedCanMessages.clear();

    resizeTable(m_mainWindow->m_userInterface->canReceiveTableWidget);
    resizeTable(m_mainWindow->m_userInterface->canTransmitTableWidget);
}

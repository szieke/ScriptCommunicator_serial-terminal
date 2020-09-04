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

#ifndef SCRIPTLISTWIDGET_H
#define SCRIPTLISTWIDGET_H

#include <QObject>

#include "scriptWidget.h"

///This wrapper class is used to access a QListWidget object (located in a script gui/ui-file) from a script.
class ScriptListWidget : public ScriptWidget
{
    Q_OBJECT
public:
    explicit ScriptListWidget(QListWidget* list, ScriptThread *scriptThread) :
        ScriptWidget(list, scriptThread, scriptThread->getScriptWindow()), m_list(list)
    {
        //connect the necessary signals with the wrapper slots (in this slots the
        //events of the wrapper class are generated, the script can connect to this
        //wrapper events)
        Qt::ConnectionType directConnectionType = scriptThread->runsInDebugger() ? Qt::DirectConnection : Qt::BlockingQueuedConnection;

        connect(this, SIGNAL(setCurrentRowSignal(int,QListWidget*)), scriptThread->getScriptWindow(),
                SLOT(setCurrentRowSlot(int,QListWidget*)), directConnectionType);

        connect(this, SIGNAL(insertNewItemSignal(int,QString,QListWidget*)), scriptThread->getScriptWindow(),
                SLOT(insertNewItem(int,QString,QListWidget*)), directConnectionType);

        connect(this, SIGNAL(setItemIconSignal(QListWidgetItem*,QString)), scriptThread->getScriptWindow(),
                SLOT(setItemIcon(QListWidgetItem*,QString)), directConnectionType);

        connect(this, SIGNAL(clearSignal(QListWidget*)), scriptThread->getScriptWindow(),
                SLOT(clearSlot(QListWidget*)), directConnectionType);

        connect(this, SIGNAL(removeItemSignal(int,QListWidget*)), scriptThread->getScriptWindow(),
                SLOT(removeItem(int,QListWidget*)), directConnectionType);

        connect(this, SIGNAL(sortItemsSignal(bool,QListWidget*)), scriptThread->getScriptWindow(),
                SLOT(sortItemsSlot(bool,QListWidget*)), directConnectionType);

        connect(this, SIGNAL(setItemForegroundColorSignal(QBrush,QListWidgetItem*)), scriptThread->getScriptWindow(),
                SLOT(setItemForegroundColorSlot(QBrush,QListWidgetItem*)), directConnectionType);

        connect(this, SIGNAL(setItemBackgroundColorSignal(QBrush,QListWidgetItem*)), scriptThread->getScriptWindow(),
                SLOT(setItemBackgroundColorSlot(QBrush,QListWidgetItem*)), directConnectionType);

        connect(this, SIGNAL(setItemTextSignal(QString,QListWidgetItem*)), scriptThread->getScriptWindow(),
                SLOT(setItemTextSlot(QString,QListWidgetItem*)), directConnectionType);


        connect(m_list, SIGNAL(currentRowChanged(int)), this, SIGNAL(currentRowChangedSignal(int)), Qt::QueuedConnection);
        connect(m_list, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(stub_itemClicked(QListWidgetItem*)), Qt::QueuedConnection);
        connect(m_list, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(stub_itemDoubleClicked(QListWidgetItem*)), Qt::QueuedConnection);


    }

    ///Returns a semicolon separated list with all public functions, signals and properties.
    virtual QString getPublicScriptElements(void)
    {
        return MainWindow::parseApiFile("ScriptListWidget.api");
    }

    ///Inserts a new list item.
    Q_INVOKABLE void insertNewItem(int row, QString itemText, QString iconFileName)
    {
        if(m_list->count() < (int)row)
        {
            row = m_list->count();
        }

        emit insertNewItemSignal (row, itemText,m_list);

        if(!iconFileName.isEmpty())
        {
            QListWidgetItem* item =  m_list->item(row);
            if(item)
            {
                emit setItemIconSignal(item, iconFileName);
            }
        }
    }

    ///Returns the number of rows in the list widget.
    Q_INVOKABLE int rowCount(void){return m_list->count();}

    ///Removes a item from the list widget.
    Q_INVOKABLE void removeItem(int row)
    {
        m_list->blockSignals(true);
        emit removeItemSignal(row, m_list);
        m_list->blockSignals(false);
    }

    ///Returns the current selected row.
    Q_INVOKABLE int currentSelectedRow(void){return m_list->currentRow();}

    ///Sets the current selected row.
    Q_INVOKABLE void setCurrentRow(int row){emit setCurrentRowSignal(row, m_list);}

    ///Clears the list widget.
    Q_INVOKABLE void clear(void){emit clearSignal(m_list);}

    ///Sets the background color of an item.
    ///Possible colors are: black, white, gray, red, green, blue, cyan, magenta and yellow.
    Q_INVOKABLE void seItemBackgroundColor(int row, QString color)
    {
        QListWidgetItem* item =  m_list->item(row);
        if(item)
        {
            emit setItemBackgroundColorSignal(ScriptWindow::stringToGlobalColor(color), item);
        }
    }
    ///Sets the foreground color of an item.
    ///Possible colors are: black, white, gray, red, green, blue, cyan, magenta and yellow.
    Q_INVOKABLE void setItemForegroundColor(int row, QString color)
    {
        QListWidgetItem* item =  m_list->item(row);
        if(item)
        {
            emit setItemForegroundColorSignal(ScriptWindow::stringToGlobalColor(color), item);
        }
    }

    ///Returns the item text.
    Q_INVOKABLE QString getItemText(int row)
    {
        QListWidgetItem* item =  m_list->item(row);
        QString result;
        if(item)
        {
            result = item->text();
        }
        return result;
    }

    ///Sets the item text.
    Q_INVOKABLE void setItemText(int row, QString text)
    {
        QListWidgetItem* item =  m_list->item(row);
        if(item)
        {
            emit setItemTextSignal(text, item);
        }
    }

    ///Sets the item icon.
    Q_INVOKABLE void setItemIcon(int row, QString iconFileName)
    {
        QListWidgetItem* item =  m_list->item(row);
        if(item)
        {
            emit setItemIconSignal(item, iconFileName);
        }
    }

    ///Sorts the items in the widget in the specified order(true=AscendingOrder,
    ///false=DescendingOrder).
    Q_INVOKABLE void sortItems(bool ascendingOrder=true){emit sortItemsSignal(ascendingOrder, m_list);}

Q_SIGNALS:


    ///This signal is emitted if the current row selection has been changed.
    ///Scripts can connect a function to this signal.
    void currentRowChangedSignal(int currentRow);

    ///This signal is emitted if a row has been clicked.
    ///Scripts can connect a function to this signal.
    void itemClickedSignal(int row);

    ///This signal is emitted if a row has been double clicked.
    ///Scripts can connect a function to this signal.
    void itemDoubleClickedSignal(int row);

    ///This signal is emitted in setCurrentRow.
    ///This signal is private and must not be used inside a script.
    void setCurrentRowSignal(int row, QListWidget* list);

    ///This signal is emitted in insertNewItemSignal.
    ///This signal is private and must not be used inside a script.
    void insertNewItemSignal (int row, QString itemText, QListWidget* list);

    ///This signal is emitted in setItemIcon.
    ///This signal is private and must not be used inside a script.
    void setItemIconSignal(QListWidgetItem* item , QString iconFileName);

    ///This signal is emitted in clear.
    ///This signal is private and must not be used inside a script.
    void clearSignal(QListWidget* list);

    ///This signal is emitted in removeItem.
    ///This signal is private and must not be used inside a script.
    void removeItemSignal(int row, QListWidget* list);

    ///This signal is emitted in sortItems.
    ///This signal is private and must not be used inside a script.
    void sortItemsSignal(bool ascendingOrder, QListWidget* listWidget);

    ///This signal is emitted in setItemBackgroundColor.
    ///This signal is private and must not be used inside a script.
    void setItemBackgroundColorSignal(QBrush brush, QListWidgetItem* item);

    ///This signal is emitted in setForegroundColor.
    ///This signal is private and must not be used inside a script.
    void setItemForegroundColorSignal(QBrush brush, QListWidgetItem* item);

    ///This signal is emitted in setItemText.
    ///This signal is private and must not be used inside a script.
    void setItemTextSignal(QString text, QListWidgetItem* item);

public Q_SLOTS:


    ///This slot function is called if a row has been clicked.
    void stub_itemClicked(QListWidgetItem* item){emit itemClickedSignal(m_list->row(item));}

    ///This slot function is called if a row has been double clicked.
    void stub_itemDoubleClicked(QListWidgetItem* item){ emit itemDoubleClickedSignal(m_list->row(item));}
private:
    ///The wrapped list widget.
    QListWidget* m_list;
};

#endif // SCRIPTLISTWIDGET_H

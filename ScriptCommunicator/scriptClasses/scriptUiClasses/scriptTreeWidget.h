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

#ifndef SCRIPTTREEWIDGET_H
#define SCRIPTTREEWIDGET_H

#include <QTreeWidget>
#include <QDebug>
#include "scriptwindow.h"
#include "scriptThread.h"


#include "scriptTreeWidgetItem.h"

///This wrapper class is used to access a QTreeWidget object (located in a script gui/ui-file) from a script.
class ScriptTreeWidget: public ScriptWidget
{
    Q_OBJECT
public:
    ScriptTreeWidget(QTreeWidget* treeWidget, ScriptThread *scriptThread) :
        ScriptWidget(treeWidget, scriptThread, scriptThread->getScriptWindow()), m_treeWidget(treeWidget), m_scriptThread(scriptThread)
    {

        //connect the necessary signals with the wrapper slots (in this slots the
        //events of the wrapper class are generated, the script can connect to this
        //wrapper events)

        Qt::ConnectionType directConnectionType = scriptThread->runsInDebugger() ? Qt::DirectConnection : Qt::BlockingQueuedConnection;

        connect(this, SIGNAL(setColumnWidthSignal(int,int,QTreeWidget*)), scriptThread->getScriptWindow(),
                SLOT(setColumnWidthSlot(int,int,QTreeWidget*)), directConnectionType);

        connect(this, SIGNAL(resizeColumnToContentsSignal(int,QTreeWidget*)), scriptThread->getScriptWindow(),
                SLOT(resizeColumnToContentsSlot(int,QTreeWidget*)), directConnectionType);

        connect(this, SIGNAL(expandItemSignal(QTreeWidgetItem*, bool)), scriptThread->getScriptWindow(),
                SLOT(expandItemSlot(QTreeWidgetItem*, bool)), directConnectionType);

        connect(this, SIGNAL(expandAllSignal(QTreeWidget*)), scriptThread->getScriptWindow(),
                SLOT(expandAllSlot(QTreeWidget*)), directConnectionType);

        connect(this, SIGNAL(setCurrentItemSignal(QTreeWidgetItem*,QTreeWidget*)), scriptThread->getScriptWindow(),
                SLOT(setCurrentItemSlot(QTreeWidgetItem*,QTreeWidget*)), directConnectionType);

        connect(this, SIGNAL(sortItemsSignal(int,bool,QTreeWidget*)), scriptThread->getScriptWindow(),
                SLOT(sortItemsSlot(int,bool,QTreeWidget*)), directConnectionType);

        connect(this, SIGNAL(setHeaderLabelsSignal(QStringList,QTreeWidget*)), scriptThread->getScriptWindow(),
                SLOT(setHeaderLabelsSignal(QStringList,QTreeWidget*)), directConnectionType);

        connect(this, SIGNAL(setColumnCountSignal(int,QTreeWidget*)), scriptThread->getScriptWindow(),
                SLOT(setColumnCountSlot(int,QTreeWidget*)), directConnectionType);

        connect(this, SIGNAL(addTopLevelItemSignal(QTreeWidgetItem*,QTreeWidget*)), scriptThread->getScriptWindow(),
                SLOT(addTopLevelItemSlot(QTreeWidgetItem*,QTreeWidget*)), directConnectionType);

        connect(this, SIGNAL(insertTopLevelItemSignal(int,QTreeWidgetItem*,QTreeWidget*)), scriptThread->getScriptWindow(),
                SLOT(insertTopLevelItemSlot(int,QTreeWidgetItem*,QTreeWidget*)), directConnectionType);

        connect(this, SIGNAL(takeTopLevelItemSignal(int,QTreeWidgetItem**,QTreeWidget*)), scriptThread->getScriptWindow(),
                SLOT(takeTopLevelItemSlot(int,QTreeWidgetItem**,QTreeWidget*)), directConnectionType);


        connect(m_treeWidget, SIGNAL(itemClicked(QTreeWidgetItem*,int)), this,
                SLOT(stub_itemClicked(QTreeWidgetItem*,int)), Qt::QueuedConnection);

        connect(m_treeWidget, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)), this,
                SLOT(stub_itemDoubleClicked(QTreeWidgetItem*,int)), Qt::QueuedConnection);


        connect(m_treeWidget, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)), this,
                SLOT(stub_currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)), Qt::QueuedConnection);



    }

    virtual ~ScriptTreeWidget()
    {

    }

    ///Returns a semicolon separated list with all public functions, signals and properties.
    virtual QString getPublicScriptElements(void)
    {
        return MainWindow::parseApiFile("ScriptTreeWidget.api");
    }

    ///Creates a script tree widget item.
    Q_INVOKABLE ScriptTreeWidgetItem* createScriptTreeWidgetItem(void)
    {
        QTreeWidgetItem* item = new QTreeWidgetItem();
        ScriptTreeWidgetItem* scriptItem = new ScriptTreeWidgetItem(item, m_scriptThread, this);
        item->setData(0, ScriptTreeWidgetItem::SCRIPT_ITEM_POINTER_DATA_ROLE, (quint64)scriptItem);
        return scriptItem;
    }

    ///Adds a column in the header for each item in the labels list, and sets the label for each column.
    ///Note that setHeaderLabels() won't remove existing columns.
    Q_INVOKABLE void setHeaderLabels(QStringList labels){emit setHeaderLabelsSignal(labels, m_treeWidget);}

    ///Sets the width of a column.
    Q_INVOKABLE void setColumnWidth(int column, int size){emit setColumnWidthSignal(column, size, m_treeWidget);}

    ///Returns the width of a column.
    Q_INVOKABLE int getColumnWidth(int column){return m_treeWidget->columnWidth(column);}

    ///Appends the item as a top-level item in the widget.
    Q_INVOKABLE void addTopLevelItem (ScriptTreeWidgetItem* item){if(item){emit addTopLevelItemSignal(item->getWrappedItem(), m_treeWidget);}}

    ///Inserts the item at index in the top level in the view.
    ///If the item has already been inserted somewhere else it won't be inserted.
    Q_INVOKABLE void insertTopLevelItem (int index, ScriptTreeWidgetItem* item){if(item){emit insertTopLevelItemSignal(index, item->getWrappedItem(), m_treeWidget);}}

    ///Returns the number of top level items.
    Q_INVOKABLE int topLevelItemCount(void){return m_treeWidget->topLevelItemCount();}

    ///Returns the tree widget's invisible root item.
    ///The invisible root item provides access to the tree widget's top-level items through
    ///the ScriptTreeWidgetItem API, making it possible to write functions that can treat top-level
    ///items and their children in a uniform way; for example, recursive functions.
    Q_INVOKABLE ScriptTreeWidgetItem* invisibleRootItem(void)
    {
        QTreeWidgetItem* rootItem = m_treeWidget->invisibleRootItem();
        ScriptTreeWidgetItem* scriptItem = (ScriptTreeWidgetItem*)rootItem->data(0, ScriptTreeWidgetItem::SCRIPT_ITEM_POINTER_DATA_ROLE).toULongLong();
        if(scriptItem == 0)
        {
            scriptItem = new ScriptTreeWidgetItem(rootItem, m_scriptThread, this);
            rootItem->setData(0, ScriptTreeWidgetItem::SCRIPT_ITEM_POINTER_DATA_ROLE, (quint64)scriptItem);
        }
       return scriptItem;
    }

    ///Returns the item above the given item.
    ///If to item is above then it returns null.
    Q_INVOKABLE ScriptTreeWidgetItem* itemAbove(ScriptTreeWidgetItem* item)
    {
        QTreeWidgetItem* elementAbove = m_treeWidget->itemAbove(item->getWrappedItem());
        ScriptTreeWidgetItem* scriptItem = 0;
        if(elementAbove != 0)
        {
            scriptItem  = (ScriptTreeWidgetItem*)elementAbove->data(0, ScriptTreeWidgetItem::SCRIPT_ITEM_POINTER_DATA_ROLE).toULongLong();
        }
       return scriptItem;
    }

    ///Returns the item below the given item.
    ///If to item is below then it returns null.
    Q_INVOKABLE ScriptTreeWidgetItem* itemBelow(ScriptTreeWidgetItem* item)
    {
        QTreeWidgetItem* elementBelow = m_treeWidget->itemBelow(item->getWrappedItem());
        ScriptTreeWidgetItem* scriptItem = 0;
        if(elementBelow != 0)
        {
            scriptItem  = (ScriptTreeWidgetItem*)elementBelow->data(0, ScriptTreeWidgetItem::SCRIPT_ITEM_POINTER_DATA_ROLE).toULongLong();
        }
       return scriptItem;
    }

    ///Removes the top-level item at the given index in the tree and returns it, otherwise returns null.
    Q_INVOKABLE ScriptTreeWidgetItem* takeTopLevelItem(int index)
    {
        QTreeWidgetItem* element = 0;
        ScriptTreeWidgetItem* scriptItem = 0;

        emit takeTopLevelItemSignal(index, &element, m_treeWidget);
        if(element != 0)
        {
            scriptItem  = (ScriptTreeWidgetItem*)element->data(0, ScriptTreeWidgetItem::SCRIPT_ITEM_POINTER_DATA_ROLE).toULongLong();
        }
       return scriptItem;
    }
    ///Returns the top level item at the given index, or null if the item does not exist.
    Q_INVOKABLE ScriptTreeWidgetItem* topLevelItem(int index)
    {
        QTreeWidgetItem* element = m_treeWidget->topLevelItem(index);
        ScriptTreeWidgetItem* scriptItem = 0;
        if(element != 0)
        {
            scriptItem  = (ScriptTreeWidgetItem*)element->data(0, ScriptTreeWidgetItem::SCRIPT_ITEM_POINTER_DATA_ROLE).toULongLong();
        }
       return scriptItem;
    }

    ///Resizes the column given to the size of its contents.
    Q_INVOKABLE void resizeColumnToContents(int column){emit resizeColumnToContentsSignal(column, m_treeWidget);}

    ///Returns the number of columns displayed in the tree widget.
    Q_INVOKABLE int columnCount(void){return m_treeWidget->columnCount();}

    ///Sets the number of columns displayed in the tree widget.
    Q_INVOKABLE void setColumnCount(int columns){emit setColumnCountSignal(columns, m_treeWidget);}

    ///Expands the item. This causes the tree containing the item's children to be expanded.
    Q_INVOKABLE void expandItem(ScriptTreeWidgetItem* item){emit expandItemSignal(item->getWrappedItem(), true);}

    ///Expands all expandable items.
    Q_INVOKABLE void expandAll(void){emit expandAllSignal(m_treeWidget);}

    ///Sets the current item in the tree widget.
    Q_INVOKABLE void setCurrentItem (ScriptTreeWidgetItem* item){emit setCurrentItemSignal(item->getWrappedItem(), m_treeWidget);}

    ///Returns current item in the tree widget.
    Q_INVOKABLE ScriptTreeWidgetItem* currentItem(void)
    {return (ScriptTreeWidgetItem*)(m_treeWidget->currentItem()->data(0, ScriptTreeWidgetItem::SCRIPT_ITEM_POINTER_DATA_ROLE).toULongLong());}

    ///Sorts the items in the widget in the specified order(true=AscendingOrder,
    ///false=DescendingOrder) by the values in the given column.
    Q_INVOKABLE void sortItems(int column, bool ascendingOrder=true){emit sortItemsSignal(column, ascendingOrder,  m_treeWidget);}


signals:

    ///This signal is emitted if the current item changes. The current item is specified by current, and this replaces the previous current item.
    ///Scripts can connect a function to this signal.
    void currentItemChangedSignal(ScriptTreeWidgetItem *current, ScriptTreeWidgetItem *previous);


    ///This signal is emitted if an item has been clicked.
    ///Scripts can connect a function to this signal.
    void itemClickedSignal(ScriptTreeWidgetItem *item, int column);

    ///This signal is emitted if an item has been double clicked.
    ///Scripts can connect a function to this signal.
    void itemDoubleClickedSignal(ScriptTreeWidgetItem *item, int column);


    ///This signal is emitted in setColumnWidth.
    ///This signal is private and must not be used inside a script.
    void setColumnWidthSignal(int column, int size, QTreeWidget* tree);

    ///This signal is emitted in resizeColumnToContents.
    ///This signal is private and must not be used inside a script.
    void resizeColumnToContentsSignal(int column, QTreeWidget* tree);

    ///This signal is emitted in expandItem.
    ///This signal is private and must not be used inside a script.
    void expandItemSignal(QTreeWidgetItem* item, bool expand);

    ///This signal is emitted in expandAll.
    ///Scripts can connect a function to this signal.
    void expandAllSignal(QTreeWidget* tree);

    ///This signal is emitted in setCurrentItem.
    ///This signal is private and must not be used inside a script.
    void setCurrentItemSignal(QTreeWidgetItem* item, QTreeWidget* tree);

    ///This signal is emitted in sortItems.
    ///This signal is private and must not be used inside a script.
    void sortItemsSignal(int column, bool ascendingOrder, QTreeWidget* tree);

    ///This signal is emitted in setHeaderLabels.
    ///This signal is private and must not be used inside a script.
    void setHeaderLabelsSignal(QStringList labels, QTreeWidget* tree);

    ///This signal is emitted in setColumnCount.
    ///This signal is private and must not be used inside a script.
    void setColumnCountSignal(int columns, QTreeWidget* tree);

    ///This signal is emitted in addTopLevelItem.
    ///This signal is private and must not be used inside a script.
    void addTopLevelItemSignal(QTreeWidgetItem* item, QTreeWidget* tree);

    ///This signal is emitted in insertTopLevelItem.
    ///This signal is private and must not be used inside a script.
    void insertTopLevelItemSignal(int index, QTreeWidgetItem* item, QTreeWidget* tree);

    ///This signal is emitted in takeTopLevelItem.
    ///This signal is private and must not be used inside a script.
    void takeTopLevelItemSignal(int index, QTreeWidgetItem** item, QTreeWidget* tree);


private Q_SLOTS:

    ///This slot function is called if an item has been clicked.
    void stub_itemClicked(QTreeWidgetItem* item, int column)
    {
        ScriptTreeWidgetItem* scriptItem = (ScriptTreeWidgetItem*)(item->data(0, ScriptTreeWidgetItem::SCRIPT_ITEM_POINTER_DATA_ROLE).toULongLong());
        emit itemClickedSignal(scriptItem, column);
    }

    ///This slot function is called if an item has been double clicked.
    void stub_itemDoubleClicked(QTreeWidgetItem* item, int column)
    {
        ScriptTreeWidgetItem* scriptItem = (ScriptTreeWidgetItem*)(item->data(0, ScriptTreeWidgetItem::SCRIPT_ITEM_POINTER_DATA_ROLE).toULongLong());
        emit itemDoubleClickedSignal(scriptItem, column);
    }

    ///This slot is called if the current item changes. The current item is specified by current, and this replaces the previous current item.
    void stub_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous)
    {
        ScriptTreeWidgetItem* scriptCurrentItem = 0;
        ScriptTreeWidgetItem* scriptPrevioustItem = 0;

        if(current)
        {
            scriptCurrentItem = (ScriptTreeWidgetItem*)(current->data(0, ScriptTreeWidgetItem::SCRIPT_ITEM_POINTER_DATA_ROLE).toULongLong());
        }

        if(previous)
        {
            scriptPrevioustItem = (ScriptTreeWidgetItem*)(previous->data(0, ScriptTreeWidgetItem::SCRIPT_ITEM_POINTER_DATA_ROLE).toULongLong());
        }

        emit currentItemChangedSignal(scriptCurrentItem, scriptPrevioustItem);
    }


private:
    ///The wrapped tree widget.
    QTreeWidget* m_treeWidget;

    ///The script thread to which this item belongs to.
    ScriptThread *m_scriptThread;


};

#endif // SCRIPTTREEWIDGET_H

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

#ifndef SCRIPTTREEWIDGETITEM_H
#define SCRIPTTREEWIDGETITEM_H

#include <QTableWidget>
#include <QDebug>
#include "scriptWidget.h"
#include "scriptObject.h"

///This wrapper class is used to access a QTreeWidgetItem object (located in a script gui/ui-file) from a script.
class ScriptTreeWidgetItem : public QObject, public ScriptObject
{
    Q_OBJECT
    Q_PROPERTY(QString publicScriptElements READ getPublicScriptElements)

public:
    ScriptTreeWidgetItem(QTreeWidgetItem* treeWidgetItem, ScriptThread *scriptThread, QObject* parent) :
        QObject(parent), m_treeWidgetItem(treeWidgetItem)
    {

        //connect the necessary signals with the wrapper slots (in this slots the
        //events of the wrapper class are generated, the script can connect to this
        //wrapper events)

        Qt::ConnectionType directConnectionType = scriptThread->runsInDebugger() ? Qt::DirectConnection : Qt::BlockingQueuedConnection;

        connect(this, SIGNAL(expandItemSignal(QTreeWidgetItem*, bool)), scriptThread->getScriptWindow(),
                SLOT(expandItemSlot(QTreeWidgetItem*, bool)), directConnectionType);

        connect(this, SIGNAL(setItemIconSignal(QTreeWidgetItem*,int,QString)), scriptThread->getScriptWindow(),
                SLOT(setItemIcon(QTreeWidgetItem*,int,QString)), Qt::QueuedConnection);

        connect(this, SIGNAL(deleteTreeWidgetItemSignal(QTreeWidgetItem*)), scriptThread->getScriptWindow(),
                SLOT(deleteTreeWidgetItemSlot(QTreeWidgetItem*)), Qt::QueuedConnection);

        connect(this, SIGNAL(insertChildSignal(int,QTreeWidgetItem*,QTreeWidgetItem*)), scriptThread->getScriptWindow(),
                SLOT(insertChildSlot(int,QTreeWidgetItem*,QTreeWidgetItem*)), directConnectionType);

        connect(this, SIGNAL(setTextSignal(int,QString,QTreeWidgetItem*)), scriptThread->getScriptWindow(),
                SLOT(setTextSlot(int,QString,QTreeWidgetItem*)), directConnectionType);

        connect(this, SIGNAL(takeChildSignal(int,QTreeWidgetItem**,QTreeWidgetItem*)), scriptThread->getScriptWindow(),
                SLOT(takeChildSlot(int,QTreeWidgetItem**,QTreeWidgetItem*)), directConnectionType);

        connect(this, SIGNAL(sortChildrenSignal(int,bool,QTreeWidgetItem*)), scriptThread->getScriptWindow(),
                SLOT(sortChildrenSlot(int,bool,QTreeWidgetItem*)), directConnectionType);

        connect(this, SIGNAL(setBackgroundColorSignal(int,QString,QTreeWidgetItem*)), scriptThread->getScriptWindow(),
                SLOT(setBackgroundColorSlot(int,QString,QTreeWidgetItem*)), directConnectionType);

        connect(this, SIGNAL(setForegroundColorSignal(int,QString,QTreeWidgetItem*)), scriptThread->getScriptWindow(),
                SLOT(setForegroundColorSlot(int,QString,QTreeWidgetItem*)), directConnectionType);

        connect(this, SIGNAL(setDisabledSignal(bool,QTreeWidgetItem*)), scriptThread->getScriptWindow(),
                SLOT(setDisabledSlot(bool,QTreeWidgetItem*)), directConnectionType);
    }

    virtual ~ScriptTreeWidgetItem()
    {


    }

    ///The data role of the script item to which a QTreeWidgetItem belongs to.
    static const int SCRIPT_ITEM_POINTER_DATA_ROLE = Qt::UserRole + 1;

    ///The data role of user data.
    static const int SET_DATA_USER_ROLE = SCRIPT_ITEM_POINTER_DATA_ROLE + 1;


    ///Returns a semicolon separated list with all public functions, signals and properties.
    virtual QString getPublicScriptElements(void)
    {
        return MainWindow::parseApiFile("ScriptTreeWidgetItem.api");
    }

    ///Sets the text to be displayed in the given column to the given text.
    Q_INVOKABLE void setText(int column, QString text){emit setTextSignal(column, text, m_treeWidgetItem);}

    ///Returns the text in the specified column.
    Q_INVOKABLE QString text(int column){return m_treeWidgetItem->text(column);}

    ///Sets the item icon.
    Q_INVOKABLE void setItemIcon(int column, QString iconFileName){ emit setItemIconSignal(m_treeWidgetItem,column, iconFileName);}

    ///Appends the child item to the list of children.
    Q_INVOKABLE void addChild(ScriptTreeWidgetItem* child){ emit insertChildSignal(m_treeWidgetItem->childCount(), m_treeWidgetItem, child->getWrappedItem());}

    ///Returns the number of child items.
    Q_INVOKABLE int childCount(void){return m_treeWidgetItem->childCount();}

    ///Inserts the child item at index in the list of children.
    ///If the child has already been inserted somewhere else it won't be inserted again.
    Q_INVOKABLE void insertChild (int index, ScriptTreeWidgetItem* child){ emit insertChildSignal(index, m_treeWidgetItem, child->getWrappedItem());}

    ///Removes the item at index and returns it, otherwise return null.
    Q_INVOKABLE ScriptTreeWidgetItem* takeChild (int index)
    {
        QTreeWidgetItem* element = 0;
        ScriptTreeWidgetItem* scriptItem = 0;

        emit takeChildSignal(index, &element, m_treeWidgetItem);
        if(element != 0)
        {
            scriptItem  = (ScriptTreeWidgetItem*)element->data(0, ScriptTreeWidgetItem::SCRIPT_ITEM_POINTER_DATA_ROLE).toULongLong();
        }
       return scriptItem;

    }

    ///Deletes the current item.
    Q_INVOKABLE void deleteItem(void)
    {
        if(m_treeWidgetItem != 0)
        {
            if(m_treeWidgetItem->treeWidget() == 0)
            {//The item is not in a tree widget (the tree widget deletes all own items)
                emit deleteTreeWidgetItemSignal(m_treeWidgetItem);
                m_treeWidgetItem = 0;
            }
        }
        deleteLater();
    }

    ///Returns the index of the given child in the item's list of children.
    Q_INVOKABLE int indexOfChild(ScriptTreeWidgetItem* child){return m_treeWidgetItem->indexOfChild(child->getWrappedItem());}

    ///Sorts the children of the item using the given order(true=AscendingOrder,
    ///false=DescendingOrder) by the values in the given column.
    Q_INVOKABLE void sortChildren(int column, bool ascendingOrder){emit sortChildrenSignal(column, ascendingOrder, m_treeWidgetItem);}

    ///Returns the item's parent.
    Q_INVOKABLE ScriptTreeWidgetItem* parent (void)
    {
        QTreeWidgetItem* element = m_treeWidgetItem->parent();
        ScriptTreeWidgetItem* scriptItem = 0;
        if(element != 0)
        {
            scriptItem  = (ScriptTreeWidgetItem*)element->data(0, ScriptTreeWidgetItem::SCRIPT_ITEM_POINTER_DATA_ROLE).toULongLong();
        }
       return scriptItem;
    }

    ///Returns the number of columns in the item.
    Q_INVOKABLE int columnCount(void){return m_treeWidgetItem->columnCount();}

    ///Sets the background color of the label in the given column to the specified color.
    ///Possible colors are: black, white, gray, red, green, blue, cyan, magenta and yellow.
    Q_INVOKABLE void setBackgroundColor(int column, QString color){emit setBackgroundColorSignal(column, color, m_treeWidgetItem);}

    ///Sets the foreground color of the label in the given column to the specified color.
    ///Possible colors are: black, white, gray, red, green, blue, cyan, magenta and yellow.
    Q_INVOKABLE void setForegroundColor(int column, QString color){emit setForegroundColorSignal(column, color, m_treeWidgetItem);}

    ///Returns true if the item is expanded, otherwise returns false.
    Q_INVOKABLE bool isExpanded(void){return m_treeWidgetItem->isExpanded();}

    ///Expands the item if expand is true, otherwise collapses the item.
    Q_INVOKABLE void setExpanded(bool expand){emit expandItemSignal(m_treeWidgetItem, expand);}

    ///Sets the value for the item's column and role to the given value.
    ///The role describes the type of data specified by value.
    Q_INVOKABLE void setData (int column, quint8 role, QString value){m_treeWidgetItem->setData(column, SET_DATA_USER_ROLE + role, value);}

    ///Returns the value for the item's column and role to the given value.
    ///The role describes the type of data specified by value.
    Q_INVOKABLE QString data(int column, quint8 role){return m_treeWidgetItem->data(column, SET_DATA_USER_ROLE + role).toString();}

    ///Disables the item if disabled is true; otherwise enables the item.
    Q_INVOKABLE void setDisabled(bool disabled){emit setDisabledSignal(disabled, m_treeWidgetItem);}

    ///Returns true if the item is disabled; otherwise returns false.
    Q_INVOKABLE bool isDisabled(void){return m_treeWidgetItem->isDisabled();}

    ///Returns the wrapped item.
    QTreeWidgetItem* getWrappedItem(void){return m_treeWidgetItem;}

signals:
    ///This signal is emitted in expandItem.
    ///This signal is private and must not be used inside a script.
    void expandItemSignal(QTreeWidgetItem* item, bool expand);

    ///This signal is emitted in expandItem.
    ///This signal is private and must not be used inside a script.
    void deleteTreeWidgetItemSignal(QTreeWidgetItem* item);

    ///This signal is emitted in insertChild.
    ///This signal is private and must not be used inside a script.
    void insertChildSignal(int index, QTreeWidgetItem* rootItem, QTreeWidgetItem* child);

    ///This signal is emitted in setItemIcon.
    ///This signal is private and must not be used inside a script.
    void setItemIconSignal(QTreeWidgetItem* item , int column, QString iconFileName);

    ///This signal is emitted in setText.
    ///Scripts can connect a function to this signal.
    void setTextSignal(int column, QString text, QTreeWidgetItem* item);

    ///This signal is emitted in takeChild.
    ///This signal is private and must not be used inside a script.
    void takeChildSignal(int index, QTreeWidgetItem** child, QTreeWidgetItem* item);

    ///This signal is emitted in sortChildren.
    ///This signal is private and must not be used inside a script.
    void sortChildrenSignal(int column, bool ascendingOrder, QTreeWidgetItem* item);

    ///This signal is emitted in setBackgroundColor.
    ///This signal is private and must not be used inside a script.
    void setBackgroundColorSignal(int column, QString color, QTreeWidgetItem* item);

    ///This signal is emitted in setForegroundColor.
    ///This signal is private and must not be used inside a script.
    void setForegroundColorSignal(int column, QString color, QTreeWidgetItem* item);

    ///This signal is emitted in setDisabled.
    ///This signal is private and must not be used inside a script.
    void setDisabledSignal(bool disabled, QTreeWidgetItem* item);

private:
    ///The wrapped tree widget item.
    QTreeWidgetItem* m_treeWidgetItem;



};

#endif // SCRIPTTREEWIDGETITEM_H

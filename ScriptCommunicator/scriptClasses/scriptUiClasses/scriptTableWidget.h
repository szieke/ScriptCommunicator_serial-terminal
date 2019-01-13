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

#ifndef SCRIPTTABLEWIDGET_H
#define SCRIPTTABLEWIDGET_H

#include <QTableWidget>
#include <QDebug>
#include "scriptWidget.h"
#include "scriptComboBox.h"
#include "scriptTextEdit.h"
#include "scriptCheckBox.h"
#include "scriptButton.h"
#include "scriptPlotWindow.h"
#include "scriptProgressBar.h"
#include "scriptSpinBox.h"
#include "scriptTimeEdit.h"
#include "scriptDateEdit.h"
#include "scriptDateTimeEdit.h"
#include "scriptTextEdit.h"
#include "scriptSlider.h"
#include "scriptDoubleSpinBox.h"
#include "scriptCalendarWidget.h"
#include "scriptSlider.h"
#include "scriptLineEdit.h"

///The position of an item in the table.
struct ScriptTableCellPosition
{
    int row;
    int column;

    static QScriptValue toScriptValue(QScriptEngine *engine, const ScriptTableCellPosition &s)
    {
      QScriptValue obj = engine->newObject();
      obj.setProperty("row", s.row);
      obj.setProperty("column", s.column);
      return obj;
    }

    static void fromScriptValue(const QScriptValue &obj, ScriptTableCellPosition &s)
    {
      s.row = obj.property("row").toInt32();
      s.column = obj.property("column").toInt32();
    }

    static QScriptValue createScriptTableCellPosition(QScriptContext *, QScriptEngine *engine)
    {
        ScriptTableCellPosition s = {0, 0};
        return engine->toScriptValue(s);
    }


    static void registerType(QScriptEngine* engine)
    {
        qScriptRegisterMetaType<ScriptTableCellPosition>(engine, toScriptValue, fromScriptValue);
        qScriptRegisterSequenceMetaType<QVector<ScriptTableCellPosition> >(engine);

        QScriptValue ctor = engine->newFunction(createScriptTableCellPosition);
        engine->globalObject().setProperty("ScriptTableCellPosition", ctor);
    }
};
Q_DECLARE_METATYPE(ScriptTableCellPosition)

///This wrapper class is used to access a QTableWidget object (located in a script gui/ui-file) from a script.
class ScriptTableWidget: public ScriptWidget
{
    Q_OBJECT
public:
    ScriptTableWidget(QTableWidget* tableWidget, ScriptThread *scriptThread) :
        ScriptWidget(tableWidget, scriptThread, scriptThread->getScriptWindow()), m_tableWidget(tableWidget), m_scriptThread(scriptThread), m_rowsCanBeMovedByUser(true)
    {

        //connect the necessary signals with the wrapper slots (in this slots the
        //events of the wrapper class are generated, the script can connect to this
        //wrapper events)

        Qt::ConnectionType directConnectionType = scriptThread->runsInDebugger() ? Qt::DirectConnection : Qt::BlockingQueuedConnection;

        connect(m_tableWidget, SIGNAL(cellPressed(int, int)), this, SIGNAL(cellPressedSignal(int, int)));
        connect(m_tableWidget, SIGNAL(cellClicked(int, int)), this, SIGNAL(cellClickedSignal(int, int)));
        connect(m_tableWidget, SIGNAL(cellDoubleClicked(int, int)), this, SIGNAL(cellDoubleClickedSignal(int, int)));
        connect(m_tableWidget, SIGNAL(cellChanged(int, int)), this, SIGNAL(cellChangedSignal(int, int)));
        connect(m_tableWidget->horizontalHeader(), SIGNAL(sectionResized(int, int, int)), this, SIGNAL(horizontalHeaderSectionResizedSignal(int, int, int)));
        connect(m_tableWidget, SIGNAL(itemSelectionChanged()), this, SIGNAL(cellSelectionChangedSignal()), Qt::QueuedConnection);
        connect(m_tableWidget, SIGNAL(cellEntered(int, int)), this, SLOT(stub_cellEnteredSlot(int, int)), Qt::DirectConnection);

        connect(this, SIGNAL(insertRowSignal(int)), m_tableWidget, SLOT(insertRow(int)), directConnectionType);
        connect(this, SIGNAL(insertColumnSignal(int)), m_tableWidget, SLOT(insertColumn(int)), directConnectionType);
        connect(this, SIGNAL(removeRowSignal(int)), m_tableWidget, SLOT(removeRow(int)), directConnectionType);
        connect(this, SIGNAL(removeColumnSignal(int)), m_tableWidget, SLOT(removeColumn(int)), directConnectionType);
        connect(this, SIGNAL(clearSignal()), m_tableWidget, SLOT(clear()), directConnectionType);


        connect(this, SIGNAL(resizeColumnToContentsSignal(int,QTableWidget*)), scriptThread->getScriptWindow(),
                SLOT(resizeColumnToContentsSlot(int,QTableWidget*)), directConnectionType);
        connect(this, SIGNAL(resizeRowToContentsSignal(int,QTableWidget*)), scriptThread->getScriptWindow(),
                SLOT(resizeRowToContentsSlot(int,QTableWidget*)), directConnectionType);

        connect(this, SIGNAL(sortItemsSignal(int,bool,QTableWidget*)), scriptThread->getScriptWindow(),
                SLOT(sortItemsSlot(int,bool,QTableWidget*)), directConnectionType);

        connect(this, SIGNAL(insertWidgetSignal(ScriptThread*,QTableWidget*,int,int,QString,bool*)), scriptThread->getScriptWindow(),
                SLOT(insertWidgetInToTableSlot(ScriptThread*,QTableWidget*,int,int,QString,bool*)), directConnectionType);

        connect(this, SIGNAL(setItemSignal(int,int,QTableWidgetItem*,QTableWidget*)), scriptThread->getScriptWindow(),
                SLOT(setItemSlot(int,int,QTableWidgetItem*,QTableWidget*)), directConnectionType);

        connect(this, SIGNAL(setTextSignal(QString,QTableWidgetItem*)), scriptThread->getScriptWindow(),
                SLOT(setTextSlot(QString,QTableWidgetItem*)), directConnectionType);

        connect(this, SIGNAL(setVerticalHeaderItemSignal(int,QTableWidgetItem*,QTableWidget*)), scriptThread->getScriptWindow(),
                SLOT(setVerticalHeaderItemSlot(int,QTableWidgetItem*,QTableWidget*)), directConnectionType);

        connect(this, SIGNAL(setHorizontalHeaderItemSignal(int,QTableWidgetItem*,QTableWidget*)), scriptThread->getScriptWindow(),
                SLOT(setHorizontalHeaderItemSlot(int,QTableWidgetItem*,QTableWidget*)), directConnectionType);

        connect(this, SIGNAL(setFlagsSignal(Qt::ItemFlags,QTableWidgetItem*)), scriptThread->getScriptWindow(),
                SLOT(setFlagsSlot(Qt::ItemFlags,QTableWidgetItem*)), directConnectionType);

        connect(this, SIGNAL(setRowCountSignal(int,QTableWidget*)), scriptThread->getScriptWindow(),
                SLOT(setRowCountSlot(int,QTableWidget*)), directConnectionType);

        connect(this, SIGNAL(setColumnCountSignal(int,QTableWidget*)), scriptThread->getScriptWindow(),
                SLOT(setColumnCountSlot(int,QTableWidget*)), directConnectionType);

        connect(this, SIGNAL(setCellBackgroundColorSignal(QBrush,QTableWidgetItem*)), scriptThread->getScriptWindow(),
                SLOT(setCellBackgroundColorSlot(QBrush,QTableWidgetItem*)), directConnectionType);

        connect(this, SIGNAL(setCellForegroundColorSignal(QBrush,QTableWidgetItem*)), scriptThread->getScriptWindow(),
                SLOT(setCellForegroundColorSlot(QBrush,QTableWidgetItem*)), directConnectionType);

        connect(this, SIGNAL(setRowHeightSignal(int,int,QTableWidget*)), scriptThread->getScriptWindow(),
                SLOT(setRowHeightSlot(int,int,QTableWidget*)), directConnectionType);

        connect(this, SIGNAL(setColumnWidthSignal(int,int,QTableWidget*)), scriptThread->getScriptWindow(),
                SLOT(setColumnWidthSlot(int,int,QTableWidget*)), directConnectionType);

        connect(this, SIGNAL(setCellIconSignal(QTableWidgetItem*,QString)), scriptThread->getScriptWindow(),
                SLOT(setItemIconSlot(QTableWidgetItem*,QString)), directConnectionType);

        connect(this, SIGNAL(cellEnteredSignal(int,int,int,QTableWidget*)), scriptThread->getScriptWindow(),
                SLOT(cellEnteredSlot(int,int,int,QTableWidget*)), Qt::DirectConnection);

        connect(this, SIGNAL(insertRowWithContentSignal(int,QStringList,QStringList,QStringList,QTableWidget*)), scriptThread->getScriptWindow(),
                SLOT(insertRowWithContentSlot(int,QStringList,QStringList,QStringList,QTableWidget*)), directConnectionType);

        connect(this, SIGNAL(selectCellSignal(QTableWidget*,int,int,bool)), scriptThread->getScriptWindow(),
                SLOT(selectCellSlot(QTableWidget*,int,int,bool)), directConnectionType);



    }

    virtual ~ScriptTableWidget()
    {

    }

    ///Returns a semicolon separated list with all public functions, signals and properties.
    virtual QString getPublicScriptElements(void)
    {
        return MainWindow::parseApiFile("ScriptTableWidget.api");
    }

    ///Returns the text of one cell.
    Q_INVOKABLE QString getText(int row, int column)
    {
        QTableWidgetItem *item = m_tableWidget->item(row, column);
        QString text;
        if(item != 0)
        {
            text = item->text();
        }
        return text;
    }

    ///Sets the icon of a single cell.
    Q_INVOKABLE void setCellIcon(int row, int column, QString iconFileName)
    {
        if(m_tableWidget->rowCount() <= row)emit setRowCountSignal(row + 1, m_tableWidget);
        if(m_tableWidget->columnCount() <= column)emit setColumnCountSignal(column + 1, m_tableWidget);

        QTableWidgetItem *item = m_tableWidget->item(row, column);

        if(item == 0)
        {
            item = new QTableWidgetItem();
            emit setItemSignal(row, column,item, m_tableWidget);
        }

        emit setCellIconSignal(item, iconFileName);

    }

    ///Sets a vertical header label.
    Q_INVOKABLE void setVerticalHeaderLabel(int row, QString text)
    {
        QTableWidgetItem *item = m_tableWidget->verticalHeaderItem(row);
        if(item == 0)
        {
            item = new QTableWidgetItem(text);
        }
        else
        {
            emit setTextSignal(text, item);
        }
        emit setVerticalHeaderItemSignal(row, item, m_tableWidget);
    }

    ///Sets a horizontal header label.
    Q_INVOKABLE void setHorizontalHeaderLabel(int column, QString text)
    {
        QTableWidgetItem *item = m_tableWidget->horizontalHeaderItem(column);
        if(item == 0)
        {
            item = new QTableWidgetItem(text);
        }
        else
        {
            emit setTextSignal(text, item);
        }
        emit setHorizontalHeaderItemSignal(column, item, m_tableWidget);
    }

    ///Makes on cell editable or not editable.
    Q_INVOKABLE void setCellEditable(int row, int column, bool editable)
    {
        if(m_tableWidget->rowCount() <= row)emit setRowCountSignal(row + 1, m_tableWidget);
        if(m_tableWidget->columnCount() <= column)emit setColumnCountSignal(column + 1, m_tableWidget);

        QTableWidgetItem *item = m_tableWidget->item(row, column);

        if(item == 0)
        {
            item = new QTableWidgetItem();
            emit setItemSignal(row, column,item, m_tableWidget);
        }

        if(editable)
        {
            emit setFlagsSignal(item->flags() | Qt::ItemIsEditable, item);
        }
        else
        {
            emit setFlagsSignal(item->flags() ^ Qt::ItemIsEditable, item);
        }
    }

    ///Sets the row count.
    Q_INVOKABLE void setRowCount(int rows){emit setRowCountSignal(rows, m_tableWidget);}

    ///Return the row count.
    Q_INVOKABLE int rowCount(void){return m_tableWidget->rowCount();}

    ///Sets the column count.
    Q_INVOKABLE void setColumnCount(int columns){emit setColumnCountSignal(columns, m_tableWidget);}

    ///Returns the column count.
    Q_INVOKABLE int columnCount(void){return m_tableWidget->columnCount();}

    ///Sets the background color of a single cell.
    ///Possible colors are: black, white, gray, red, green, blue, cyan, magenta and yellow.
    Q_INVOKABLE void setCellBackgroundColor(QString color, int row, int column)
    {
        QTableWidgetItem *item = m_tableWidget->item(row, column);
        if(item == 0)
        {
            item = new QTableWidgetItem();
            emit setItemSignal(row, column,item, m_tableWidget);
        }

        emit setCellBackgroundColorSignal(ScriptWindow::stringToGlobalColor(color), item);

    }

    ///Sets the background color of a single cell with the RGB value.
    Q_INVOKABLE void setCellBackgroundColorRgb(int row, int column, int r, int g, int b, int alpha = 255)
    {
        QTableWidgetItem *item = m_tableWidget->item(row, column);
        if(item == 0)
        {
            item = new QTableWidgetItem();
            emit setItemSignal(row, column,item, m_tableWidget);
        }

        emit setCellBackgroundColorSignal(QColor(r, g, b, alpha), item);
    }

    ///Sets the foreground color of a single cell.
    ///Possible colors are: black, white, gray, red, green, blue, cyan, magenta and yellow.
    Q_INVOKABLE void setCellForegroundColor(QString color, int row, int column)
    {
        QTableWidgetItem *item = m_tableWidget->item(row, column);
        if(item == 0)
        {
            item = new QTableWidgetItem();
            emit setItemSignal(row, column,item, m_tableWidget);
        }

        emit setCellForegroundColorSignal(ScriptWindow::stringToGlobalColor(color), item);

    }

    ///Sets the foreground color of a single cell with the RGB value.
    Q_INVOKABLE void setCellForegroundColorRgb(int row, int column, int r, int g, int b, int alpha = 255)
    {
        QTableWidgetItem *item = m_tableWidget->item(row, column);
        if(item == 0)
        {
            item = new QTableWidgetItem();
            emit setItemSignal(row, column,item, m_tableWidget);
        }

        emit setCellForegroundColorSignal(QColor(r, g, b, alpha), item);

    }


    ///Resizes the column given to the size of its contents.
    Q_INVOKABLE void resizeColumnToContents(int column){emit resizeColumnToContentsSignal(column, m_tableWidget);}

    ///Resizes the row given to the size of its contents.
    Q_INVOKABLE void resizeRowToContents(int row){emit resizeRowToContentsSignal(row, m_tableWidget);}

    ///Sets the height of the given row to be height.
    Q_INVOKABLE void setRowHeight(int row, int height){emit setRowHeightSignal(row, height, m_tableWidget);}

    ///Returns the height of the given row.
    Q_INVOKABLE int rowHeight(int row){return m_tableWidget->rowHeight(row);}

    ///Sets the width of the given column to be width.
    Q_INVOKABLE void setColumnWidth(int column, int width){emit setColumnWidthSignal(column, width, m_tableWidget);}

    ///Returns the width of the given column.
    Q_INVOKABLE int columnWidth(int column){return m_tableWidget->columnWidth(column);}

    ///Returns the width of the frame that is drawn.
    Q_INVOKABLE int frameWidth(void){return m_tableWidget->frameWidth();}

    ///Returns the width of the vertical header.
    Q_INVOKABLE int verticalHeaderWidth(void){return m_tableWidget->verticalHeader()->width();}

    ///Returns the width of the vertical scroll bar.
    Q_INVOKABLE int verticalScrollBarWidth(void){return m_tableWidget->verticalScrollBar()->width();}

    ///Returns true if the vertical scroll bar is visible.
    Q_INVOKABLE bool isVerticalScrollBarVisible(void){return m_tableWidget->verticalScrollBar()->isVisible();}

    ///Sorts the items in the widget in the specified order(true=AscendingOrder,
    ///false=DescendingOrder) by the values in the given column.
    Q_INVOKABLE void sortItems(int column, bool ascendingOrder=true){emit sortItemsSignal(column, ascendingOrder, m_tableWidget);}

    ///If set to true the user can move a selected row up or down
    ///(while holding the left mouse button at the row and moving the mouse up and/or down).
    Q_INVOKABLE void rowsCanBeMovedByUser(bool canBeMoved){m_rowsCanBeMovedByUser = canBeMoved;}

    ///Returns the rows and columns of the selected cells.
    Q_INVOKABLE QVector<ScriptTableCellPosition> getAllSelectedCells(void)
    {
        QVector<ScriptTableCellPosition> result;

        QList<QTableWidgetSelectionRange>  ranges = m_tableWidget->selectedRanges();

        for(int i = 0; i < ranges.length(); i++)
        {
            QTableWidgetSelectionRange currentRange = ranges[i];

            for(int column = currentRange.leftColumn(); column <= currentRange.rightColumn();column++)
            {

                for(int row = currentRange.topRow(); row <= currentRange.bottomRow();row++)
                {
                    ScriptTableCellPosition pos;
                    pos.column = column;
                    pos.row = row;
                    result.append(pos);

                }
            }


        }
        return result;
    }

    ///Creates and inserts a script widget into a table cell.
    ///Possible type values are:
    ///- LineEdit
    ///- ComboBox
    ///- Button
    ///- CheckBox
    ///- SpinBox
    ///- DoubleSpinBox
    ///- VerticalSlider
    ///- HorizontalSlider
    ///- Dial
    ///- TimeEdit
    ///- DateEdit
    ///- DateTimeEdit
    ///- CalendarWidget
    ///- TextEdit
    Q_INVOKABLE bool insertWidget(int row, int column, QString type)
    {
        bool succeeded;
        emit insertWidgetSignal(m_scriptThread,m_tableWidget, row, column, type, &succeeded);
        return succeeded;
    }

    ///Sets the text of one cell.
    Q_INVOKABLE void setText(int row, int column, QString text)
    {
        if(m_tableWidget->rowCount() <= row)emit setRowCountSignal(row + 1, m_tableWidget);
        if(m_tableWidget->columnCount() <= column)emit setColumnCountSignal(column + 1, m_tableWidget);

        QTableWidgetItem *item = m_tableWidget->item(row, column);
        if(item == 0)
        {
            item = new QTableWidgetItem(text);
            emit setItemSignal(row, column,item, m_tableWidget);
        }
        else
        {
            emit setTextSignal(text, item);
        }

    }

    ///Returns the cell widget.
    Q_INVOKABLE QScriptValue getWidget(int row, int column)
    {
        QScriptValue result;

        QWidget* widget = m_tableWidget->cellWidget(row, column);

        if(widget)
        {
            QString type = widget->metaObject()->className();

            if(type == "QComboBox")
            {
                ScriptComboBox* scriptElement = new ScriptComboBox(static_cast<QComboBox*>(widget), m_scriptThread);
                result = m_scriptThread->getScriptEngine()->newQObject(scriptElement, QScriptEngine::ScriptOwnership);

            }
            else if(type == "QLineEdit")
            {
                ScriptLineEdit* scriptElement = new ScriptLineEdit(static_cast<QLineEdit*>(widget), m_scriptThread);
                result = m_scriptThread->getScriptEngine()->newQObject(scriptElement, QScriptEngine::ScriptOwnership);
            }
            else if(type == "QPushButton")
            {
                ScriptButton* scriptElement = new ScriptButton(static_cast<QPushButton*>(widget), m_scriptThread);
                result = m_scriptThread->getScriptEngine()->newQObject(scriptElement, QScriptEngine::ScriptOwnership);
            }
            else if(type == "QCheckBox")
            {
                ScriptCheckBox* scriptElement = new ScriptCheckBox(static_cast<QCheckBox*>(widget), m_scriptThread);
                result = m_scriptThread->getScriptEngine()->newQObject(scriptElement, QScriptEngine::ScriptOwnership);
            }
            else if(type == "QSpinBox")
            {
                ScriptSpinBox* scriptElement = new ScriptSpinBox(static_cast<QSpinBox*>(widget), m_scriptThread);
                result = m_scriptThread->getScriptEngine()->newQObject(scriptElement, QScriptEngine::ScriptOwnership);
            }
            else if(type == "QDoubleSpinBox")
            {
                ScriptDoubleSpinBox* scriptElement = new ScriptDoubleSpinBox(static_cast<QDoubleSpinBox*>(widget), m_scriptThread);
                result = m_scriptThread->getScriptEngine()->newQObject(scriptElement, QScriptEngine::ScriptOwnership);
            }
            else if(type == "QSlider")
            {
                ScriptSlider* scriptElement = new ScriptSlider(static_cast<QSlider*>(widget), m_scriptThread);
                result = m_scriptThread->getScriptEngine()->newQObject(scriptElement, QScriptEngine::ScriptOwnership);
            }
            else if(type == "QTimeEdit")
            {
                ScriptTimeEdit* scriptElement = new ScriptTimeEdit(static_cast<QTimeEdit*>(widget), m_scriptThread);
                result = m_scriptThread->getScriptEngine()->newQObject(scriptElement, QScriptEngine::ScriptOwnership);
            }
            else if(type == "QDateEdit")
            {
                ScriptDateEdit* scriptElement = new ScriptDateEdit(static_cast<QDateEdit*>(widget), m_scriptThread);
                result = m_scriptThread->getScriptEngine()->newQObject(scriptElement, QScriptEngine::ScriptOwnership);
            }
            else if(type == "QDateTimeEdit")
            {
                ScriptDateTimeEdit* scriptElement = new ScriptDateTimeEdit(static_cast<QDateTimeEdit*>(widget), m_scriptThread);
                result = m_scriptThread->getScriptEngine()->newQObject(scriptElement, QScriptEngine::ScriptOwnership);
            }
            else if(type == "QTextEdit")
            {
                ScriptTextEdit* scriptElement = new ScriptTextEdit(static_cast<QTextEdit*>(widget), m_scriptThread, m_scriptThread->getScriptWindow());
                result = m_scriptThread->getScriptEngine()->newQObject(scriptElement, QScriptEngine::ScriptOwnership);
            }
            else if(type == "QDial")
            {
                ScriptSlider* scriptElement = new ScriptSlider(static_cast<QDial*>(widget), m_scriptThread);
                result = m_scriptThread->getScriptEngine()->newQObject(scriptElement, QScriptEngine::ScriptOwnership);
            }
            else if(type == "QCalendarWidget")
            {
                ScriptCalendarWidget* scriptElement = new ScriptCalendarWidget(static_cast<QCalendarWidget*>(widget), m_scriptThread);
                result = m_scriptThread->getScriptEngine()->newQObject(scriptElement, QScriptEngine::ScriptOwnership);

            }
            else
            {//Unknown type.
            }

        }
        return result;
    }

    ///This function inserts one row at row and fills the cells with content.
    ///Possible colors are: black, white, gray, red, green, blue, cyan, magenta and yellow.
    Q_INVOKABLE void insertRowWithContent(int row, QStringList texts, QStringList backgroundColors, QStringList foregroundColors)
    {emit insertRowWithContentSignal(row, texts, backgroundColors, foregroundColors, m_tableWidget);}


    ///Selects a cell in a table widget.
    Q_INVOKABLE void selectCell(int row, int column, bool scrollToCell=true)
    {
        emit selectCellSignal(m_tableWidget, row, column, scrollToCell);
    }


public Q_SLOTS:

    ///This slot function inserts one row at row.
    void insertRow(int row){emit insertRowSignal(row);}

    ///This slot function inserts one column at column.
    void insertColumn(int column){emit insertColumnSignal(column);}

    ///This slot function removes the row at row.
    void removeRow(int row){emit removeRowSignal(row);}

    ///This slot function removes the column at column.
    void removeColumn(int column){emit removeColumnSignal(column);}

    ///This slot function clears the table (removes all cells).
    void clear(void){emit clearSignal();}


Q_SIGNALS:


    ///This signal is emitted if the user has pressed a cell.
    ///Scripts can connect a function to this signal
    void cellPressedSignal(int row, int column);

    ///This signal is emitted if the user has clicked a cell.
    ///Scripts can connect a function to this signal
    void cellClickedSignal(int row, int column);

    ///This signal is emitted if the user has double clicked a cell.
    ///Scripts can connect a function to this signal
    void cellDoubleClickedSignal(int row, int column);

    ///This signal is emitted if a cell has been changed.
    ///Scripts can connect a function to this signal
    void cellChangedSignal(int row, int column);

    ///This signal is emitted if a horizontal header section is resized.
    ///The section's logical number is specified by logicalIndex, the old size by oldSize, and the new size by newSize.
    ///Scripts can connect a function to this signal
    void horizontalHeaderSectionResizedSignal(int logicalIndex, int oldSize, int newSize);

    ///This signal is emitted whenever the selection changes.
    ///Scripts can connect a function to this signal
    void cellSelectionChangedSignal(void);

    ///This signal is emitted in resizeRowToContents.
    ///This signal is private and must not be used inside a script.
    void resizeRowToContentsSignal(int row, QTableWidget* table);

    ///This signal is emitted in resizeColumnToContents.
    ///This signal is private and must not be used inside a script.
    void resizeColumnToContentsSignal(int column, QTableWidget* table);

    ///This signal is emitted in insertWidget.
    ///This signal is private and must not be used inside a script.
    void insertWidgetSignal(ScriptThread *scriptThread, QTableWidget* table, int row, int column, QString type, bool* succeeded);

    ///This signal is emitted in sortItems.
    ///This signal is private and must not be used inside a script.
    void sortItemsSignal(int column, bool ascendingOrder, QTableWidget* tableWidget);

    ///This signal is emitted in several function.
    ///This signal is private and must not be used inside a script.
    void setItemSignal(int row, int column, QTableWidgetItem *item, QTableWidget* tableWidget);

    ///This signal is emitted in several function.
    ///This signal is private and must not be used inside a script.
    void setTextSignal(const QString text, QTableWidgetItem *item);

    ///This signal is emitted in setCellIcon.
    ///This signal is private and must not be used inside a script.
    void setCellIconSignal(QTableWidgetItem *item, const QString iconFileName);

    ///This signal is emitted in setVerticalHeaderLabel.
    ///This signal is private and must not be used inside a script.
    void setVerticalHeaderItemSignal(int row, QTableWidgetItem *item, QTableWidget* tableWidget);

    ///This signal is emitted in setHorizontalHeaderItem.
    ///This signal is private and must not be used inside a script.
    void setHorizontalHeaderItemSignal(int column, QTableWidgetItem *item, QTableWidget* tableWidget);

    ///This signal is emitted in setCellEditable.
    ///This signal is private and must not be used inside a script.
    void setFlagsSignal(Qt::ItemFlags flags, QTableWidgetItem *item);

    ///This signal is emitted in several function.
    ///This signal is private and must not be used inside a script.
    void setRowCountSignal(int rows, QTableWidget* tableWidget);

    ///This signal is emitted in several function.
    ///This signal is private and must not be used inside a script.
    void setColumnCountSignal(int columns, QTableWidget* tableWidget);

    ///This signal is emitted in setCellBackgroundColor.
    ///This signal is private and must not be used inside a script.
    void setCellBackgroundColorSignal(QBrush brush, QTableWidgetItem *item);

    ///This signal is emitted in setCellBackgroundColor.
    ///This signal is private and must not be used inside a script.
    void setCellForegroundColorSignal(QBrush brush, QTableWidgetItem *item);

    ///This signal is emitted in setRowHeight.
    ///This signal is private and must not be used inside a script.
    void setRowHeightSignal(int row, int height, QTableWidget* tableWidget);

    ///This signal is emitted in setColumnWidth.
    ///This signal is private and must not be used inside a script.
    void setColumnWidthSignal(int column, int width, QTableWidget* tableWidget);

    ///This signal is emitted in stub_cellEnteredSlot.
    ///This signal is private and must not be used inside a script.
    void cellEnteredSignal(int row, int rowsel, int column, QTableWidget* tableWidget);

    ///This signal is emitted in insertRow.
    ///This signal is private and must not be used inside a script.
    void insertRowSignal(int row);

    ///This signal is emitted in insertColumn.
    ///This signal is private and must not be used inside a script.
    void insertColumnSignal(int column);

    ///This signal is emitted in removeRow.
    ///This signal is private and must not be used inside a script.
    void removeRowSignal(int row);

    ///This signal is emitted in removeColumn.
    ///This signal is private and must not be used inside a script.
    void removeColumnSignal(int column);

    ///This signal is emitted in clear.
    ///This signal is private and must not be used inside a script.
    void clearSignal(void);

    ///This signal is emitted in insertRowWithContent.
    ///This signal is private and must not be used inside a script.
    void insertRowWithContentSignal(int row, QStringList texts, QStringList backgroundColors, QStringList foregroundColors, QTableWidget* tableWidget);

    ///This signal is emitted in selectCell.
    ///This signal is private and must not be used inside a script.
    void selectCellSignal(QTableWidget* tableWidget, int row, int column, bool scrollToCell);

private Q_SLOTS:


    ///With this function the user can move the selected row up or down
    ///(while holding the left mouse button at the row and moving the mouse up and/or down).
    void stub_cellEnteredSlot(int row, int column)
    {
        int rowsel;

        if(m_rowsCanBeMovedByUser)
        {

            //Stop from entering again.
            m_rowsCanBeMovedByUser = false;

            if(m_tableWidget->currentIndex().row()<row)
            {
                //the position of the selected row has to be decremented
                rowsel=row-1;
            }
            else if(m_tableWidget->currentIndex().row()>row)
            {
                //the position of the selected row has to be decremented
                rowsel=row+1;
            }
            else
            {   //the position of the selected row is not changed
                rowsel = row;
            }

            if(rowsel != row)
            {//the selected row has to change his position with an other row

                emit cellEnteredSignal(row, rowsel, column, m_tableWidget);
            }

            m_rowsCanBeMovedByUser = true;
        }
    }
private:
    ///The wrapped table widget.
    QTableWidget* m_tableWidget;

    ///The script thread;
    ScriptThread* m_scriptThread;

    ///If true the user can move a selected row up or down
    ///(while holding the left mouse button at the row and moving the mouse up and/or down).
    bool m_rowsCanBeMovedByUser;


};

#endif // SCRIPTTABLEWIDGET_H

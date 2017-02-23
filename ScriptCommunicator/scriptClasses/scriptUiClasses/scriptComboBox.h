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

#ifndef SCRIPTWIDGETS_H
#define SCRIPTWIDGETS_H

#include "QComboBox"
#include "scriptWidget.h"

///This wrapper class is used to access a QComboBox object (located in a script gui/ui-file) from a script.
class ScriptComboBox : public ScriptWidget
{
    Q_OBJECT

public:
    ScriptComboBox(QComboBox* comboBox, ScriptThread *scriptThread):
        ScriptWidget(comboBox, scriptThread, scriptThread->getScriptWindow()), m_comboBox(comboBox)
    {
        //connect the necessary signals with the wrapper slots (in this slots the
        //events of the wrapper class are generated, the script can connect to this
        //wrapper events)

        Qt::ConnectionType directConnectionType = scriptThread->runsInDebugger() ? Qt::DirectConnection : Qt::BlockingQueuedConnection;

        connect(m_comboBox, SIGNAL(currentTextChanged(QString)), this, SIGNAL(currentTextChangedSignal(QString)));
        connect(m_comboBox, SIGNAL(currentIndexChanged(int)), this, SIGNAL(currentIndexChangedSignal(int)));


        connect(this, SIGNAL(addItemSignal(QString,QComboBox*)), scriptThread->getScriptWindow(),
                SLOT(addItemSlot(QString,QComboBox*)), directConnectionType);

        connect(this, SIGNAL(insertItemSignal(int,QString,QComboBox*)), scriptThread->getScriptWindow(),
                SLOT(insertItemSlot(int,QString,QComboBox*)), directConnectionType);

        connect(this, SIGNAL(removeItemSignal(int,QComboBox*)), scriptThread->getScriptWindow(),
                SLOT(removeItemSlot(int,QComboBox*)), directConnectionType);

        connect(this, SIGNAL(setEditableSignal(bool,QComboBox*)), scriptThread->getScriptWindow(),
                SLOT(setEditableSlot(bool,QComboBox*)), directConnectionType);

        connect(this, SIGNAL(setItemTextSignal(int,QString,QComboBox*)), scriptThread->getScriptWindow(),
                SLOT(setItemTextSlot(int,QString,QComboBox*)), directConnectionType);

        connect(this, SIGNAL(setCurrentTextSignal(QString)), m_comboBox, SLOT(setCurrentText(QString)), directConnectionType);

        connect(this, SIGNAL(setCurrentIndexSignal(int)), m_comboBox, SLOT(setCurrentIndex(int)), directConnectionType);

        connect(this, SIGNAL(clearSignal()), m_comboBox, SLOT(clear()), directConnectionType);

    }
    virtual ~ScriptComboBox()
    {

    }
    ///Returns a semicolon separated list with all public functions, signals and properties.
    virtual QString getPublicScriptElements(void)
    {
        return MainWindow::parseApiFile("ScriptComboBox.api");
    }

    ///Adds one item to the combo box.
    Q_INVOKABLE void addItem(const QString text){emit addItemSignal(text, m_comboBox);}

    ///Inserts one item into the combo box.
    Q_INVOKABLE void insertItem(int index, const QString text){emit insertItemSignal(index, text, m_comboBox);}

    ///Removes one item from the combo box.
    Q_INVOKABLE void removeItem(int index){emit removeItemSignal(index, m_comboBox);}

    ///Sets the editable property of the combo box.
    ///If the editable property is true, then the text of the selected item can be changed.
    Q_INVOKABLE void setEditable(bool editable){emit setEditableSignal(editable, m_comboBox);}

    ///Returns true if the combo box is editable.
    ///If the editable property is true, then the text of the selected item can be changed.
    Q_INVOKABLE bool isEditable(void){return m_comboBox->isEditable();}

    ///Returns the index of the current selected item.
    Q_INVOKABLE int currentIndex(void){return m_comboBox->currentIndex();}

    ///Returns the text of the current selected item.
    Q_INVOKABLE QString currentText(void){return m_comboBox->currentText();}

    ///Returns the item (identified by index) text.
    Q_INVOKABLE QString itemText(int index){return m_comboBox->itemText(index);}

    ///Sets the item (identified by index) text.
    Q_INVOKABLE void setItemText(int index, const QString text){emit setItemTextSignal(index, text, m_comboBox);}

    ///Sets the text of the current selected item.
    Q_INVOKABLE void setCurrentText(const QString text){emit setCurrentTextSignal(text);}

    ///Sets the index of the current selected item.
    Q_INVOKABLE void setCurrentIndex(int index){emit setCurrentIndexSignal(index);}

    ///Returns the number of items in the combo box.
    Q_INVOKABLE int count(void){return m_comboBox->count();}

    ///Clears the combo box and removes all items.
    Q_INVOKABLE void clear(void){emit clearSignal();}

Q_SIGNALS:
    ///This signal is emitted if the text of the current selected item has been changed
    ///(new index or the text has been modified).
    ///Scripts can connect a function to this signal.
    void currentTextChangedSignal(QString newText);

    ///This signal is emitted if the current selected index has been changed.
    ///Scripts can connect a function to this signal.
    void currentIndexChangedSignal(int currentSelectedIndex);

    ///Is emitted in addItem.
    ///This signal is private and must not be used inside a script.
    void addItemSignal(const QString text, QComboBox* box);

    ///Is emitted in insertItem.
    ///This signal is private and must not be used inside a script.
    void insertItemSignal(int index, const QString text, QComboBox* box);

    ///Is emitted in removeItem.
    ///This signal is private and must not be used inside a script.
    void removeItemSignal(int index, QComboBox* box);

    ///Is emitted in setEditable.
    ///This signal is private and must not be used inside a script.
    void setEditableSignal(bool editable, QComboBox* box);

    ///Is emitted in setItemText.
    ///This signal is private and must not be used inside a script.
    void setItemTextSignal(int index, const QString text, QComboBox* box);

    ///Is emitted in setCurrentText.
    ///This signal is private and must not be used inside a script.
    void setCurrentTextSignal(const QString &text);

    ///Is emitted in setCurrentIndex.
    ///This signal is private and must not be used inside a script.
    void setCurrentIndexSignal(int index);

    ///Is emitted in clear.
    ///This signal is private and must not be used inside a script.
    void clearSignal(void);


private:
    QComboBox* m_comboBox;

};

#endif // SCRIPTWIDGETS_H

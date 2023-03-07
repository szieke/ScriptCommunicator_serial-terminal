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

#ifndef SCRIPTGROUPBOX_H
#define SCRIPTGROUPBOX_H

#include <QObject>

#include "scriptWidget.h"
#include "scriptPlotWidget.h"
#include "plotwidget.h"


///This wrapper class is used to access a QGroupBox object (located in a script gui/ui-file) from a script.
class ScriptGroupBox : public ScriptWidget
{
    Q_OBJECT
public:
    explicit ScriptGroupBox(QGroupBox* box, ScriptThread *scriptThread) :
        ScriptWidget(box, scriptThread, scriptThread->getScriptWindow()), m_box(box), m_scriptThread(scriptThread)
    {
        //connect the necessary signals with the wrapper slots (in this slots the
        //events of the wrapper class are generated, the script can connect to this
        //wrapper events)

        Qt::ConnectionType directConnectionType = scriptThread->runsInDebugger() ? Qt::DirectConnection : Qt::BlockingQueuedConnection;

        connect(this, SIGNAL(setTitleSignal(QString,QGroupBox*)), scriptThread->getScriptWindow(),
                SLOT(setTitleSlot(QString,QGroupBox*)), directConnectionType);


        connect(this, SIGNAL(createGuiElementSignal(QString,QObject**, ScriptWindow*, ScriptThread*,QObject*)), scriptThread->getScriptWindow(),
                SLOT(createGuiElementSlot(QString,QObject**,ScriptWindow*,ScriptThread*,QObject*)), directConnectionType);


        connect(this, SIGNAL(setCheckedSignal(bool)), m_box,SLOT(setChecked(bool)), directConnectionType);
        connect(m_box, SIGNAL(clicked(bool)), this,SIGNAL(checkBoxClickedSignal(bool)), Qt::QueuedConnection);

    }

    ///Returns a semicolon separated list with all public functions, signals and properties.
    virtual QString getPublicScriptElements(void)
    {
        return MainWindow::parseApiFile("ScriptGroupBox.api");
    }

    ///Sets the group box title.
    Q_INVOKABLE void setTitle(QString title){emit setTitleSignal(title, m_box);}

    ///Returns the group box title.
    Q_INVOKABLE QString title(void){return m_box->title();}

    ///Adds a plot widget to the group box.
    Q_INVOKABLE QJSValue addPlotWidget(void)
    {
        QJSValue scriptObj;
        QHBoxLayout* layout = 0;
        PlotWidget* widget = 0;
        ScriptPlotWidget* scriptPlotWidget= 0;
        emit createGuiElementSignal("QHBoxLayout", (QObject**)&layout, m_scriptWindow, m_scriptThread, m_box);
        m_box->setLayout(layout);
        emit createGuiElementSignal("PlotWidget", (QObject**)&widget, m_scriptWindow, m_scriptThread, layout);

        if(widget != 0)
         {
             scriptPlotWidget = new ScriptPlotWidget(widget, m_scriptThread, m_scriptWindow);
             m_scriptThread->addCreatedGuiElement(scriptPlotWidget);

             scriptObj = m_scriptThread->getScriptEngine()->newQObject(scriptPlotWidget);
             QJSEngine::setObjectOwnership(scriptPlotWidget, QJSEngine::CppOwnership);
         }

        return scriptObj;
    }

    ///Checks or unchecks the group box checkbox.
    Q_INVOKABLE void setChecked(bool checked){emit setCheckedSignal(checked);}

    ///Returns true if the groub box check box is checked.
    Q_INVOKABLE bool isChecked(void){return m_box->isChecked();}

Q_SIGNALS:

    ///Is emitted if the group box checkbox is clicked.
    void checkBoxClickedSignal(bool checked);


    ///Is emitted in setTitle.
    ///This signal is private and must not be used inside a script.
    void setTitleSignal(const QString text, QGroupBox* box);

    ///This signal is used to create a gui element (for instance PlotWindow).
    ///Is connected with ScriptWindow::createGuiElementSlot (to signalize the thread state change).
    /// Note: Gui elements in Qt can only be created in the main thread.
    ///This function must not be used from script.
    void createGuiElementSignal(QString elementType, QObject** createdGuiElement, ScriptWindow* scriptWindow, ScriptThread* thread, QObject* additionalArgument);

    ///This signal is used to checks or unchecks the checkbox
    ///Is connected with ScriptWindow::createGuiElementSlot (to signalize the thread state change).
    void setCheckedSignal(bool checked);

    void addCreatedGuiElementsFromScriptSignal(QObject* entry);

private:
    ///The wrapped push button.
    QGroupBox* m_box;

    ///Pointer to the script thread.
    ScriptThread* m_scriptThread;
};

#endif // SCRIPTGROUPBOX_H

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
#include "scriptPlotwidget.h"

#include "context2d.h"
#include "qcontext2dcanvas.h"

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
    Q_INVOKABLE ScriptPlotWidget* addPlotWidget(void)
    {
        QHBoxLayout* layout = 0;
        ScriptPlotWidget* widget = 0;
        emit createGuiElementSignal("QHBoxLayout", (QObject**)&layout, m_scriptWindow, m_scriptThread, m_box);
        m_box->setLayout(layout);
        emit createGuiElementSignal("ScriptPlotWidget", (QObject**)&widget, m_scriptWindow, m_scriptThread, layout);
        return widget;
    }

    ///Adds a Canvas2D object to the group box.
    Q_INVOKABLE QScriptValue addCanvas2DWidget(void)
    {
        Context2D* context = new Context2D();
        QContext2DCanvas* widget;
        context->setSize(m_box->size().width(), m_box->size().height());
        emit createGuiElementSignal("QContext2DCanvas", (QObject**)&widget, m_scriptWindow, m_scriptThread, m_box);
        widget->connectContextSignals(context);


        return m_scriptThread->getScriptEngine()->newQObject(context, QScriptEngine::ScriptOwnership);;

    }

Q_SIGNALS:

    ///Is emitted in setTitle.
    ///This signal is private and must not be used inside a script.
    void setTitleSignal(const QString text, QGroupBox* box);

    ///This signal is used to create a gui element (for instance PlotWindow).
    ///Is connected with ScriptWindow::createGuiElementSlot (to signalize the thread state change).
    /// Note: Gui elements in Qt can only be created in the main thread.
    ///This function must not be used from script.
    void createGuiElementSignal(QString elementType, QObject** createdGuiElement, ScriptWindow* scriptWindow, ScriptThread* thread, QObject* additionalArgument);

    void addCreatedGuiElementsFromScriptSignal(QObject* entry);

private:
    ///The wrapped push button.
    QGroupBox* m_box;

    ///Pointer to the script thread.
    ScriptThread* m_scriptThread;
};

#endif // SCRIPTGROUPBOX_H

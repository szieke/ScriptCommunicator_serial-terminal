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

#ifndef SCRIPSPLITTER_H
#define SCRIPSPLITTER_H

#include <QObject>

#include "scriptWidget.h"
#include "scriptObject.h"

///This wrapper class is used to access a QSplitter object (located in a script gui/ui-file) from a script.
class ScriptSplitter : public QObject, public ScriptObject
{
    Q_OBJECT
    Q_PROPERTY(QString publicScriptElements READ getPublicScriptElements)
public:
    explicit ScriptSplitter(QSplitter* splitter, ScriptThread *scriptThread) :
        QObject(scriptThread), m_splitter(splitter)
    {
        //connect the necessary signals with the wrapper slots (in this slots the
        //events of the wrapper class are generated, the script can connect to this
        //wrapper events)
        Qt::ConnectionType directConnectionType = scriptThread->runsInDebugger() ? Qt::DirectConnection : Qt::BlockingQueuedConnection;

        connect(this, SIGNAL(setSplitterSizes(QSplitter*,QList<int>&)), scriptThread->getScriptWindow(),
                SLOT(setSplitterSizesSlot(QSplitter*,QList<int>&)), directConnectionType);

    }

    ///Returns a semicolon separated list with all public functions, signals and properties.
    virtual QString getPublicScriptElements(void)
    {
        return MainWindow::parseApiFile("ScriptSplitter.api");
    }

    ///Returns a list of the size parameters of all the widgets in this splitter.
    ///If the splitter's orientation is horizontal, the list contains the widgets width in pixels,
    ///from left to right; if the orientation is vertical, the list contains the widgets height in pixels, from top to bottom.
    Q_INVOKABLE QList<int> sizes(void){return m_splitter->sizes();}

    ///Sets the child widgets respective sizes to the values given in the list.
    ///If the splitter is horizontal, the values set the width of each widget in pixels, from left to right.
    ///If the splitter is vertical, the height of each widget is set, from top to bottom.
    ///Extra values in the list are ignored. If list contains too few values, the result is undefined but the program will still be well-behaved.
    ///The overall size of the splitter widget is not affected. Instead, any additional/missing space is distributed amongst the widgets according
    ///to the relative weight of the sizes.
    ///If you specify a size of 0, the widget will be invisible. The size policies of the widgets are preserved.
    ///That is, a value smaller then the minimal size hint of the respective widget will be replaced by the value of the hint.
    Q_INVOKABLE void setSizes (QList<int> list){emit setSplitterSizes(m_splitter, list);}


Q_SIGNALS:
    ///Is emitted by the setSizes function.
    ///This signal is private and must not be used inside a script.
    void setSplitterSizes (QSplitter* splitter, QList<int>& list);
private:
    ///The wrapped splitter.
    QSplitter* m_splitter;
};

#endif // SCRIPSPLITTER_H

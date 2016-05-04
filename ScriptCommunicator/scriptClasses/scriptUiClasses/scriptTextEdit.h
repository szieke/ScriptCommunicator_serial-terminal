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

#ifndef SCRIPTTEXTEDIT_H
#define SCRIPTTEXTEDIT_H

#include <QObject>
#include <QTextEdit>
#include "mainwindow.h"
#include "scriptWidget.h"

///This wrapper class is used to access a QTextEdit object (located in a script gui/ui-file) from a script.
class ScriptTextEdit : public ScriptWidget
{
    Q_OBJECT
public:
    explicit ScriptTextEdit(QTextEdit* textEdit, ScriptThread *scriptThread, ScriptWindow* scriptWindow ):
        ScriptWidget(textEdit, scriptThread, scriptThread->getScriptWindow()), m_textEdit(textEdit), m_maxChars(100000), m_lockScrolling(false)
    {

        //connect the necessary signals with the wrapper slots (in this slots the
        //events of the wrapper class are generated, the script can connect to this
        //wrapper events)
        Qt::ConnectionType directConnectionType = scriptThread->runsInDebugger() ? Qt::DirectConnection : Qt::BlockingQueuedConnection;

        connect(m_textEdit, SIGNAL(textChanged()), this, SLOT(stub_textChangedSlot()));
        connect(this, SIGNAL(limtCharsInTextEditSignal(QTextEdit*,int)),
                scriptWindow, SLOT(limtCharsInTextEditSlot(QTextEdit*,int)), directConnectionType);
        connect(this, SIGNAL(setPlainTextSignal(QString)),m_textEdit, SLOT(setPlainText(QString)), directConnectionType);
        connect(this, SIGNAL(setTextSignal(QString)), m_textEdit, SLOT(setText(QString)), directConnectionType);
        connect(this, SIGNAL(moveTextPositionToEndSignal(QTextEdit*)),scriptWindow, SLOT(moveTextPositionToEndSlot(QTextEdit*)), directConnectionType);
        connect(this, SIGNAL(verticalScrollBarSetValueSignal(int)),m_textEdit->verticalScrollBar(), SLOT(setValue(int)), directConnectionType);
        connect(this, SIGNAL(clearSignal()),m_textEdit, SLOT(clear()), directConnectionType);
        connect(this, SIGNAL(setFontPointSizeSignal(qreal)),m_textEdit, SLOT(setFontPointSize(qreal)), directConnectionType);
        connect(this, SIGNAL(setFontFamilySignal(QString)),m_textEdit, SLOT(setFontFamily(QString)), directConnectionType);

        connect(this, SIGNAL(writeTextSignal(QTextEdit*,QString,bool,bool,bool,bool,quint32,bool)), scriptThread->getScriptWindow(),
                SLOT(writeTextSlot(QTextEdit*,QString,bool,bool,bool,bool,quint32,bool)), directConnectionType);

    }


    ///Returns the vertical scroll bar value.
    Q_INVOKABLE int verticalScrollBarValue(void){return m_textEdit->verticalScrollBar()->value();}

    ///Sets the vertical scroll bar value.
    Q_INVOKABLE void verticalScrollBarSetValue(int value){ emit verticalScrollBarSetValueSignal(value);}

    ///Returns the content of the text edit as plain text.
    Q_INVOKABLE QString toPlainText(void){return m_textEdit->toPlainText();}

    ///Returns the content of the text edit as html.
    Q_INVOKABLE QString toHtml(void){return m_textEdit->toHtml();}

    ///Sets the max. number of chars in the text edit.
    Q_INVOKABLE void setMaxChars(int maxChars){ m_maxChars = maxChars;}

    ///Replaces the characters '\n',' ', '<' and '>' to their html representation.
    Q_INVOKABLE QString replaceNonHtmlChars(QString text)
    {
        text.replace("<", "&lt;");
        text.replace(">", "&gt;");
        text.replace("\n", "<br>");
        text.replace(" ", "&nbsp;");
        return text;
    }
    ///Moves the curser to the end of the text edit.
    Q_INVOKABLE void moveTextPositionToEnd(void){emit moveTextPositionToEndSignal(m_textEdit);}

public Q_SLOTS:

    ///This slot function sets font size.
    void setFontPointSize(qreal fontSize){emit setFontPointSizeSignal(fontSize);}

    ///This slot function sets font family.
    void setFontFamily(QString fontFamily){emit setFontFamilySignal(fontFamily);}

    ///This slot function clears the text edit.
    void clear(void){emit clearSignal();}

    ///This slot function inserts plain text into the text edit.
    void insertPlainText(QString text, bool atTheEnd=true){emit writeTextSignal(m_textEdit, text, false, true, false, m_lockScrolling, m_maxChars, atTheEnd);}

    ///This slot function inserts HTML text into the text edit.
    void insertHtml(QString htmlString, bool atTheEnd=true){emit writeTextSignal(m_textEdit, htmlString, true, false, false, m_lockScrolling, m_maxChars, atTheEnd);}

    ///This slot function appends text at the end of text edit (includes a new line) and moves the cursor to the end of the text.
    void append(QString text){emit writeTextSignal(m_textEdit, text, false, false, true, m_lockScrolling, m_maxChars, true);}

    ///This slot function sets the text of the text edit (plain text).
    void setPlainText(QString text){emit setPlainTextSignal(text); emit limtCharsInTextEditSignal(m_textEdit, m_maxChars);}

    ///This slot function sets the text of the text edit.
    void setText(QString text){emit setTextSignal(text); emit limtCharsInTextEditSignal(m_textEdit, m_maxChars);}

	///Locks or unlocks the scrolling of the vertical scroll bar.
    void lockScrolling(bool lock){m_lockScrolling = lock;}

Q_SIGNALS:
    ///This signal is emitted if the text of the text edit has been changed.
    ///Scripts can connect a function to this signal.
    void textChangedSignal(void);

    ///This signal is emitted if the number of chars in text has to be limited.
    ///If there are more then maxChars characters in the text edit then the first characters will be removed.
    ///This signal is private and must not be used inside a script.
    void limtCharsInTextEditSignal(QTextEdit* textEdit, const int maxChars);

    ///This signal is emitted if the setPlainText function is called.
    ///This signal is private and must not be used inside a script.
    void setPlainTextSignal(const QString &text);

    ///This signal is emitted if the setText function is called.
    ///This signal is private and must not be used inside a script.
    void setTextSignal(const QString &text);

    ///This signal is emitted if the moveTextPositionToEnd function is called.
    ///This signal is private and must not be used inside a script.
    void moveTextPositionToEndSignal(QTextEdit* textEdit);

    ///This signal is emitted if the verticalScrollBarSetValue function is called.
    ///This signal is private and must not be used inside a script.
    void verticalScrollBarSetValueSignal(int);

    ///This signal is emitted if the setFontPointSize function is called.
    ///This signal is private and must not be used inside a script.
    void setFontPointSizeSignal(qreal);

    ///This signal is emitted if the clear function is called.
    ///This signal is private and must not be used inside a script.
    void clearSignal();

    ///This signal is emitted if the setFontFamily function is called.
    ///This signal is private and must not be used inside a script.
    void setFontFamilySignal(QString);

    ///This signal is emitted in several add functions.
    ///This signal is private and must not be used inside a script.
    void writeTextSignal(QTextEdit* textEdit, QString text, bool insertHtml, bool insertText,
                         bool append, bool isLocked, quint32 maxChars, bool atTheEnd);

private Q_SLOTS:
    ///This slot function is called if the text of the text edit has been changed.
    void stub_textChangedSlot(){ emit textChangedSignal();}

private:

    ///The wrapped text edit.
    QTextEdit* m_textEdit;

    ///The maximum number of chars in the text edit.
    int m_maxChars;

    ///True if the scrolling is locked.
    bool m_lockScrolling;

};

#endif // SCRIPTTEXTEDIT_H

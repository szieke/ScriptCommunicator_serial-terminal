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

#ifndef SCRIPTLINEEDIT_H
#define SCRIPTLINEEDIT_H

#include "QLineEdit"
#include <QIntValidator>
#include <QDoubleValidator>
#include <QRegularExpression>
#include "scriptWidget.h"

///This wrapper class is used to access a QLineEdit object (located in a script gui/ui-file) from a script.
class ScriptLineEdit: public ScriptWidget
{
    Q_OBJECT

    ///Returns a semicolon separated list with all public functions, signals and properties.
    Q_PROPERTY(QString publicScriptElements READ getPublicScriptElements CONSTANT)

public:
    ScriptLineEdit(QLineEdit* lineEdit, ScriptThread *scriptThread) :
        ScriptWidget(lineEdit,scriptThread, scriptThread->getScriptWindow()), m_lineEdit(lineEdit), m_hexModeActivated(false),
        m_maxValue(0), m_savedText()
    {
        //connect the necessary signals with the wrapper slots (in this slots the
        //events of the wrapper class are generated, the script can connect to this
        //wrapper events)
        Qt::ConnectionType directConnectionType = scriptThread->runsInDebugger() ? Qt::DirectConnection : Qt::BlockingQueuedConnection;

        connect(m_lineEdit, SIGNAL(textChanged(QString&)),this, SIGNAL(textChangedSignal(QString&)));

        connect(this, SIGNAL(setTextSignal(QString)),m_lineEdit, SLOT(setText(QString)), directConnectionType);

        connect(this, SIGNAL(setReadOnlySignal(bool,QLineEdit*)),scriptThread->getScriptWindow(),
                SLOT(setReadOnlySlot(bool,QLineEdit*)), directConnectionType);

        connect(this, SIGNAL(clearSignal()),m_lineEdit, SLOT(clear()), directConnectionType);

        connect(this, SIGNAL(addValidatorSignal(QValidator*,QLineEdit*)),scriptThread->getScriptWindow(),
                SLOT(addValidatorSignal(QValidator*,QLineEdit*)), directConnectionType);

    }

    ///Returns a semicolon separated list with all public functions, signals and properties.
    virtual QString getPublicScriptElements(void)
    {
        return MainWindow::parseApiFile("ScriptLineEdit.api");
    }

    ///Returns the text of the line exit.
    Q_INVOKABLE QString text(void){return m_lineEdit->text();}

    ///Returns true of the line edit is editable.
    Q_INVOKABLE bool isReadOnly(void){return m_lineEdit->isReadOnly();}

    ///Sets the editable property of the line edit.
    Q_INVOKABLE void setReadOnly(bool readOnly){emit setReadOnlySignal(readOnly, m_lineEdit);}

    ///Adds an int validator to the line edit
    ///(this ensures that the line edit contains only integer).
    Q_INVOKABLE void addIntValidator(int bottom, int top){addValidator(new QIntValidator(bottom, top));}

    ///Adds an double validator to the line edit
    ///(this ensures that the line edit contains only double values).
    Q_INVOKABLE void addDoubleValidator(double bottom, double top, int decimals){addValidator(new QDoubleValidator(bottom, top, decimals));}

    ///Adds an regular expression validator to the line edit
    ///(this ensures that the line edit contains only the allowed values which are specified in the pattern).
    Q_INVOKABLE void addRexpExValidator(QString pattern, bool caseSensitiv)
    {addValidator(new QRegularExpressionValidator(QRegularExpression(pattern, caseSensitiv ? QRegularExpression::NoPatternOption : QRegularExpression::CaseInsensitiveOption)));}

    ///This slot function sets the text of the line edit.
    Q_INVOKABLE void setText(QString text){emit setTextSignal(text);}

    ///This slot functions clears the line edit.
    Q_INVOKABLE void clear(void){m_lineEdit->clear();}

    ///Activates the hexadecimal mode (only hexedecimal values are allowed).
    Q_INVOKABLE void setToHexMode(quint64 maxValue)
    {
        m_hexModeActivated = true;
        m_maxValue = maxValue;

        connect(m_lineEdit, SIGNAL(textChanged(QString&)),this, SLOT(textChangedInHexMode(QString&)));
    };

public slots:

    void textChangedInHexMode(QString newText)
    {

        QString tmpText1;
        QString tmpText2;
        QString savedNewText = newText;

        //Allow only hexedecimal characters.
        newText.replace(QRegularExpression("[^xa-fA-F\\d\\s]"), "");

        if(newText.startsWith("x"))
        {//The current value starts with x.

            newText = "0" + newText;
        }
        else if(newText == "0")
        {
            newText = "0x";
        }
        else if(!newText.startsWith("0x"))
        {//The current value does not start with 0x.

            newText = "0x" + newText;
        }

        //Remove all 0 after 0x (except the string is 0x0).
        bool replaced = false;
        do
        {
            replaced = false;
            if(newText.startsWith("0x0") && (newText != "0x0"))
            {
                newText.replace("0x0", "0x");
                replaced = true;
            }
        }while(replaced);



        if(newText.length() > 2)
        {
            //Remove all x that are not the second character (0x).
            tmpText1 = newText.right(newText.length() - 2);
            tmpText1.replace("x", "");

            tmpText2 = newText.left(2);
            newText = tmpText2 + tmpText1;

        }

        if(text().toULongLong(nullptr, 16) > m_maxValue)
        {
            newText = m_savedText;
        }

        if(savedNewText != newText)
        {//The text was modified.

            setText(newText);
        }

        m_savedText = newText;
    }

Q_SIGNALS:
    ///This signal is emitted if the text of the line edit has been changed.
    ///Scripts can connect a function to this signal.
    void textChangedSignal(QString currentText);

    ///This signal is emitted if the text of the wrapped line edit shall be changed.
    ///This signal is private and must not be used inside a script.
    void setTextSignal(QString currentText);

    ///This signal is emitted in setReadOnly.
    ///This signal is private and must not be used inside a script.
    void setReadOnlySignal(bool readOnly, QLineEdit* lineEdit);

    ///This signal is emitted in clear.
    ///This signal is private and must not be used inside a script.
    void clearSignal(void);

    ///This signal is used to add a validator the line edit.
    ///This signal is private and must not be used inside a script.
    void addValidatorSignal(QValidator* validator, QLineEdit* lineEdit);

private:

    ///Adds a validator to the line edit.
    void addValidator(QValidator* validator)
    {
        validator->moveToThread(QApplication::instance()->thread());
        emit addValidatorSignal(validator, m_lineEdit);
    }

    ///The wrapped line edit.
    QLineEdit* m_lineEdit;

    ///True if the hex mode is activated.
    bool m_hexModeActivated;

    ///the max. value in hex mode.
    quint64 m_maxValue;

    ///The save text in hex mode.
    QString m_savedText;
};

#endif // SCRIPTLINEEDIT_H

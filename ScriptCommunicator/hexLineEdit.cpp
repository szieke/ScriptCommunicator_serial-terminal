#include "hexLineEdit.h"

#include <QRegularExpression>



/**
 * Is called if the current value has been changed.
 *
 * @param newText The new text/value.
 */
void HexLineEdit::textChangedSlot(QString newText)
{
    QString tmpText1;
    QString tmpText2;

    blockSignals(true);

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


    bool isOk;
    quint32 value = newText.toULongLong(&isOk, 16);
    if(value > m_max)
    {
        newText = "0x" + QString::number(m_max, 16);
    }



    setText(newText);

    blockSignals(false);
}

/**
 * Returns the current value.
 *
 * @return The current value.
 */
quint32 HexLineEdit::getValue(void)
{
    bool isOk;
    return text().toULong(&isOk, 16);
}

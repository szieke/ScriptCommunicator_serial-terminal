#include "scriptStandardDialogs.h"
#include "scriptwindow.h"
#include "scriptThread.h"



/**
 * Constructor.
 * @param parent
 *      The parent.
 */
ScriptStandardDialogs::ScriptStandardDialogs(QObject *parent) :
    QObject(parent)
{
}


/**
 * Connects all signals.
 * @param scriptWindow
 *      Pointer to the script window.
 * @param runsInDebugger
 *      True if the script runs in the script debugger.
 */
void ScriptStandardDialogs::intSignals(ScriptWindow *scriptWindow, bool runsInDebugger)
{
    Qt::ConnectionType directConnectionType = runsInDebugger ? Qt::DirectConnection : Qt::BlockingQueuedConnection;

    connect(this, SIGNAL(showTextInputDialogSignal(QString,QString,QString,QString*, QWidget*)),
            scriptWindow, SLOT(showTextInputDialogSlot(QString,QString,QString,QString*, QWidget*)), directConnectionType);

    connect(this, SIGNAL(showMultiLineTextInputDialogSignal(QString,QString,QString,QString*,QWidget*)),
            scriptWindow, SLOT(showMultiLineTextInputDialogSlot(QString,QString,QString,QString*,QWidget*)), directConnectionType);

    connect(this, SIGNAL(showGetItemDialogSignal(QString,QString,QStringList,int,bool,QString*,QWidget*)),
            scriptWindow, SLOT(showGetItemDialogSlot(QString,QString,QStringList,int,bool,QString*,QWidget*)), directConnectionType);

    connect(this, SIGNAL(showGetIntDialogSignal(QString,QString,int,int,int,int,int*,bool*,QWidget*)),
            scriptWindow, SLOT(showGetIntDialogSlot(QString,QString,int,int,int,int,int*,bool*,QWidget*)), directConnectionType);

    connect(this, SIGNAL(showGetDoubleDialogSignal(QString,QString,double,double,double,int,double*,bool*,QWidget*)),
            scriptWindow, SLOT(showGetDoubleDialogSlot(QString,QString,double,double,double,int,double*,bool*,QWidget*)), directConnectionType);

    connect(this, SIGNAL(showOpenFileNamesDialogSignal(QString,QString,QString,QStringList*,QWidget*)),
            scriptWindow, SLOT(showOpenFileNamesDialogSlot(QString,QString,QString,QStringList*,QWidget*)), directConnectionType);

    connect(this, SIGNAL(showFileDialogSignal(bool,QString,QString,QString,QString*, QWidget*)),
            scriptWindow, SLOT(showFileDialogSlot(bool,QString,QString,QString,QString*, QWidget*)), directConnectionType);

    connect(this, SIGNAL(showDirectoryDialogSignal(QString,QString,QString*,QWidget*)),
            scriptWindow, SLOT(showDirectoryDialogSlot(QString,QString,QString*,QWidget*)), directConnectionType);

    connect(this, SIGNAL(showColorDialogSignal(quint8,quint8,quint8,quint8,bool,QWidget*,QList<int>*)),
            scriptWindow, SLOT(showColorDialogSlot(quint8,quint8,quint8,quint8,bool,QWidget*,QList<int>*)), directConnectionType);

    connect(this, SIGNAL(showYesNoDialogSignal(QMessageBox::Icon,QString,QString,QWidget*,bool*)),
            scriptWindow->getMainWindow(), SLOT(showYesNoDialogSlot(QMessageBox::Icon,QString,QString,QWidget*,bool*)),
            directConnectionType);

    connect(this, SIGNAL(showMessageBoxSignal(QMessageBox::Icon, QString, QString, QMessageBox::StandardButtons, QWidget* )),
            scriptWindow->getMainWindow(), SLOT(showMessageBoxSlot(QMessageBox::Icon, QString, QString, QMessageBox::StandardButtons, QWidget*)),
            directConnectionType);

    connect(this, SIGNAL(disableMouseEventsSignal()),
            scriptWindow->getMainWindow(), SLOT(disableMouseEventsSlot()), directConnectionType);

    connect(this, SIGNAL(enableMouseEventsSignal()),
            scriptWindow->getMainWindow(), SLOT(enableMouseEventsSlot()), directConnectionType);
}

/**
 * Wrapper for QFileDialog::getSaveFileName and QFileDialog::getOpenFileName.
 * @param isSaveDialog
 *      True for a QFileDialog::getSaveFileName and false for a QFileDialog::getOpenFileName dialog.
 * @param caption
 *      The caption of the dialog.
 * @param dir
 *      The initial dir for showing the dialog.
 * @param filter
 *      Filter for the file dialog.
 * @param parent
 *      The parent of the dialog.
 * @return
 *      The path of the selected file
 */
QString ScriptStandardDialogs::showFileDialog(bool isSaveDialog, QString caption, QString dir, QString filter, QWidget* parent)
{
    QString resultFileName;
    emit disableMouseEventsSignal();
    emit enableMouseEventsSignal();
    emit showFileDialogSignal(isSaveDialog, caption, dir, filter, &resultFileName, parent);
    return resultFileName;
}

/**
* Wrapper for QFileDialog::getOpenFileNames.
* @param isSaveDialog
*      True for a QFileDialog::getSaveFileName and false for a QFileDialog::getOpenFileName dialog.
* @param caption
*      The caption of the dialog.
* @param dir
*      The initial dir for showing the dialog.
* @param filter
*      Filter for the file dialog.
* @param parent
*      The parent of the dialog.
* @return
*      The paths of the selected files
*/
QStringList ScriptStandardDialogs::showOpenFileNamesDialog(QString caption, QString dir, QString filter, QWidget* parent)
{
    QStringList resultFileNames;
    emit disableMouseEventsSignal();
    emit enableMouseEventsSignal();
    emit showOpenFileNamesDialogSignal(caption, dir, filter, &resultFileNames, parent);
    return resultFileNames;
}

/**
 * Wrapper for QFileDialog::getExistingDirectory.
 * @param caption
 *      The caption of the dialog.
 * @param dir
 *      The initial dir for showing the dialog.
 * @param parent
 *      The parent of the dialog.
 * @return
 *      The selected directory.
 */
QString ScriptStandardDialogs::showDirectoryDialog(QString caption, QString dir, QWidget* parent)
{
    QString resultDirectoryName;
    emit disableMouseEventsSignal();
    emit enableMouseEventsSignal();
    emit showDirectoryDialogSignal(caption, dir, &resultDirectoryName, parent);
    return resultDirectoryName;
}

/**
 * Shows a message box.
 * (emits the ScriptStandardDialogs::showMessageBoxSignal signal).
 * @param icon
 *      The icon if the message box. Possible values are:
 *      - Information
 *      - Warning
 *      - Critical
 *      - Question
 * @param title
 *      The title of the message box.
 * @param text
 *      The text of the message box.
 * @param parent
 *      The parent of the dialog.
 */
void ScriptStandardDialogs::messageBox(QString icon, QString title, QString text, QWidget* parent)
{
    emit disableMouseEventsSignal();
    emit enableMouseEventsSignal();
    emit showMessageBoxSignal(ScriptThread::stringToMessageBoxIcon(icon), title, text, QMessageBox::Ok, parent);
}

/**
 * Shows a yes/no dialog.
 * @param icon
 *      The icon if the message box. Possible values are:
 *      - Information
 *      - Warning
 *      - Critical
 *      - Question
 * @param title
 *      The title of the message box.
 * @param text
 *      The text of the message box.
 * @param parent
 *      The parent of the dialog.
 * @return
 *      True if the yes button has been pressed.
 */
bool ScriptStandardDialogs::showYesNoDialog(QString icon, QString title, QString text, QWidget* parent)
{
    bool yesButtonPressed = false;

    emit disableMouseEventsSignal();
    emit enableMouseEventsSignal();
    emit showYesNoDialogSignal(ScriptThread::stringToMessageBoxIcon(icon), title, text, parent,&yesButtonPressed);

    return yesButtonPressed;
}

/**
 * Convenience function to get a string from the user.
 * Shows a QInputDialog::getText dialog (line edit).
 * @param title
 *      The title of the dialog.
 * @param label
 *      The label over the input section.
 * @param displayedText
 *      The display text in the input section
 * @return
 *      The text in the input section after closing the dialog (empty if the ok button was not pressed).
 */
QString ScriptStandardDialogs::showTextInputDialog(QString title, QString label, QString displayedText, QWidget* parent)
{
    QString result;
    emit showTextInputDialogSignal(title, label, displayedText, &result, parent);
    return result;
}


/**
 * Convenience function to get a multiline string from the user.
 * Shows a QInputDialog::getMultiLineText dialog (plain text edit).
 * @param title
 *      The title of the dialog.
 * @param label
 *      The label over the input section.
 * @param displayedText
 *      The display text in the input section
 * @param parent
 *      The parent of the dialog.
 * @return
 *      The text in the input section after closing the dialog (empty if the ok button was not pressed).
 */
QString ScriptStandardDialogs::showMultiLineTextInputDialog(QString title, QString label, QString displayedText, QWidget* parent)
{
    QString result;

    emit disableMouseEventsSignal();
    emit enableMouseEventsSignal();
    emit showMultiLineTextInputDialogSignal(title,label,displayedText,&result, parent);

    return result;
}

/**
 * Convenience function to let the user select an item from a string list.
 * Shows a QInputDialog::getItem dialog (combobox).
 * @param title
 *      The title of the dialog.
 * @param label
 *      The label over the combobox.
 * @param displayedItems
 *      The displayed items.
 * @param currentItemIndex
 *      The current combobox index.
 * @param editable
 *      True if the combobox shall be editable.
 * @param parent
 *      The parent of the dialog.
 * @return
 *      The text of the selected item after closing the dialog (empty if the ok button was not pressed).
 */
QString ScriptStandardDialogs::showGetItemDialog(QString title, QString label, QStringList displayedItems,
                           int currentItemIndex, bool editable, QWidget* parent)
{
    QString result;

    emit disableMouseEventsSignal();
    emit enableMouseEventsSignal();
    emit showGetItemDialogSignal(title,label,displayedItems,currentItemIndex,editable,&result, parent);

    return result;
}

/**
 * Convenience function to get an integer input from the user.
 * Shows a QInputDialog::getInt dialog (spinbox).
 * @param title
 *      The title of the dialog.
 * @param label
 *      The label over the input section.
 * @param initialValue
 *      The initial value.
 * @param min
 *      The minimum value.
 * @param max
 *      The maximum value.
 * @param step
 *      The amount by which the values change as the user presses the arrow buttons to
 *      increment or decrement the value.
 * @param parent
 *      The parent of the dialog.
 * @result
 *      item 0: 1 if the ok button has been pressed, 0 otherwise
 *      item 1: The value of the spinbox after closing the dialog.
 */
QList<int> ScriptStandardDialogs::showGetIntDialog(QString title, QString label, int initialValue, int min, int max, int step, QWidget* parent)
{
    QList<int> result;
    int inputValue;
    bool okPressed;

    emit disableMouseEventsSignal();
    emit enableMouseEventsSignal();
    emit showGetIntDialogSignal(title,label,initialValue,min,max,step,&inputValue, &okPressed, parent);

    if(okPressed){result.push_back(1);}
    else{result.push_back(0);}

    result.push_back(inputValue);
    return result;
}

/**
 * Convenience function to get a floating point number from the user.
 * Shows a QInputDialog::getDouble dialog (spinbox).
 *
 * @param title
 *      The title of the dialog.
 * @param label
 *      The label over the input section.
 * @param initialValue
 *      The initial value.
 * @param min
 *      The minimum value.
 * @param max
 *      The maximum value.
 * @param decimals
 *      The maximum number of decimal places the number may have.
 * @param parent
 *      The parent of the dialog.
 * @result
 *      item 0: 1.0 if the ok button has been pressed, 0 otherwise
 *      item 1: The value of the spinbox after closing the dialog.
 */
QList<double> ScriptStandardDialogs::showGetDoubleDialog(QString title, QString label, double initialValue, double min, double max, int decimals, QWidget* parent)
{
    QList<double> result;
    double inputValue;
    bool okPressed;

    emit disableMouseEventsSignal();
    emit enableMouseEventsSignal();
    emit showGetDoubleDialogSignal(title, label, initialValue, min, max, decimals, &inputValue, &okPressed, parent);

    if(okPressed){result.push_back(1.0);}
    else{result.push_back(0.0);}

    result.push_back(inputValue);

    return result;
}
/**
 * Convenience function to get color settings from the user.
 * Shows a color_widgets::ColorDialog dialog.
 * @param initInitalRed
 *      The inital value for red.
 * @param initInitalGreen
 *      The inital value for green.
 * @param initInitalBlue
 *      The inital value for blue.
 * @param initInitalAlpha
 *      The inital value for the alpha value.
 * @param alphaIsEnabled
 *      True if the color alpha channel should be editedable.
 *      If alpha is disabled, the selected color's alpha will always be 255.
 * @return
 *      The list contains following:
 *      - 1 if the user has pressed the OK button, otherwise 0
 *      - the selected red value
 *      - the selected green value
 *      - the selected blue value
 *      - the selected alpha value
 */
QList<int> ScriptStandardDialogs::showColorDialog(quint8 initInitalRed, quint8 initInitalGreen, quint8 initInitalBlue, quint8 initInitalAlpha, bool alphaIsEnabled, QWidget* parent)
{
    QList<int> result;
    emit showColorDialogSignal(initInitalRed, initInitalGreen, initInitalBlue, initInitalAlpha, alphaIsEnabled, parent, &result);
    return result;

}

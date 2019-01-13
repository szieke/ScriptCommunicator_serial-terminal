
#include "scriptSlots.h"

#include<QTimer>
#include <QMenu>
#include <QFileDialog>
#include <QBuffer>
#include<QDomDocument>
#include "plotwindow.h"
#include "scriptComboBox.h"
#include "scriptLineEdit.h"
#include "mainwindow.h"
#include "scriptTableWidget.h"
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
#include "scriptwindow.h"
#include "colorWidgets/color_dialog.hpp"
#include "qcontext2dcanvas.h"


QWidget* createWidget(QString& type, QWidget* parent, bool insertedInTableWidget)
{
    QWidget* widget = NULL;

    if(type == "ComboBox")
    {
        QComboBox* el = new QComboBox(parent);
        if(insertedInTableWidget){el->setFrame(false);}
        widget = el;
    }
    else if(type == "LineEdit")
    {
        QLineEdit* el = new QLineEdit(parent);
        if(insertedInTableWidget){el->setFrame(false);}
        widget = el;
    }
    else if(type == "Button")
    {
        QPushButton* el = new QPushButton(parent);
        widget = el;
    }
    else if(type == "CheckBox")
    {
        QCheckBox* el = new QCheckBox(parent);
        widget = el;
    }
    else if(type == "SpinBox")
    {
        QSpinBox* el = new QSpinBox(parent);
        if(insertedInTableWidget){el->setFrame(false);}
        widget = el;

    }
    else if(type == "DoubleSpinBox")
    {
        QDoubleSpinBox* el = new QDoubleSpinBox(parent);
        if(insertedInTableWidget){el->setFrame(false);}
        widget = el;

    }
    else if(type == "VerticalSlider")
    {
        QSlider* el = new QSlider(parent);
        el->setOrientation(Qt::Vertical);
        widget = el;

    }
    else if(type == "HorizontalSlider")
    {
        QSlider* el = new QSlider(parent);
        el->setOrientation(Qt::Horizontal);
        widget = el;
    }
    else if(type == "TimeEdit")
    {
        QTimeEdit* el = new QTimeEdit(parent);
        if(insertedInTableWidget){el->setFrame(false);}
        widget = el;

    }
    else if(type == "DateEdit")
    {
        QDateEdit* el = new QDateEdit(parent);
        if(insertedInTableWidget){el->setFrame(false);}
        widget = el;
    }
    else if(type == "DateTimeEdit")
    {
        QDateTimeEdit* el = new QDateTimeEdit(parent);
        if(insertedInTableWidget){el->setFrame(false);}
        widget = el;
    }
    else if(type == "TextEdit")
    {
        QTextEdit* el = new QTextEdit(parent);
        widget = el;

    }
    else if(type == "Dial")
    {
        QDial* el = new QDial(parent);
        widget = el;

    }
    else if(type == "CalendarWidget")
    {
        QCalendarWidget* el = new QCalendarWidget(parent);
        widget = el;

    }

    return widget;
}

/**
 * Creates and inserts a script widget into a table cell.
 * @param scriptThread
 *      The script thread.
 * @param table
 *      The table widget.
 * @param row
 *      The row of the cell.
 * @param column
 *      The column of the cell..
 * @param type
 *      The script widget type.
 * @param succeeded
 *      True on success.
 */
void ScriptSlots::insertWidgetInToTableSlot(ScriptThread* scriptThread, QTableWidget* table, int row, int column, QString type, bool* succeeded)
{
    *succeeded = false;
    (void)scriptThread;

    if(table->rowCount() <= row)table->setRowCount(row + 1);
    if(table->columnCount() <= column)table->setColumnCount(column + 1);

    QTableWidgetItem *item = table->item(row, column);

    if(item == 0)
    {
        item = new QTableWidgetItem();
        table->setItem(row, column, item);
    }
    if(item)
    {
        QWidget* widget = createWidget(type, table, true);
        if(widget)
        {
            table->setCellWidget(row, column, widget);
            *succeeded = true;
        }
        else
        {
            *succeeded = false;
        }
    }

}

/**
 * With this slot function a script thread can change the background color
 * of a script gui element.
 * Note: Gui elements are created in the main thread.
 * @param color
 *      The new background color.
 * @param element
 *      The script gui element.
 */
void ScriptSlots::setScriptGuiElementBackgroundColorSlot(QColor color, QWidget* element)
{
    QPalette palette(element->palette());
    palette.setColor(element->backgroundRole(), color);
    element->setPalette(palette);
}

/**
 * With this slot function a script thread can change the color
 * of a script gui element.
 * Note: Gui elements are created in the main thread.
 * @param color
 *      The new background color.
 * @param element
 *      The script gui element.
 * @param colorRole
 *      The new color role.
 */
void ScriptSlots::setScriptGuiElementColorSlot(QColor color, QWidget* element, QPalette::ColorRole colorRole)

{
    QPalette palette(element->palette());
    palette.setColor(colorRole, color);
    element->setPalette(palette);
}


/**
 * Sets the auto fill background property of the gui element.
 *
 * @param element
 *      The script gui element.
 * @param enabled
 *      True enabled, false disabled.
 */
void ScriptSlots::setScriptGuiElementAutoFillBackgroundSlot(QWidget* element, bool enabled)
{
    element->setAutoFillBackground(enabled);
}

/**
 * Selects a cell in a table widget.
 *
 * @param tableWidget
 *      The table widget.
 * @param row
 *      The row of the cell.
 * @param column
 *      The column of the cell.
 */
void ScriptSlots::selectCellSlot(QTableWidget* tableWidget, int row, int column, bool scrollToCell)
{

    QModelIndex index = tableWidget->model()->index(row,column);
    if(index.isValid())
    {
        tableWidget->selectionModel()->clearSelection();
        tableWidget->selectionModel()->select(index , QItemSelectionModel::Select);
        tableWidget->setFocus();

        if(scrollToCell)
        {
            tableWidget->scrollTo(index);
        }
    }
}

/**
 * Converts a string to Qt::GlobalColor.
 * @param color
 *  The color string. Possible values are:
 *  black, white, gray, red, green, blue, cyan, magenta and yellow.
 *
 * @return
 *  The converted color.
 */
Qt::GlobalColor ScriptSlots::stringToGlobalColor(QString color)
{
    Qt::GlobalColor convertedColor = Qt::white;


    if(color == "black")
    {
        convertedColor = Qt::black;
    }
    else if(color == "white")
    {
        convertedColor = Qt::white;
    }
    else if(color == "darkGray")
    {
        convertedColor = Qt::darkGray;
    }
    else if(color == "gray")
    {
        convertedColor = Qt::gray;
    }
    else if(color == "lightGray")
    {
        convertedColor = Qt::lightGray;
    }
    else if(color == "red")
    {
        convertedColor = Qt::red;
    }
    else if(color == "green")
    {
        convertedColor = Qt::green;
    }
    else if(color == "blue")
    {
        convertedColor = Qt::blue;
    }
    else if(color == "cyan")
    {
        convertedColor = Qt::cyan;
    }
    else if(color == "magenta")
    {
        convertedColor = Qt::magenta;
    }
    else if(color == "yellow")
    {
        convertedColor = Qt::yellow;
    }
    else if(color == "darkRed")
    {
        convertedColor = Qt::darkRed;
    }
    else if(color == "darkGreen")
    {
        convertedColor = Qt::darkGreen;
    }
    else if(color == "darkBlue")
    {
        convertedColor = Qt::darkBlue;
    }
    else if(color == "darkCyan")
    {
        convertedColor = Qt::darkCyan;
    }
    else if(color == "darkMagenta")
    {
        convertedColor = Qt::darkMagenta;
    }
    else if(color == "darkYellow")
    {
        convertedColor = Qt::darkYellow;
    }
    else
    {
        convertedColor = Qt::white;
    }

    return convertedColor;
}



/**
 * Converts a string to a QPalette::ColorRole.
 * @param colorRole
 *      The color role.
 * @return
 *      The converted color role.
 */
QPalette::ColorRole ScriptSlots::stringToPaletteColorRole(QString colorRole)
{
    QPalette::ColorRole convertedColor;

    if(colorRole == "Base")
    {
        convertedColor = QPalette::Base;
    }
    else if(colorRole == "Foreground")
    {
        convertedColor = QPalette::Foreground;
    }
    else if(colorRole == "Background")
    {
        convertedColor = QPalette::Background;
    }
    else if(colorRole == "WindowText")
    {
        convertedColor = QPalette::WindowText;
    }
    else if(colorRole == "Button")
    {
        convertedColor = QPalette::Button;
    }
    else if(colorRole == "Window")
    {
        convertedColor = QPalette::Window;
    }
    else if(colorRole == "Text")
    {
        convertedColor = QPalette::Text;
    }
    else if(colorRole == "ButtonText")
    {
        convertedColor = QPalette::ButtonText;
    }
    else
    {
        convertedColor = QPalette::NoRole;
    }

    return convertedColor;

}

/**
 * Sets the tool tip text and the tool tip duration of a gui element.
 * @param text
 *      The tool tip text.
 * @param duration
 *      The tool tip duration. If the value is -1 (default)
 *      the duration is calculated depending on the length of the tool tip.
 * @param element
 *      The gui element
 */
void ScriptSlots::setToolTipSlot(QString text, int duration, QWidget* element)
{
    element->setToolTip(text);
    element->setToolTipDuration(duration);
}

/**
 * Returns the window size and position
 * The return string has following format:
 * "top left x, top left y, width, height"
 *
 * @param string
 *      The return string.
 * @param widget
 *      The window.
 */
void ScriptSlots::windowPositionAndSizeSlot(QString *string, QWidget* widget)
{
    QRect rect= MainWindow::windowPositionAndSize(widget);

    *string = QString("%1,%2,%3,%4")
            .arg(rect.topLeft().x())
            .arg(rect.topLeft().y())
            .arg(rect.size().width())
            .arg(rect.size().height());
}



/**
 * Sets the position and the size of a window (from script).
 * @param positionAndSize
 *      The position and size string ("top left x, top left y, width, height").
 * @param widget
 *      The window.
 */
void ScriptSlots::setWindowPositionAndSizeSlot(QString positionAndSize, QWidget* widget)
{
    QStringList list = positionAndSize.split(",");
    QRect rect(QPoint(list[0].toInt(), list[1].toInt()), QSize(list[2].toInt(), list[3].toInt()));

    MainWindow::setWindowPositionAndSize(widget, rect);
}

/**
 * Removes a row.
 * @param row
 *      The row.
 * @param list
 *      The list widget.
 */
void ScriptSlots::removeItem(int row, QListWidget* list)
{
    QListWidgetItem* item =  list->takeItem(row);
    if(item)
    {
        delete item;
    }

}

/**
 * Limits the number of chars in the given text edit to the value of maxChars.
 * @param textEdit
 *      The text edit.
 * @param maxChars
 *      The max. number of chars.
 */
void ScriptSlots::limtCharsInTextEditSlot (QTextEdit *textEdit, const int maxChars)
{
    MainWindow::limtCharsInTextEdit(textEdit, maxChars);
    textEdit->moveCursor(QTextCursor::End);
}

/**
 * This slot function moves the curser to the end of a console.
 * @param textEdit
 *      The console.
 */
void ScriptSlots::moveTextPositionToEndSlot(QTextEdit* textEdit)
{
    textEdit->moveCursor(QTextCursor::End);
    textEdit->horizontalScrollBar()->setSliderPosition(0);
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
 * @param result
 *      The text in the input section after closing the dialog (empty if the ok button was not pressed).
 * @param parent
 *      The parent of the dialog.
 */
void ScriptSlots::showTextInputDialogSlot(QString title, QString label, QString displayedText, QString* result, QWidget* parent)
{
    bool okPressed;
    *result = QInputDialog::getText(parent, title, label, QLineEdit::Normal, displayedText,
                                         &okPressed);
    if(!okPressed)
    {
        *result = "";
    }
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
 * @param result
 *      The text in the input section after closing the dialog (empty if the ok button was not pressed).
 * @param parent
 *      The parent of the dialog.
 */
void ScriptSlots::showMultiLineTextInputDialogSlot(QString title, QString label, QString displayedText, QString* result, QWidget* parent)
{
    bool okPressed;
    *result = QInputDialog::getMultiLineText(parent, title, label, displayedText, &okPressed);
    if(!okPressed)
    {
        *result = "";
    }
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
 * @param result
 *      The text of the selected item after closing the dialog (empty if the ok button was not pressed).
 * @param parent
 *      The parent of the dialog.
 */
void ScriptSlots::showGetItemDialogSlot(QString title, QString label, QStringList displayedItems,
                                         int currentItemIndex, bool editable, QString* result, QWidget* parent)
{
    bool okPressed;
    *result = QInputDialog::getItem(parent, title, label, displayedItems, currentItemIndex, editable, &okPressed);
    if(!okPressed)
    {
        *result = "";
    }
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
 * @param result
 *      The value of the spinbox after closing the dialog.
 * @param okPressed
 *      True if the user has pressed the ok button.
 * @param parent
 *      The parent of the dialog.
 */
void ScriptSlots::showGetIntDialogSlot(QString title, QString label, int initialValue, int min, int max, int step,
                                        int* result, bool* okPressed, QWidget* parent)
{
    *result = QInputDialog::getInt(parent, title, label, initialValue,min, max, step, okPressed);
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
 * @param result
 *      The value in the input section after closing the dialog.
 * @param okPressed
 *      True if the user has pressed the ok button.
 * @param parent
 *      The parent of the dialog.
 */
void ScriptSlots::showGetDoubleDialogSlot(QString title, QString label, double initialValue, double min, double max,
                                           int decimals, double* result, bool* okPressed, QWidget* parent)
{
    *result = QInputDialog::getDouble(parent, title, label, initialValue, min, max, decimals, okPressed);
}

/**
 * This slot function shows a file dialog for selecting one file (QFileDialog::getSaveFileName or QFileDialog::getOpenFileName).
 * @param isSaveDialog
 *      True for QFileDialog::getSaveFileName and false for QFileDialog::getOpenFileName dialog.
 * @param caption
 *      The caption of the dialog.
 * @param dir
 *      The initial directory of the dialog.
 * @param filter
 *      The dialog filter.
 * @param resultFileName
 *      The path of the selected file.
 * @param parent
 *      The parent of the file dialog.
 */
void ScriptSlots::showFileDialogSlot(bool isSaveDialog, QString caption, QString dir, QString filter, QString *resultFileName, QWidget* parent)
{
    *resultFileName = isSaveDialog ? QFileDialog::getSaveFileName(parent, caption,dir,filter) : QFileDialog::getOpenFileName(parent, caption,dir,filter);
}

/**
 * This slot function shows a file dialog for selecting one or more existing files (QFileDialog::getOpenFileNames).
 * @param caption
 *      The caption of the dialog.
 * @param dir
 *      The initial directory of the dialog.
 * @param filter
 *      The dialog filter.
 * @param resultFileNames
 *      The paths of the selected files.
 * @param parent
 *      The parent of the file dialog.
 */
void ScriptSlots::showOpenFileNamesDialogSlot(QString caption, QString dir, QString filter, QStringList *resultFileNames, QWidget* parent)
{
    *resultFileNames = QFileDialog::getOpenFileNames(parent, caption,dir,filter);
}

/**
 * This slot function shows a QFileDialog::getExistingDirectory.
 * @param caption
 *      The caption of the dialog.
 * @param dir
 *      The initial dir for showing the dialog.
 * @param directoryName
 *      The selected directory.
 * @param parent
 *      The parent of the dialog.
 */
void ScriptSlots::showDirectoryDialogSlot(QString caption, QString dir, QString *directoryName, QWidget* parent)
{
    *directoryName = QFileDialog::getExistingDirectory(parent, caption, dir);
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
 * @param parent
 *      The parent widget.
 * @param result
 *      The list contains following:
 *      - 1 if the user has pressed the OK button, otherwise 0
 *      - the selected red value
 *      - the selected green value
 *      - the selected blue value
 *      - the selected alpha value
 */
void ScriptSlots::showColorDialogSlot(quint8 initInitalRed, quint8 initInitalGreen, quint8 initInitalBlue, quint8 initInitalAlpha,
                                 bool alphaIsEnabled, QWidget* parent, QList<int>* result)
{
    color_widgets::ColorDialog* dialog = new color_widgets::ColorDialog(parent);
    dialog->setButtonMode(color_widgets::ColorDialog::OkCancel);
    dialog->setColor(QColor(initInitalRed, initInitalGreen, initInitalBlue, alphaIsEnabled ? initInitalAlpha : 255));
    dialog->setAlphaEnabled(alphaIsEnabled);

    int res = dialog->exec();
    result->append(res);

    QColor color = dialog->color();
    result->append(color.red());
    result->append(color.green());
    result->append(color.blue());
    result->append(color.alpha());
}

/**
 * With this function the user can move the selected row up or down
 * (while holding the left mouse button at the row and moving the mouse up and/or down).
 * @param row
 *      The row.
 * @param rowsel
 *      The second row.
 * @param column
 *      The column.
 * @param tableWidget
 *      The table widget.
 */
void ScriptSlots::cellEnteredSlot(int row, int rowsel, int column, QTableWidget* tableWidget)
{
    (void) column;
    int colCount = tableWidget->columnCount();

    bool verticalScrollBarPressed = false;
    if(tableWidget->verticalScrollBar())
    {
        verticalScrollBarPressed = tableWidget->verticalScrollBar()->underMouse();
    }

    bool horizontalScrollBarPressed = false;
    if(tableWidget->horizontalScrollBar())
    {
        horizontalScrollBarPressed = tableWidget->horizontalScrollBar()->underMouse();
    }

    if((QApplication::mouseButtons() != Qt::NoButton) &&
        !verticalScrollBarPressed && !horizontalScrollBarPressed)
    {
        if(tableWidget->verticalHeaderItem(row) == 0)
        {
            QTableWidgetItem *headerItem = new QTableWidgetItem(QString("%1").arg(rowsel + 1));
            tableWidget->setVerticalHeaderItem(row, headerItem);
        }

        if(tableWidget->verticalHeaderItem(rowsel) == 0)
        {
            QTableWidgetItem *headerItem = new QTableWidgetItem(QString("%1").arg(rowsel + 1));
            tableWidget->setVerticalHeaderItem(rowsel, headerItem);
        }

        QList<QTableWidgetItem*> rowItems,rowItems1;
        tableWidget->blockSignals(true);

        QString savedHeaderText;
        if(tableWidget->verticalHeader()->isVisible())
        {
            savedHeaderText = tableWidget->verticalHeaderItem(row)->text();
        }

        //remove all cells from the two rows which position have to be swapped
        for (int col = 0; col < colCount; ++col)
        {
            rowItems << tableWidget->takeItem(row, col);
            rowItems1 << tableWidget->takeItem(rowsel, col);

        }

        //insert all cells from the two rows which positions have to be swapped
        //at their new positions
        for (int cola = 0; cola < colCount; ++cola)
        {
            tableWidget->setItem(rowsel, cola, rowItems.at(cola));
            tableWidget->setItem(row, cola, rowItems1.at(cola));
        }

        if(tableWidget->verticalHeader()->isVisible())
        {
            //Swap the vertical header labels.
            tableWidget->verticalHeaderItem(row)->setText(tableWidget->verticalHeaderItem(rowsel)->text());
            tableWidget->verticalHeaderItem(rowsel)->setText(savedHeaderText);
        }

        tableWidget->blockSignals(false);


    }//if(QApplication::mouseButtons() != Qt::NoButton)


}
/**
 * With this slot function a script thread can create a gui element (for instance PlotWindow).
 * This slot function must be connect with  Qt::BlockingQueuedConnection to work properly.
 * Note: Gui elements in Qt can only be created in the main thread.
 * @param elementType
 *      The gui element type to create.
 * @param createdGuiElement
 *      The created element.
 * @param scriptWindow
 *      The script window.
 * @param scriptThread
 *      The calling script thread.
 * @param additionalArgument
 *      The additional argument.
 */
void ScriptSlots::createGuiElementSlot(QString elementType, QObject** createdGuiElement, ScriptWindow* scriptWindow,
                                       ScriptThread* scriptThread, QObject *additionalArgument)
{
    *createdGuiElement = 0;

    if(elementType == "ScriptPlotWindow")
    {
        PlotWindow* plotWindow = new PlotWindow(scriptThread, scriptWindow);
        *createdGuiElement = new ScriptPlotWindow(plotWindow, scriptThread, scriptWindow);

        connect(scriptWindow->getMainWindow(), SIGNAL(bringWindowsToFrontSignal()),
                *createdGuiElement, SLOT(bringWindowsToFrontSlot()), Qt::DirectConnection);
    }
    else if(elementType == "ScriptPlotWidget")
    {
        *createdGuiElement = new ScriptPlotWidget(scriptThread, scriptWindow, (QHBoxLayout*) additionalArgument);
        (*createdGuiElement)->setParent((QHBoxLayout*) additionalArgument);
    }
    else if(elementType == "QHBoxLayout")
    {
        *createdGuiElement = new QHBoxLayout((QWidget*) additionalArgument);
    }
    else if(elementType == "QContext2DCanvas")
    {
        QGroupBox* box = (QGroupBox*) additionalArgument;
        QHBoxLayout* layout = new QHBoxLayout(box);
        box->setLayout(layout);
        QContext2DCanvas* canvas2D = new QContext2DCanvas(scriptThread->runsInDebugger());
        layout->addWidget(canvas2D);

        *createdGuiElement = canvas2D;

    }
    else
    {//invalid element type

        *createdGuiElement = 0;
    }
}

/**
 * Loads an user interface file.
 * @param scriptUi
 *      The loaded user interface.
 * @param path
 *      The path to the user interface file.
 */
void ScriptSlots::loadUserInterfaceFileSlot(QWidget** scriptUi, QString path)
{
    *scriptUi = 0;
    QUiLoader loader;
    QFile uiFile(path);

    if(uiFile.open(QIODevice::ReadOnly))
    {
        *scriptUi = loader.load(&uiFile);
        uiFile.close();
    }
}


/**
 * This slot is used to add a validator the line edit.
 * @param validator
 *      The validator.
 * @param lineEdit
 *      The line edit.
 */
void ScriptSlots::addValidatorSignal(QValidator* validator, QLineEdit* lineEdit)
{
    if(lineEdit->validator())
    {
        //Remove the old validator.
        lineEdit->setValidator(0);
    }
    validator->setParent(lineEdit);
    lineEdit->setValidator(validator);
}

/**
 * Process stored operations from a ScriptTextEdit.
 * @param m_storedOperations
 *      The stored  operations.
 */
void ScriptSlots::processStoredOperationsSlot(QTextEdit* textEdit, bool isLocked, quint32 maxChars,
                                              QVector<ScriptTextEditStoredOperations_t>* m_storedOperations)
{
    int pos = 0;

    QTextCursor cursor = textEdit->textCursor();
    textEdit->setUpdatesEnabled(false);
    textEdit->blockSignals(true);
    textEdit->document()->blockSignals(true);

    if(isLocked)
    {
        //Store the scroll bar position.
        pos = textEdit->verticalScrollBar()->value();
    }

    for(auto el : *m_storedOperations)
    {
        if(el.atTheEnd)textEdit->moveCursor(QTextCursor::End);

        if(el.operation == SCRIPT_TEXT_EDIT_OPERATION_CLEAR)
        {
            textEdit->clear();
        }
        else if(el.operation == SCRIPT_TEXT_EDIT_OPERATION_INSERT_PLAIN_TEXT)
        {
            textEdit->insertPlainText(el.data);
        }
        else if(el.operation == SCRIPT_TEXT_EDIT_OPERATION_INSERT_HTML)
        {
            textEdit->insertHtml(el.data);
        }
        else if(el.operation == SCRIPT_TEXT_EDIT_OPERATION_APPEND)
        {
            textEdit->append(el.data);
        }
        else if(el.operation == SCRIPT_TEXT_EDIT_OPERATION_SET_PLAIN_TEXT)
        {
            textEdit->setPlainText(el.data);
        }
        else
        {//SCRIPT_TEXT_EDIT_OPERATION_SET_TEXT

            textEdit->setText(el.data);
        }

        MainWindow::limtCharsInTextEdit(textEdit, maxChars);

        if(el.atTheEnd)textEdit->moveCursor(QTextCursor::End);
    }

    if(isLocked)
    {
        //Restore the scroll bar position.
        textEdit->verticalScrollBar()->setValue(pos);
    }
    else
    {   //Move the scroll bar to the end.
        textEdit->horizontalScrollBar()->setSliderPosition(0);
    }

    textEdit->blockSignals(false);
    textEdit->document()->blockSignals(false);
    textEdit->setUpdatesEnabled(true);
}


///This function inserts one row at row and fills the cells with content.
///Possible colors are: black, white, gray, red, green, blue, cyan, magenta and yellow.
void ScriptSlots::insertRowWithContentSlot(int row, QStringList texts, QStringList backgroundColors, QStringList foregroundColors, QTableWidget* tableWidget)
{
    tableWidget->insertRow(row);

    for(int i = 0; i < tableWidget->columnCount(); i++)
    {
        QTableWidgetItem *item = new QTableWidgetItem();
        tableWidget->setItem(row, i, item);

        if(i < texts.length())
        {
            item->setText(texts[i]);
        }

        if(i < backgroundColors.length())
        {
            item->setBackground(stringToGlobalColor(backgroundColors[i]));
        }

        if(i < foregroundColors.length())
        {
            item->setForeground(stringToGlobalColor(foregroundColors[i]));
        }

    }
}

/**
 * Creates a shortcut.
 * @param keys
 *      The keys for the shortcut.
 * @param parent
 *      The parent widget of the shortcut.
 * @param shortCut
 *      The created shortcut.
 */
void ScriptSlots::createShortCutSlot(QString keys, QWidget *parent, QShortcut **shortCut)
{
    *shortCut = new QShortcut(QKeySequence(keys),parent);

}

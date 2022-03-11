#ifndef SCRIPTSLOTS_H
#define SCRIPTSLOTS_H

#include "scriptThread.h"
#include "mainwindow.h"
#include <QSplitter>
#include <QObject>
#include <QTableWidget>
#include <QMainWindow>
#include <QListWidget>
#include <QTreeWidgetItem>
#include <QAbstractButton>
#include <QDateTimeEdit>
#include <QSpinBox>
#include <QTextEdit>
#include <QAction>
#include <QToolBox>
#include <QCalendarWidget>
#include <QPushButton>
#include <QCheckBox>
#include <QComboBox>
#include <QGroupBox>
#include <QLineEdit>
#include <QListWidgetItem>
#include <QRadioButton>
#include <QIcon>
#include <QShortcut>

///The ScriptTextEdit operations which are stored for later processing.
typedef enum
{
    SCRIPT_TEXT_EDIT_OPERATION_CLEAR,
    SCRIPT_TEXT_EDIT_OPERATION_INSERT_PLAIN_TEXT,
    SCRIPT_TEXT_EDIT_OPERATION_INSERT_HTML,
    SCRIPT_TEXT_EDIT_OPERATION_APPEND,
    SCRIPT_TEXT_EDIT_OPERATION_SET_PLAIN_TEXT,
    SCRIPT_TEXT_EDIT_OPERATION_SET_TEXT

}ScriptTextEditOperation_t;

///Contains a stored ScriptTextEditoperation.
typedef struct
{
    ScriptTextEditOperation_t operation;///The stored operation.
    QString data;///The data of the stored operation.
    bool atTheEnd;///True if the data shall append.

}ScriptTextEditStoredOperations_t;


///This class contains all helper slots which are called by scripts (to execute code which must
///be executed in the main thread).
///ScriptWindow derives form this class.
class ScriptSlots :  public QMainWindow
{
    Q_OBJECT

public slots:

    ///Sets the splitter child widgets respective sizes to the values given in the list.
    void setSplitterSizesSlot(QSplitter* splitter, QList<int>& list){splitter->setSizes(list);}

    ///Creates and inserts a script widget into a table cell.
    void insertWidgetInToTableSlot(ScriptThread *scriptThread, QTableWidget* table, int row, int column, QString type, bool* succeeded);

    ///Sets the icon of a button.
   void setItemIconSlot(QAbstractButton* button, QString iconFileName){button->setIcon(QIcon(iconFileName));}

   ///Clears the list widget
   void clearSlot(QListWidget* list){list->clear();}

   ///Removes a row.
   void removeItem(int row, QListWidget* list);

   ///Sets the icon.
   void setItemIcon(QTreeWidgetItem* item , int column, QString iconFileName){item->setIcon(column, QIcon(iconFileName));}

   ///Sets the icon.
   void setItemIcon(QListWidgetItem* item , QString iconFileName){item->setIcon(QIcon(iconFileName));}

   ///Sets the icon.
   void setItemIconSlot(QTableWidgetItem* item, const QString iconFileName){item->setIcon(QIcon(iconFileName));}

   ///Selects a cell in a table widget.
   void selectCellSlot(QTableWidget* tableWidget, int row, int column,bool scrollToCell);

   ///Scrolls to a row in a table widget.
   void scrollToRowSlot(QTableWidget* tableWidget, int row);

   ///Inserts a new item.
   void insertNewItem (int row, QString itemText, QListWidget* list){list->insertItem(row, itemText);}

   ///Inserts a child.
   void insertChildSlot (int index, QTreeWidgetItem* rootItem, QTreeWidgetItem* child){rootItem->insertChild(index, child);}

   ///Deletes a QTreeWidgetItem object.
   void deleteTreeWidgetItemSlot(QTreeWidgetItem* item){delete item;}

   ///Resizes the column given to the size of its contents.
   void resizeColumnToContentsSlot(int column, QTableWidget* table){table->resizeColumnToContents(column);}

   ///Resizes the row given to the size of its contents.
   void resizeRowToContentsSlot(int row, QTableWidget* table){table->resizeRowToContents(row);}

   ///Sorts the items in the widget in the specified order(true=AscendingOrder,
   ///false=DescendingOrder) by the values in the given column.
   void sortItemsSlot(int column, bool ascendingOrder, QTableWidget* tableWidget){tableWidget->sortItems(column, ascendingOrder ? Qt::AscendingOrder : Qt::DescendingOrder);}

   ///Sets the current item in the tree widget.
   void setCurrentItemSlot(QTreeWidgetItem* item, QTreeWidget* tree){tree->setCurrentItem(item);}

   ///Expands all expandable items.
   void expandAllSlot(QTreeWidget* tree){tree->expandAll();}

   ///Sorts the items in the widget in the specified order(true=AscendingOrder,
   ///false=DescendingOrder) by the values in the given column.
   void sortItemsSlot(int column, bool ascendingOrder,  QTreeWidget* tree){tree->sortItems(column, ascendingOrder ? Qt::AscendingOrder : Qt::DescendingOrder);}

   ///Expands the item. This causes the tree containing the item's children to be expanded.
   void expandItemSlot(QTreeWidgetItem* item, bool expand){item->setExpanded(expand);}

   ///Sets the width of a column.
   void setColumnWidthSlot(int column, int size, QTreeWidget* tree){tree->setColumnWidth(column, size);}

   ///Resizes the column given to the size of its contents.
   void resizeColumnToContentsSlot(int column, QTreeWidget* tree){tree->resizeColumnToContents(column);}

   ///Sets the current selected row.
   void setCurrentRowSlot(int row, QListWidget* list){list->setCurrentRow(row);}

   ///Sorts the items in the widget in the specified order(true=AscendingOrder,
   ///false=DescendingOrder).
   void sortItemsSlot(bool ascendingOrder, QListWidget* listWidget){listWidget->sortItems(ascendingOrder ? Qt::AscendingOrder : Qt::DescendingOrder);}

   ///Sets the window flags.
   void setWindowFlagsSlot(Qt::WindowFlags flags, QWidget* element){element->setWindowFlags(flags);}

   ///Sets the display format of a QDateTimeEdit.
   void setDisplayFormat(QString format, QDateTimeEdit* element){element->setDisplayFormat(format);}

   ///Sets the spin box's minimum and maximum values to minimum and maximum respectively.
   void setRangeSlot(int minimum, int maximum, QSpinBox* spinBox){spinBox->setRange(minimum,maximum);}

   ///Sets the spin box's minimum and maximum values to minimum and maximum respectively.
   void setRangeSlot(double minimum, double maximum, QDoubleSpinBox* spinBox){spinBox->setRange(minimum,maximum);}

   ///Sets the precision of a double spin box, in decimals.
   void setDecimalsSlot(int value, QDoubleSpinBox* spinBox){spinBox->setDecimals(value);}

   ///Sets the text of one tab in a tab widget.
   void setTabWidgetTabTextSlot(int index, QString text, QTabWidget* tabWidget){tabWidget->setTabText(index, text);}

   ///Sets the current tab index.
   void setCurrentIndexSlot(int index, QTabWidget* tabWidget){tabWidget->setCurrentIndex(index);}

   ///Removes a tab and returns the tab id (can be used in insertTab).
   void removeTabSlot(QTabWidget* tabWidget, int index, QWidget** tab, QString* tabText);

   ///Inserts a tab that was removed with removeTab.
   void insertTabSlot(QTabWidget* tabWidget, QWidget* tab, QString tabText, int index);

   ///Sets the text of an action.
   void setActionTextSlot(QString text, QAction* action){action->setText(text);}

   ///Sets the check state of an action.
   void setActionCheckStateSlot(bool checked, QAction* action){action->setChecked(checked);}

   ///Limits the number of chars in the given text edit to the value of maxChars.
   void limtCharsInTextEditSlot (QTextEdit *textEdit, const int maxChars);

   ///This slot function moves the curser to the end of the console.
   void moveTextPositionToEndSlot(QTextEdit* textEdit);

   ///With this slot function a script thread can change the background color
   ///of a script gui element.
   ///Note: Gui elements are created in the main thread.
   void setScriptGuiElementBackgroundColorSlot(QColor color, QWidget* element);

   ///With this slot function a script thread can change the color
   ///of a script gui element.
   ///Note: Gui elements are created in the main thread.
   void setScriptGuiElementColorSlot(QColor color, QWidget* element, QPalette::ColorRole colorRole);

   ///Sets the auto fill background property of the gui element.
   void setScriptGuiElementAutoFillBackgroundSlot(QWidget* element, bool enabled);

   ///Converts a string to Qt::GlobalColor.
   static Qt::GlobalColor stringToGlobalColor(QString color);

   ///Converts a string to a QPalette::ColorRole.
   static QPalette::ColorRole stringToPaletteColorRole(QString colorRole);

   ///Sets the tool tip text and the tool tip duration of a gui element.
   void setToolTipSlot(QString text, int duration, QWidget* element);

   ///Sets the position and the size of a window (from script).
   void setWindowPositionAndSizeSlot(QString positionAndSize, QWidget *widget);

   ///Returns the window size and position
   ///The return string has following format:
   ///"top left x; top left y; width; height"
   void windowPositionAndSizeSlot(QString *string, QWidget* widget);

   ///Convenience function to get a string from the user.
   ///Shows a QInputDialog::getText dialog (plain text edit).
   void showTextInputDialogSlot(QString title, QString label, QString displayedText, QString* result, QWidget *parent);

   ///Convenience function to get a multi line string from the user.
   ///Shows a QInputDialog::getMultiLineText dialog.
   void showMultiLineTextInputDialogSlot(QString title, QString label, QString displayedText, QString* result, QWidget* parent);

   ///Convenience function to let the user select an item from a string list.
   ///Shows a QInputDialog::getItem dialog (combobox).
   void showGetItemDialogSlot(QString title, QString label, QStringList displayedItems,
                              int currentItemIndex, bool editable, QString* result, QWidget* parent);

   ///Convenience function to get an integer input from the user.
   ///Shows a QInputDialog::getInt dialog (spinbox).
   void showGetIntDialogSlot(QString title, QString label, int initialValue, int min, int max, int step,
                                           int* result, bool* okPressed, QWidget* parent);

   ///Convenience function to get a floating point number from the user.
   ///Shows a QInputDialog::getDouble dialog (spinbox).
   void showGetDoubleDialogSlot(QString title, QString label, double initialValue, double min, double max,
                                int decimals, double* result, bool* okPressed, QWidget* parent);

   ///This slot function shows a file dialog for selecting one file (QFileDialog::getSaveFileName or QFileDialog::getOpenFileName).
   void showFileDialogSlot(bool isSaveDialog, QString caption, QString dir, QString filter, QString *resultFileName, QWidget *parent);

   ///This slot function shows a file dialog for selecting one or more existing files (QFileDialog::getOpenFileNames).
   void showOpenFileNamesDialogSlot(QString caption, QString dir, QString filter, QStringList *resultFileNames, QWidget* parent);

   ///This slot function shows a QFileDialog::getExistingDirectory.
   void showDirectoryDialogSlot(QString caption, QString dir, QString *directoryName, QWidget *parent);

   ///Convenience function to get color settings from the user.
   void showColorDialogSlot(quint8 initInitalRed, quint8 initInitalGreen, quint8 initInitalBlue, quint8 initInitalAlpha,
                                    bool alphaIsEnabled, QWidget* parent, QList<int>* result);

   ///Loads an user interface file.
   void loadUserInterfaceFileSlot(QWidget** scriptUi, QString path);

   ///With this slot function a script thread can create a gui element (for instance PlotWindow).
   ///This slot function must be connect with  Qt::BlockingQueuedConnection to work properly.
   ///Note: Gui elements in Qt can only be created in the main thread.
   void createGuiElementSlot(QString elementType, QObject **createdGuiElement, ScriptWindow *scriptWindow, ScriptThread *scriptThread, QObject* additionalArgument);

   ///Sets the item text.
   void setItemTextSlot(int index, QString text, QToolBox* box){box->setItemText(index, text);}

   ///Sets the current item index.
   void setCurrentIndex(int index, QToolBox* box){box->setCurrentIndex(index);}

   ///Sets the button text.
   void setTextSlot(const QString text, QPushButton* button){button->setText(text);}

   ///Sets the check box text.
   void setTextSlot(const QString text, QCheckBox* box){box->setText(text);}

   ///Sets the checked state of the check box.
   void setCheckedSlot(bool checked, QCheckBox* box){box->setChecked(checked);}

   ///Adds one item to the combo box.
   void addItemSlot(const QString text, QComboBox* box){box->addItem(text);}

   ///Inserts one item into the combo box.
   void insertItemSlot(int index, const QString text, QComboBox* box){box->insertItem(index, text);}

   ///Removes one item from the combo box.
   void removeItemSlot(int index, QComboBox* box){box->removeItem(index);}

   ///Sets the editable property of the combo box.
   ///If the editable property is true, then the text of the selected item can be changed.
   void setEditableSlot(bool editable, QComboBox* box){box->setEditable(editable);}

   ///Sets the item (identified by index) text.
   void setItemTextSlot(int index, const QString text, QComboBox* box){box->setItemText(index, text);}

   ///If the user uses the arrows to change the spin box's value the value will be
   ///incremented/decremented by the amount of the single step. The default value is 1.0.
   ///Setting a single step value of less than 0 does nothing.
   void setSingleStepSlot(double value, QDoubleSpinBox* spinBox){spinBox->setSingleStep(value);}

   ///If the user uses the arrows to change the spin box's value the value will be
   ///incremented/decremented by the amount of the single step. The default value is 1.
   ///Setting a single step value of less than 0 does nothing.
   void setSingleStepSlot(int value, QSpinBox* spinBox){spinBox->setSingleStep(value);}

   ///Sets the group box title.
   void setTitleSlot(const QString text, QGroupBox* box){box->setTitle(text);}

   ///Sets the editable property of the line edit.
   void setReadOnlySlot(bool readOnly, QLineEdit* lineEdit){lineEdit->setReadOnly(readOnly);}

   ///Sets the background color of an item.
   void setItemBackgroundColorSlot(QBrush brush, QListWidgetItem* item){item->setBackground(brush);}

   ///Sets the foreground color of an item.
   void setItemForegroundColorSlot(QBrush brush, QListWidgetItem* item){item->setForeground(brush);}

   ///Sets the item text.
   void setItemTextSlot(QString text, QListWidgetItem* item){item->setText(text);}

   ///Sets the radio button text.
   void setTextSlot(QString text, QRadioButton* button){button->setText(text);}

   ///Sets the checked state of the radio button.
   void setCheckedSlot(bool checked, QRadioButton* button){button->setChecked(checked);}

   ///Sets the button text.
   void setTextSlot(QString text, QToolButton* button){button->setText(text);}

   ///Adds a column in the header for each item in the labels list, and sets the label for each column.
   ///Note that setHeaderLabels() won't remove existing columns.
   void setHeaderLabelsSignal(QStringList labels, QTreeWidget* tree){tree->setHeaderLabels(labels);}

   ///Sets the number of columns displayed in the tree widget.
   void setColumnCountSlot(int columns, QTreeWidget* tree){tree->setColumnCount(columns);}

   ///Appends the item as a top-level item in the widget.
   void addTopLevelItemSlot(QTreeWidgetItem* item, QTreeWidget* tree){tree->addTopLevelItem(item);}

   ///Inserts the item at index in the top level in the view.
   ///If the item has already been inserted somewhere else it won't be inserted.
   void insertTopLevelItemSlot(int index, QTreeWidgetItem* item, QTreeWidget* tree){tree->insertTopLevelItem(index, item);}

   ///Removes the top-level item at the given index in the tree and returns it, otherwise returns null.
   void takeTopLevelItemSlot(int index, QTreeWidgetItem** item, QTreeWidget* tree){*item = tree->takeTopLevelItem(index);}

   ///Sets the text to be displayed in the given column to the given text.
   void setTextSlot(int column, QString text, QTreeWidgetItem* item){item->setText(column, text);}

   ///Removes the item at index and returns it, otherwise return null.
   void takeChildSlot(int index, QTreeWidgetItem** child, QTreeWidgetItem* item){*child = item->takeChild(index);}

   ///Sorts the children of the item using the given order(true=AscendingOrder,
   ///false=DescendingOrder) by the values in the given column.
   void sortChildrenSlot(int column, bool ascendingOrder, QTreeWidgetItem* item){item->sortChildren(column, ascendingOrder ? Qt::AscendingOrder : Qt::DescendingOrder);}

   ///Sets the background color of the label in the given column to the specified color.
   ///Possible colors are: black, white, gray, red, green, blue, cyan, magenta and yellow.
   void setBackgroundColorSlot(int column, QString color, QTreeWidgetItem* item){item->setBackground(column, QBrush(stringToGlobalColor(color)));}

   ///Sets the foreground color of the label in the given column to the specified color.
   ///Possible colors are: black, white, gray, red, green, blue, cyan, magenta and yellow.
   void setForegroundColorSlot(int column, QString color, QTreeWidgetItem* item){item->setForeground(column, QBrush(stringToGlobalColor(color)));}

   ///Disables the item if disabled is true; otherwise enables the item.
   void setDisabledSlot(bool disabled, QTreeWidgetItem* item){item->setDisabled(disabled);}

   ///Sets the item for the given row and column to item.
   void setItemSlot(int row, int column, QTableWidgetItem *item, QTableWidget* tableWidget){tableWidget->setItem(row, column, item);}

   ///Sets the item's text to the text specified.
   void setTextSlot(const QString text, QTableWidgetItem *item){item->setText(text);}

   ///Sets the vertical header item for row to item.
   void setVerticalHeaderItemSlot(int row, QTableWidgetItem *item, QTableWidget* tableWidget){tableWidget->setVerticalHeaderItem(row, item);}

   ///Sets the horizontal header item for column to item.
   void setHorizontalHeaderItemSlot(int column, QTableWidgetItem *item, QTableWidget* tableWidget){tableWidget->setHorizontalHeaderItem(column, item);}

   ///Sets the flags for the item to the given flags. These determine whether the item can be selected or modified.
   void setFlagsSlot(Qt::ItemFlags flags, QTableWidgetItem *item){item->setFlags(flags);}

   ///Sets the number of rows.
   void setRowCountSlot(int rows, QTableWidget* tableWidget){tableWidget->setRowCount(rows);}

   ///Sets the number of columns.
   void setColumnCountSlot(int columns, QTableWidget* tableWidget){tableWidget->setColumnCount(columns);}

   ///Sets the item's background color.
   void setCellBackgroundColorSlot(QBrush brush, QTableWidgetItem *item){item->setBackground(brush);}

   ///Sets the item's foreground color.
   void setCellForegroundColorSlot(QBrush brush, QTableWidgetItem *item){item->setForeground(brush);}

   ///Sets the height of the given row to be height.
   void setRowHeightSlot(int row, int height, QTableWidget* tableWidget){tableWidget->setRowHeight(row, height);}

   ///Sets the width of the given column to be width.
   void setColumnWidthSlot(int column, int width, QTableWidget* tableWidget){tableWidget->setColumnWidth(column, width);}

   ///With this function the user can move the selected row up or down
   ///(while holding the left mouse button at the row and moving the mouse up and/or down).
   void cellEnteredSlot(int row, int rowsel, int column, QTableWidget* tableWidget);

   ///This slot is used to add a validator the line edit.
   void addValidatorSignal(QValidator* validator, QLineEdit* lineEdit);

   ///Process stored operations from a ScriptTextEdit.
   void processStoredOperationsSlot(QTextEdit* textEdit, bool isLocked, quint32 maxChars, QVector<ScriptTextEditStoredOperations_t>* m_storedOperations);

   ///This function inserts one row at row and fills the cells with content.
   ///Possible colors are: black, white, gray, red, green, blue, cyan, magenta and yellow.
   void insertRowWithContentSlot(int row, QStringList texts, QStringList backgroundColors, QStringList foregroundColors, QTableWidget* tableWidget);

   ///Sets the window icon of a dialog or a main window.
   ///Supported formats: .ico, .gif, .png, .jpeg, .tiff, .bmp, .icns.
   void setWindowIconSlot(QString iconFile, QWidget* widget){widget->setWindowIcon(QIcon(iconFile));}

   ///Creates a shortcut.
   void createShortCutSlot(QString keys, QWidget* parent, QShortcut **shortCut);

   ///Sets the style sheet of a script widget.
   void setStyleSheetSlot(QString styleSheet, QWidget* element);

};

#endif // SCRIPTSLOTS_H

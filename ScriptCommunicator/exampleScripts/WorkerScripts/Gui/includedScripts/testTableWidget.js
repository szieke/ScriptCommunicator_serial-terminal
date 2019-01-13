
//the content of a cell has been changed
function testSendTableWidgetCellChanged(row, column)
{
	UI_testTextEdit.append("UI_testSendTableWidgetCellChanged");
	UI_testReceiveTableWidget.setText(row, column, UI_testSendTableWidget.getText(row, column));
	UI_testReceiveTableWidget.setCellBackgroundColor("red", row, column);
}
//Adjust the width of the right column, so that all columns fit in the complete table.
function adjustSendTableColmnWidth()
{
	var verticalScrollBarWidth = 0;
	if(UI_testSendTableWidget.isVerticalScrollBarVisible())
	{
		verticalScrollBarWidth = UI_testSendTableWidget.verticalScrollBarWidth();
	}
	UI_testSendTableWidget.setColumnWidth(1, UI_testSendTableWidget.width() -
                           (UI_testSendTableWidget.columnWidth(0)
                           + 2 * UI_testSendTableWidget.frameWidth()
                           + UI_testSendTableWidget.verticalHeaderWidth()
						   + verticalScrollBarWidth));
}
//Adjust the width of the right column, so that all columns fit in the complete table.
function adjustReceiveTableColmnWidth()
{
	var verticalScrollBarWidth = 0;
	if(UI_testReceiveTableWidget.isVerticalScrollBarVisible())
	{
		verticalScrollBarWidth = UI_testReceiveTableWidget.verticalScrollBarWidth();
	}
	UI_testReceiveTableWidget.setColumnWidth(1, UI_testReceiveTableWidget.width() -
                           (UI_testReceiveTableWidget.columnWidth(0)
                           + 2 * UI_testReceiveTableWidget.frameWidth()
                           + UI_testReceiveTableWidget.verticalHeaderWidth()
						   + verticalScrollBarWidth));
}
//The width of a header has been changed.
function sendTableHorizontalHeaderSectionResizedSignal(logicalIndex, oldSize, newSize)
{
	adjustSendTableColmnWidth();
}
//The width of a header has been changed.
function receiveTableHorizontalHeaderSectionResizedSignal(logicalIndex, oldSize, newSize)
{
	adjustReceiveTableColmnWidth();
}

function ReceiveTableSelectionChanged()
{
	UI_testTextEdit.append("ReceiveTableSelectionChanged");
	
	var cells = UI_testReceiveTableWidget.getAllSelectedCells();
	if(cells.length != 0)
	{
		for(var i = 0; i < cells.length; i++)
		{
			UI_testTextEdit.append('column: ' + cells[i].column + '  row: ' + cells[i].row);
		}
	}
	else
	{
		UI_testTextEdit.append('no selected cells');
	}
}

function SendTableSelectionChanged()
{
	UI_testTextEdit.append("ReceiveTableSelectionChanged");
	
	var cells = UI_testSendTableWidget.getAllSelectedCells();
	if(cells.length != 0)
	{
		for(var i = 0; i < cells.length; i++)
		{
			UI_testTextEdit.append('column: ' + cells[i].column + '  row: ' + cells[i].row);
		}
	}
	else
	{
		UI_testTextEdit.append('no selected cells');
	}
}

//test group box
UI_groupBoxTableWidgets.setTitle(UI_groupBoxTableWidgets.title());


UI_testReceiveTableWidget.setRowCount(2);
UI_testReceiveTableWidget.setColumnCount(2);
UI_testReceiveTableWidget.setVerticalHeaderLabel(0, "ver1");
UI_testReceiveTableWidget.setVerticalHeaderLabel(1, "ver2");
UI_testReceiveTableWidget.setHorizontalHeaderLabel(0, "hor1");
UI_testReceiveTableWidget.setHorizontalHeaderLabel(1, "hor2");
UI_testReceiveTableWidget.setCellEditable(0,0, false);
UI_testReceiveTableWidget.setCellEditable(1,0, false);
UI_testReceiveTableWidget.insertRow(UI_testReceiveTableWidget.rowCount() )
UI_testReceiveTableWidget.setHorizontalHeaderLabel(2, "hor3");
UI_testReceiveTableWidget.setText(0, 0, "test1");
UI_testReceiveTableWidget.setText(1, 0, "test2");
UI_testReceiveTableWidget.setText(2, 0, "test3");
UI_testReceiveTableWidget.sortItems(0, false);
UI_testReceiveTableWidget.resizeColumnToContents(0);
UI_testReceiveTableWidget.resizeRowToContents(0);

UI_testReceiveTableWidget.cellSelectionChangedSignal.connect(ReceiveTableSelectionChanged)

UI_testReceiveTableWidget.setRowHeight(1, UI_testReceiveTableWidget.rowHeight(1) + 20);
UI_testReceiveTableWidget.setColumnWidth(1, UI_testReceiveTableWidget.columnWidth(1) + 50);

UI_testSendTableWidget.setRowCount(2);
UI_testSendTableWidget.setColumnCount(2);
UI_testSendTableWidget.setVerticalHeaderLabel(0, "ver1");
UI_testSendTableWidget.setVerticalHeaderLabel(1, "ver2");
UI_testSendTableWidget.setHorizontalHeaderLabel(0, "hor1");
UI_testSendTableWidget.setHorizontalHeaderLabel(1, "hor2");
UI_testSendTableWidget.setCellEditable(0,0, false);
UI_testSendTableWidget.setCellEditable(1,0, false);
UI_testSendTableWidget.insertRow(UI_testSendTableWidget.rowCount() )
UI_testSendTableWidget.setHorizontalHeaderLabel(2, "hor3");
UI_testSendTableWidget.setText(0, 0, "test1");
UI_testSendTableWidget.setCellIcon(0, 0, scriptFile.createAbsolutePath("icons/folder.gif"));
UI_testSendTableWidget.setText(1, 0, "test2");
UI_testSendTableWidget.setText(2, 0, "test3");
UI_testReceiveTableWidget.setCellForegroundColor("blue", 2, 0);

UI_testSendTableWidget.cellSelectionChangedSignal.connect(SendTableSelectionChanged)
						   
UI_testSendTableWidget.horizontalHeaderSectionResizedSignal.connect(sendTableHorizontalHeaderSectionResizedSignal)
UI_testReceiveTableWidget.horizontalHeaderSectionResizedSignal.connect(receiveTableHorizontalHeaderSectionResizedSignal)

//Start a timer which periodically calls adjustTableColmnWidth(if the dialog/table 
//size has been changed the columns have to be adjusted)
var tableAdjustWidthTimer = scriptThread.createTimer()
tableAdjustWidthTimer.timeoutSignal.connect(adjustSendTableColmnWidth)
tableAdjustWidthTimer.timeoutSignal.connect(adjustReceiveTableColmnWidth)
tableAdjustWidthTimer.start(200);

/******************test widgets in table***************************************/
function tableComboBox1TextChanged(text)
{
	var row = UI_tableComboBox1.getAdditionalData(0);
	var column = UI_tableComboBox1.getAdditionalData(1);
	UI_testReceiveTableWidget.setText(row, column, text);
}
function tableLineEdit1TextChanged(text)
{
	var row = UI_tableLineEdit1.getAdditionalData(0);
	var column = UI_tableLineEdit1.getAdditionalData(1);
	UI_testReceiveTableWidget.setText(row, column, text);
}
function tableButton1Clicked()
{
	UI_testTextEdit.append("UI_tableButton1Clicked: " + UI_tableButton1.isCheckable() + "  " + UI_tableButton1.isChecked());
}
function tableCheckBoxClicked(checked)
{
	UI_testTextEdit.append("UI_tableCheckboxClicked: " + checked);
}
function tableSpinBoxValueChanged(value)
{
	UI_testTextEdit.append("UI_tableSpinBoxValueChanged: " + value);
}
function tableDateTimeEditDateTimeChanged(dateTime)
{
	UI_testTextEdit.append("UI_tableDateTimeEditDateTimeChanged: " + dateTime);
}

function tableDoubleSpinBoxValueChanged(value)
{
	UI_testTextEdit.append("UI_tableDoubleSpinBoxValueChanged: " + value);
}
function tableVerticalSliderValueChanged(value)
{
	UI_testTextEdit.append("UI_tableVerticalSliderValueChanged: " + value);
}
function tableCalendarSelectionChanged(date)
{
	UI_testTextEdit.append("UI_tableCalendarselectionChanged: " + date  + " ; " + UI_tableCalendarWidget1.getSelectedDate());
}

function tableHorizontalSliderValueChanged(value)
{
	UI_testTextEdit.append("UI_tableHorizontalSliderValueChanged: " + value);
}
function tableDialValueChanged(value)
{
	UI_testTextEdit.append("UI_tableDialValueChanged: " + value);
}
function tableTimeEditTimeChangedSignal(time)
{
	UI_testTextEdit.append("UI_tableTimeEditTimeChangedSignal: " + time);
}
function tableDateEditTimeChangedSignal(data)
{
	UI_testTextEdit.append("UI_tableDateEditTimeChangedSignal: " + data);
}
function tableTextEditTextChangedSignal()
{
	UI_testTextEdit.append("UI_tableTextEditTextChangedSignal: " + UI_tableTextEdit1.toPlainText());
}

UI_testSendTableWidget.insertWidget(2, 0, "ComboBox");
var UI_tableComboBox1 = UI_testSendTableWidget.getWidget(2, 0)
UI_tableComboBox1.addItem("val1");
UI_tableComboBox1.addItem("val2");
UI_tableComboBox1.setCurrentIndex(0)
UI_tableComboBox1.currentTextChangedSignal.connect(tableComboBox1TextChanged)
UI_tableComboBox1.setAdditionalData(0, 2);
UI_tableComboBox1.setAdditionalData(1, 0);

UI_testSendTableWidget.insertWidget(2, 1, "LineEdit");
var UI_tableLineEdit1 = UI_testSendTableWidget.getWidget(2, 1)
UI_tableLineEdit1.textChangedSignal.connect(tableLineEdit1TextChanged);
UI_tableLineEdit1.setAdditionalData(0, 2);
UI_tableLineEdit1.setAdditionalData(1, 1);

UI_testSendTableWidget.insertWidget(3, 0, "Button");
var UI_tableButton1 = UI_testSendTableWidget.getWidget(3, 0)
UI_tableButton1.clickedSignal.connect(tableButton1Clicked)
UI_tableButton1.setCheckable(true);
UI_tableButton1.setChecked(true);

UI_testSendTableWidget.insertWidget(3, 1, "CheckBox");
var UI_tableCheckBox1 = UI_testSendTableWidget.getWidget(3, 1)
UI_tableCheckBox1.clickedSignal.connect(tableCheckBoxClicked)

UI_testSendTableWidget.insertWidget(4, 0, "VerticalSlider");
var UI_tableVerticalSlider1 = UI_testSendTableWidget.getWidget(4, 0)
UI_tableVerticalSlider1.valueChangedSignal.connect(tableVerticalSliderValueChanged)

UI_testSendTableWidget.insertWidget(4, 1, "HorizontalSlider");
var UI_tableHorizontalSlider1 = UI_testSendTableWidget.getWidget(4, 1)
UI_tableHorizontalSlider1.valueChangedSignal.connect(tableHorizontalSliderValueChanged)

UI_testSendTableWidget.insertWidget(5, 0, "DoubleSpinBox");
var UI_tableDoubleSpinBox1 = UI_testSendTableWidget.getWidget(5, 0)
UI_tableDoubleSpinBox1.valueChangedSignal.connect(tableDoubleSpinBoxValueChanged)

UI_testSendTableWidget.insertWidget(5, 1, "Dial");
var UI_tableDial1 = UI_testSendTableWidget.getWidget(5, 1)
UI_tableDial1.valueChangedSignal.connect(tableDialValueChanged)

UI_testSendTableWidget.insertRowWithContent(1, Array("text1", "text2"), Array("red", "green"), Array("white", "black"))

UI_testReceiveTableWidget.insertWidget(3, 0, "SpinBox");
var UI_tableSpinBox1 = UI_testReceiveTableWidget.getWidget(3, 0)
UI_tableSpinBox1.valueChangedSignal.connect(tableSpinBoxValueChanged)

UI_testReceiveTableWidget.insertWidget(3, 1, "TimeEdit");
var UI_tableTimeEdit1 = UI_testReceiveTableWidget.getWidget(3,1)
UI_tableTimeEdit1.timeChangedSignal.connect(tableTimeEditTimeChangedSignal)

UI_testReceiveTableWidget.insertWidget(4, 0, "DateEdit");
var UI_tableDateEdit1 = UI_testReceiveTableWidget.getWidget(4, 0)
UI_tableDateEdit1.dateChangedSignal.connect(tableDateEditTimeChangedSignal)

UI_testReceiveTableWidget.insertWidget(4, 1, "TextEdit");
var UI_tableTextEdit1 = UI_testReceiveTableWidget.getWidget(4, 1)
UI_tableTextEdit1.textChangedSignal.connect(tableTextEditTextChangedSignal)

UI_testReceiveTableWidget.insertWidget(5, 0, "DateTimeEdit");
var UI_tableDateTimeEdit1 = UI_testReceiveTableWidget.getWidget(5, 0)
UI_tableDateTimeEdit1.dateTimeChangedSignal.connect(tableDateTimeEditDateTimeChanged)

UI_testReceiveTableWidget.insertWidget(5, 1, "CalendarWidget");
var UI_tableCalendarWidget1 = UI_testReceiveTableWidget.getWidget(5, 1)
UI_tableCalendarWidget1.selectionChangedSignal.connect(tableCalendarSelectionChanged)
UI_testReceiveTableWidget.resizeRowToContents(5);

var invalidWidget = UI_testReceiveTableWidget.getWidget(0, 0)
if(typeof invalidWidget == 'undefined')
{
	UI_testTextEdit.append("invalidWidget: " + invalidWidget);
}
UI_testTextEdit.append("class name: " + UI_tableCalendarWidget1.getClassName());



UI_testSendTableWidget.resizeColumnToContents(0);
adjustSendTableColmnWidth();
UI_testReceiveTableWidget.resizeColumnToContents(0);
adjustReceiveTableColmnWidth();

//wait 100ms
scriptThread.sleepFromScript(100);
UI_testSendTableWidget.cellChangedSignal.connect(testSendTableWidgetCellChanged);


/*************************************************************************
This worker script (worker scripts can be added in the script window) demonstrates the the gui 
functionalities which can be used by a worker script. 
***************************************************************************/
//Is called if this script shall be exited.
function stopScript() 
{
    scriptThread.appendTextToConsole("script gui example stopped: " + scriptThread.getTimestamp());
}

//the user has changed the item index
function toolBoxCurrentItemChangedSignal(index)
{
	UI_testTextEdit.append("UI_toolBoxCurrentItemChangedSignal: " + index);
}

//the user has changed the tab index
function currentTabChangedSignal(index)
{
	UI_testTextEdit.append("UI_currentTabChangedSignal: " + index);
}

//the user has changed the date
function calendarSelectionChangedSignal(date)
{
	UI_testTextEdit.append("UI_calendarSelectionChangedSignal: " + date + " ; " + UI_calendar.getSelectedDate());

	UI_dateEdit.blockSignals(true);
	UI_dateEdit.setDate(date);
	UI_dateEdit.blockSignals(false);	
	
	//Test ScriptWidget::selectCell
	UI_testSendTableWidget.selectCell(2, 0);
}
//the user has changed the date
function dateEditDateChanged(date)
{
	UI_testTextEdit.append("UI_dateEditDateChanged: " + date + "  " + UI_dateEdit.getDate());
	UI_calendar.setSelectedDate(date);
}


//the user has changed the date and/or the time
function dateTimeEditTimeChanged(dateTime)
{

	UI_testTextEdit.append("UI_dateTimeEditTimeChanged: " + dateTime + "  " + UI_dateTimeEdit.getDateTime());	
	
	var dateTimeSplit = UI_dateTimeEdit.getDateTime().split(" ");
	UI_timeEdit.setTime(dateTimeSplit[1]);	
}


//the user has changed the time
function timeEditTimeChanged(time)
{
	UI_testTextEdit.append("UI_timeEditTimeChanged: " + time + "  " + UI_timeEdit.getTime());	
}


//the user has pressed the create process detached button
function createProcessDetachedPushButtonClicked()
{
	UI_testTextEdit.append("UI_createProcessDetachedPushButtonClicked");
	if(!scriptThread.createProcessDetached(UI_createProcessProgramLineEdit.text(), 
									   UI_createProcessArgumentsLineEdit.text().split(";"),
									   ""))
	{
		scriptThread.messageBox("Critical", "error", 'could not create process')
	}

}
//the user has changed the line edit text
function createProcessPushButtonClicked()
{
	UI_testTextEdit.append("UI_createProcessPushButtonClicked");
	if(0 != scriptThread.createProcess(UI_createProcessProgramLineEdit.text(), 
									   UI_createProcessArgumentsLineEdit.text().split(";")))
	{
		scriptThread.messageBox("Critical", "error", 'could not create process')
	}
	
	var standartOut = scriptThread.readAllStandardOutputLastProcess();
	var standartError = scriptThread.readAllStandardErrorLastProcess();

}


//the user has closed the dialog
function DialogFinished(e)
{
	UI_testTextEdit.append("UI_DialogFinished");
	scriptThread.stopScript()
}

function NewElementsButtonButtonClicked()
{
	UI_testTextEdit.append("NewElementsButtonButtonClicked: " + this.isCheckable() + "  " + this.isChecked());
	UI_testTextEdit.append("NewElementsButtonButtonClicked: " + this.getObjectName());
	UI_testTextEdit.append("NewElementsButtonButtonClicked: " + testButton.getObjectName());
	UI_testTextEdit.append("NewElementsButtonButtonClicked: " + UI_NewElementsButton.getObjectName());
	
	var input = scriptThread.showTextInputDialog("Enter name", "item name", "newItem", UI_Dialog.getWidgetPointer())
	
	if(input != "")
	{
		UI_sendListWidget.insertNewItem(UI_sendListWidget.rowCount(), input, scriptFile.createAbsolutePath("icons/folder.gif"));
		
		var treeItem1 = UI_treeWidget.createScriptTreeWidgetItem();
		treeItem1.setText(0, input);
		treeItem1.setItemIcon(0, scriptFile.createAbsolutePath("icons/openfolder.gif"));
		UI_treeWidget.addTopLevelItem(treeItem1);
		
		UI_testReceiveTableWidget.insertRow(UI_testReceiveTableWidget.rowCount() )
		UI_testReceiveTableWidget.setVerticalHeaderLabel(UI_testReceiveTableWidget.rowCount() - 1, "hor" + + UI_testReceiveTableWidget.rowCount());
		UI_testReceiveTableWidget.setText(UI_testReceiveTableWidget.rowCount() - 1, 0, input);
		UI_testReceiveTableWidget.resizeColumnToContents(0);
		
		UI_testSendTableWidget.insertRow(UI_testSendTableWidget.rowCount() )
		UI_testSendTableWidget.setVerticalHeaderLabel(UI_testSendTableWidget.rowCount() - 1, "hor" + + UI_testSendTableWidget.rowCount());
		UI_testSendTableWidget.setText(UI_testSendTableWidget.rowCount() - 1, 0, input);
		UI_testSendTableWidget.resizeColumnToContents(0);
	}
}

function secondDialogOkButtonClicked()
{
	UI_testTextEdit.append("UI_secondDialogOkButtonClicked: " + UI_secondDialogLineEdit.text());
}

try
{
	//Test if the GUI has already been loaded.
	//If not, then an exception will be occur.
	UI_createProcessProgramLabel.text();
}
catch(e)
{
	scriptThread.loadUserInterfaceFile("guiExample.ui");
}

//Get all script arguments (command-line argument -A)
var argList = scriptThread.getScriptArguments();
for(var i = 0; i < argList.length; i++)
{
	UI_testTextEdit.append("script argument: " + argList[i]);
}

scriptThread.appendTextToConsole('script gui example started:'  + scriptThread.getTimestamp());

//create and delete a timer object
var timer2 = scriptThread.createTimer()
timer2 = undefined;


UI_testTextEdit.setMaxChars(100000)
UI_testTextEdit.append("Skript start");


/********************************test the file api*********************/
if(scriptFile.checkFileExists("testFileFromGuiExample.txt"))
{
	var fileContent = scriptFile.readFile("testFileFromGuiExample.txt");
	UI_testTextEdit.append(fileContent);
}
else
{
	UI_testTextEdit.append(scriptFile.createAbsolutePath("testFileFromGuiExample.txt") + " does not exist");
}
scriptFile.writeFile("testFileFromGuiExample.txt", true, "test file content\r\nline 2", true);
/***********************************************************************/



UI_Dialog.setWindowTitle("example gui used by script and created with QtDesigner");
UI_Dialog.finishedSignal.connect(DialogFinished);
	

//test the script label
UI_createProcessProgramLabel.setText(UI_createProcessProgramLabel.text());

/****************create process***********************************/
UI_createProcessDetached.clickedSignal.connect(createProcessDetachedPushButtonClicked)
UI_createProcess.clickedSignal.connect(createProcessPushButtonClicked)
UI_createProcessProgramLineEdit.setText("C:/Users/internet/Desktop/npp.6.6.9.bin/notepad++.exe");
UI_createProcessArgumentsLineEdit.setText("C:\\Users\\internet\\Desktop\\npp.6.6.9.bin\\stylers.xml;C:\\Users\\internet\\Desktop\\npp.6.6.9.bin\\session.xml");
/***********************************************************************/

/****************tabWidget************************/
UI_tabWidget1.setTabText(1, UI_tabWidget1.tabText(1));
UI_tabWidget1.setCurrentIndex(UI_tabWidget1.currentIndex() + 1);
UI_tabWidget1.currentTabChangedSignal.connect(currentTabChangedSignal);

/****************tool box***********************************/
UI_toolBox.setItemText(1, UI_toolBox.itemText(1));
UI_toolBox.setCurrentIndex(UI_toolBox.currentIndex() + 1);
UI_toolBox.currentItemChangedSignal.connect(toolBoxCurrentItemChangedSignal);

/****************time / date / calendar************************/
UI_timeEdit.setDisplayFormat("hh:mm:ss");
UI_timeEdit.setTime("10:09:08");
UI_timeEdit.timeChangedSignal.connect(timeEditTimeChanged);

UI_dateEdit.setDisplayFormat("dd.MM.yyyy");
UI_dateEdit.setDate("01.02.2014");
UI_dateEdit.dateChangedSignal.connect(dateEditDateChanged);

UI_calendar.setDateFormat("dd.MM.yyyy");
UI_calendar.setDateRange("01.01.2014", "01.01.2016");
UI_calendar.setSelectedDate("01.02.2015");
UI_calendar.selectionChangedSignal.connect(calendarSelectionChangedSignal);


UI_dateTimeEdit.setDisplayFormat("dd.MM.yyyy hh:mm:ss");
UI_dateTimeEdit.setDateTime("01.01.2016 10:09:08");
UI_dateTimeEdit.dateTimeChangedSignal.connect(dateTimeEditTimeChanged);
/***********************************************************************/

//new elements
UI_NewElementsButton.clickedSignal.connect(UI_NewElementsButton, NewElementsButtonButtonClicked)
UI_NewElementsButton.setCheckable(true);
UI_NewElementsButton.setChecked(true);
var testButton = UI_NewElementsButton;
UI_NewElementsButton.setObjectName("testButton");
UI_NewElementsButton.setIcon(scriptFile.createAbsolutePath("icons/new.png"));
		
//test text edit, checkbox, radio button
scriptThread.loadScript("includedScripts/testLineEdit_CheckBox_RadioButton.js");

//test progress bar / slider /dial
scriptThread.loadScript("includedScripts/testProgressBar_Slider_SpinBox_Dial.js");

//test table widget
scriptThread.loadScript("includedScripts/testTableWidget.js");

//test list widget
scriptThread.loadScript("includedScripts/testListWidget.js");

//test tree widget
scriptThread.loadScript("includedScripts/testTreeWidget.js");

//test the loading of a script by an other script and call a function
scriptThread.loadScript("includedScripts/includedScript.js");
includeFunction()

if(!scriptThread.loadUserInterfaceFile("secondDialog.ui", true, false))
{
	scriptThread.messageBox("Critical", "error", 'could not load: secondDialog.ui')
}
else
{
	UI_secondDialogLineEdit.setText("item text");
	UI_secondDialogOkButton.clickedSignal.connect(secondDialogOkButtonClicked)
	UI_secondDialog.show();
	
	/********************test splitter****************************************/
	var sizeList = UI_SecondDialogSplitter.sizes();
	UI_testTextEdit.append(sizeList);
	
	//Change the sizes.
	sizeList[0] -= 50;
	sizeList[1] += 50;
	UI_SecondDialogSplitter.setSizes(sizeList);
	sizeList = UI_SecondDialogSplitter.sizes();
	UI_testTextEdit.append(sizeList);
	
	//Restore the old sizes.
	sizeList[0] += 50;
	sizeList[1] -= 50;
	UI_SecondDialogSplitter.setSizes(sizeList);
	sizeList = UI_SecondDialogSplitter.sizes();
	UI_testTextEdit.append(sizeList);
	/*************************************************************************/
	
	scriptThread.setScriptThreadPriority("NormalPriority");
}	


/*************************************************************************
This worker script (worker scripts can be added in the script window) 
demonstrates the usage of a main window.
***************************************************************************/

//Is called if this script shall be exited.
function stopScript() 
{
    scriptThread.appendTextToConsole("script test main window stopped");
}

//The dialog is closed.
function UI_MainWindowFinished(e)
{
	scriptThread.stopScript()
}

//the user has pressed the action
function UI_action1ClickedSlot(value)
{
	var text = "UI_action1ClickedSlot: ";
	if(UI_actionAction1.isChecked())
	{ 
		text += "checked";
	}
	else
	{
		text += "not checked";
	}
	
	//show a message in the status bar
	UI_statusbar.showMessage(text, 1000);
}

scriptThread.appendTextToConsole('script test main window started');


UI_MainWindow.finishedSignal.connect(UI_MainWindowFinished);
UI_MainWindow.setWindowTitle("main window example");
UI_MainWindow.setWindowPositionAndSize("100,100,300,300");

UI_actionAction1.setText("action 1 text");
var text = UI_actionAction1.text();
UI_actionAction1.setChecked(true);
UI_actionAction1.clickedSignal.connect(UI_action1ClickedSlot);




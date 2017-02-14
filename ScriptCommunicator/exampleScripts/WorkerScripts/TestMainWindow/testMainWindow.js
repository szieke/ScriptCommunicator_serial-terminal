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
function mainWindowFinished(e)
{
	scriptThread.stopScript()
}

//the user has pressed the action
function action1ClickedSlot(value)
{
	var text = "action1ClickedSlot: ";
	if(UI_actionAction1.isChecked())
	{ 
		text += "checked";
	}
	else
	{
		text += "not checked";
	}
	
	//show a message in the status bar
	UI_MainWindow.showMessage(text, 1000);
}

scriptThread.appendTextToConsole('script test main window started');


UI_MainWindow.finishedSignal.connect(mainWindowFinished);
UI_MainWindow.setWindowTitle("main window example");
UI_MainWindow.setWindowPositionAndSize("100,100,300,300");

UI_actionAction1.setText("action 1 text");
var text = UI_actionAction1.text();
UI_actionAction1.setChecked(true);
UI_actionAction1.clickedSignal.connect(action1ClickedSlot);




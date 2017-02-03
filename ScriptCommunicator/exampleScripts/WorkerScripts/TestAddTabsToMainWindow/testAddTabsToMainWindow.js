/***************************************************************************************
This worker script (worker scripts can be added in the script window) demonstrates how to add
a tab (with custom GUI elements) to the main window.
****************************************************************************************/
//Is called if this script shall be exited.
function stopScript() 
{
	scriptThread.setMainWindowTitle(oldMainWindowText);
    scriptThread.appendTextToConsole("script has been stopped");
}

//Is called if the 'start' button is pressed.
function start()
{
	g_currentProgressValue = 0;
	timer.start(10);
	UI_StartButton.setEnabled(false);
}

//Is called if the is elapsed.
function timerElapsed()
{
	//Increase the progress bar value.
	g_currentProgressValue++;
	UI_ProgressBar.setValue(g_currentProgressValue)
	UI_TextEdit1.append("progress: " + g_currentProgressValue)
	
	if(g_currentProgressValue >= maxValue)
	{
		timer.stop();
		UI_StartButton.setEnabled(true);
	}
}
//The Lock button in the main window has been pressed.
function mainWindowLockScrollingClicked(isChecked)
{
	UI_TextEdit1.lockScrolling(isChecked);
}
//The Clear button in the main window has been pressed.
function mainWindowClearConsoleClicked()
{
	UI_TextEdit1.clear();
}

scriptThread.appendTextToConsole('script has started');


//Hide the dialog (the tab will be removed from the dialog therefore the dialog is not needed).
UI_Dialog.hide();

var maxValue = 500;

//Remove the tab from the dialog and add it to the main window.
if(scriptThread.addTabsToMainWindow(UI_TabWidget))
{
	//Create a timer which sets the value of the progress bar.
	var timer = scriptThread.createTimer();
	timer.timeoutSignal.connect(timerElapsed);

	var g_currentProgressValue = 0;
	UI_ProgressBar.setMinimum(0);
	UI_ProgressBar.setMaximum(maxValue);

	UI_StartButton.clickedSignal.connect(start)
	
	scriptThread.mainWindowLockScrollingClickedSignal.connect(mainWindowLockScrollingClicked);
	scriptThread.mainWindowClearConsoleClickedSignal.connect(mainWindowClearConsoleClicked);

	scriptThread.messageBox("Information", "Information", "The tab from the dialog has been added to the main window");
}
else
{
	scriptThread.messageBox("Critical", "Error", "scriptThread::addTabsToMainWindow cannot be called in command-line mode");
	scriptThread.stopScript();
}

var oldMainWindowText = scriptThread.getMainWindowTitle();
scriptThread.setMainWindowTitle("ScriptText   " + oldMainWindowText);
UI_TextEdit1.setUpdateRate(50);

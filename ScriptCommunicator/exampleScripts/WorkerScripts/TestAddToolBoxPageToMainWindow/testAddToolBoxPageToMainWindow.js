/***************************************************************************************
This worker script (worker scripts can be added in the script window) demonstrates how to add
a tab (with custom GUI elements) to the main window.
****************************************************************************************/
//Is called if this script shall be exited.
function stopScript() 
{
    scriptThread.appendTextToConsole("script has been stopped");
}

//Is called if the 'start' button is pressed.
function start()
{
	g_currentProgressValue = 0;
	timer.start(25);
	UI_StartButton.setEnabled(false);
}

//Is called if the is elapsed.
function timerElapsed()
{
	//Increase the progress bar value.
	g_currentProgressValue++;
	UI_ProgressBar.setValue(g_currentProgressValue)
	UI_TextEdit1.append("progress: " + g_currentProgressValue)
	
	if(g_currentProgressValue >= 100)
	{
		timer.stop();
		UI_StartButton.setEnabled(true);
	}
}

scriptThread.appendTextToConsole('script has started');


//Hide the dialog (the tab will be removed from the dialog therefore the dialog is not needed).
UI_Dialog.hide();

//Remove the pages from the dialog and add it to the main window.
if(scriptThread.addToolBoxPagesToMainWindow(UI_ToolBox))
{
	//Create a timer which sets the value of the progress bar.
	var timer = scriptThread.createTimer();
	timer.timeoutSignal.connect(timerElapsed);

	var g_currentProgressValue = 0;
	UI_ProgressBar.setMinimum(0);
	UI_ProgressBar.setMaximum(100);

	UI_StartButton.clickedSignal.connect(start)

	scriptThread.messageBox("Information", "Information", "The page from the dialog has been added to the main window");
}
else
{
	scriptThread.messageBox("Critical", "Error", "scriptThread::addToolBoxPagesToMainWindow cannot be called in command-line mode");
	scriptThread.stopScript();
}


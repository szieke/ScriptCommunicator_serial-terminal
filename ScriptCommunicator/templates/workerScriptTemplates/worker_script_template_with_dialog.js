/*
In the main function (all code outside a function) only the script initialization code should be placed. 
The working code should be placed in asynchronous function calls (like timer callbacks or data receive callbacks).  
If the main function has been left the script does not stop. To stop a script call scriptThread.stopScript() or 
press the stop button (main or script window).
Example:
************************************************************************************************************
function dataReceived(data)
{
	//Working code.
	if(workDone)
	{
		scriptThread.stopScript();
	}
}

//Initialization code.
scriptThread.dataReceivedSignal.connect(dataReceived);
************************************************************************************************************

If the worker code shall be placed in the main function then scriptThread.scriptShallExit() should be called 
to check if the script must exit().
Example:
************************************************************************************************************
while(!scriptThread.scriptShallExit())
{
	//Working code.
}
************************************************************************************************************
*/

//Is called if this script shall be exited.
function stopScript() 
{
    scriptThread.appendTextToConsole("script has been stopped");
}

//Is called if the dialog is closed.
function UI_DialogFinished(e)
{
	scriptThread.stopScript()
}

scriptThread.appendTextToConsole('script has started');
UI_Dialog.finishedSignal.connect(UI_DialogFinished);




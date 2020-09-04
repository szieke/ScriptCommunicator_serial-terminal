/*
Important note:

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

function stopScript() 
{
    scriptThread.appendTextToConsole("@ScriptName@" +  "has been stopped");
}

//Send the data elements.
function sendElementFunc()
{
	sendTimer.stop();
	
	if(sendElements.length > sendElementsIndex)
	{
		if(scriptThread.sendDataArray(sendElements[sendElementsIndex]))
		{//Sending succeeded.
		
			sendElementsIndex++;
				
			if((sendPause != 0) && (sendElements.length > sendElementsIndex))
			{
				sendTimer.start(sendPause);
			}
			else
			{
				//Send the next element.
				sendElementFunc();
			}
		}
		else
		{//Sending failed.
		
			scriptThread.messageBox("Critical", "@ScriptName@", "sending failed");
			scriptThread.stopScript();
		}
	}
	else
	{//All elements have been sent.
		scriptThread.stopScript();
	}
}


scriptThread.appendTextToConsole("@ScriptName@" +  "has started");

var sendElements = Array();
var sendElementsIndex = 0;
var sendPause = @SendPause@;
@SendElementsInit@

var sendTimer = scriptThread.createTimer();
sendTimer.timeout.connect(sendElementFunc);

///Start sending.
sendElementFunc();






function stopScript() 
{
    scriptThread.appendTextToConsole("script has been stopped");
}
function UI_DialogFinished(e)
{
	scriptThread.stopScript()
}

function canMessageReceivedSlot(types, ids, timeStamps, data)
{
	
	for(var i = 0; i < types.length; i++)
	{
		var firstPart = data[i].splice(0, 4);
		var receivedCounter = conv.byteArrayToUint32(data[i], false);
		if((types[i] == 2) && (ids[i] == g_currentCanId) && (receivedCounter == g_counter) &&
			(firstPart[0] == 1) && (firstPart[1] == 2) && (firstPart[2] == 3) && (firstPart[3] == 4))
		{
			g_currentCanId++;
		}
		else
		{
			scriptThread.appendTextToConsole("received data is not correct: " + types[i] + " " + ids[i] + " " + g_currentCanId
			+ " " + receivedCounter + " " + g_counter + " " + firstPart[0]  + " " + firstPart[1]  + " " + firstPart[2]  + " " + firstPart[3]);
			stop()
		}
	}
}

function timerSlot()
{
	
	if(!g_stopSending)
	{
		if(g_currentCanId == 0x01020400 + g_numberOfMessages )
		{
			g_counter++;
			g_currentCanId = 0x01020400;
			UI_Counter1.setText("counter: " + g_counter);
			
			for(var i = 0; i < g_numberOfMessages; i++)
			{
				scriptInf.sendCanMessage(2, g_currentCanId + i, conv.addUint32ToArray(Array(1,2,3,4),g_counter, false));
			}
		}
	}
}

function start()
{
	g_stopSending = false;
	UI_StartButton.setEnabled(false);
	UI_StopButton.setEnabled(true);
	g_currentCanId = 0x01020400 + g_numberOfMessages ;
	timer.start(10);
}

function stop()
{
	g_stopSending = true;
	UI_StartButton.setEnabled(true);
	UI_StopButton.setEnabled(false);
	timer.stop();
}


var g_counter = 0;
var g_stopSending = true;
var g_currentCanId = 0x01020400;
var g_numberOfMessages = 10;

scriptThread.appendTextToConsole('script has started');

//Hide the dialog (the tab will be removed from the dialog therefore the dialog is not needed).
UI_Dialog.hide();

//Remove the pages from the dialog and add it to the main window.
if(scriptThread.addToolBoxPagesToMainWindow(UI_ToolBox))
{

	UI_Dialog.finishedSignal.connect(UI_DialogFinished);
	scriptInf.canMessagesReceivedSignal.connect(canMessageReceivedSlot);
	UI_StartButton.clickedSignal.connect(start);
	UI_StopButton.clickedSignal.connect(stop);
	scriptThread.setScriptThreadPriority("HighestPriority");
	var timer = scriptThread.createTimer();
	timer.timeoutSignal.connect(timerSlot);

}
else
{
	scriptThread.messageBox("Critical", "Error", "scriptThread::addToolBoxPagesToMainWindow cannot be called in command-line mode");
	scriptThread.stopScript();
}





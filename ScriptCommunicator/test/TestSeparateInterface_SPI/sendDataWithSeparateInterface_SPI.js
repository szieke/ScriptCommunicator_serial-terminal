

function stopScript() 
{
	g_interface.changePinConfiguration(0, true);
	g_interface.changePinConfiguration(1, true);
	
    scriptThread.appendTextToConsole("script has been stopped");
}
function UI_DialogFinished(e)
{
	scriptThread.stopScript()
}

function checkReceivedData(data)
{
	g_receivedData = g_receivedData.concat(data);
	
	if(g_receivedData.length == (g_compareString + g_counter).length)
	{	
		if(scriptThread.byteArrayToString(g_receivedData) == (g_compareString + g_counter))
		{
			g_counter++;
		}
		else
		{
			scriptThread.appendTextToConsole("received data is not correct");
			stop();
		}
		g_receivedData = Array();
	}
}

function aardvarkI2cSpiInputStatesChangedSlot(states)
{
	if(states[0])
	{	
		sendData();
	}
	else
	{
		g_interface.setOutput(1, true);
	}
}

function sendData()
{
	if(!g_stopSending)
	{
		var sendData = conv.stringToArray(g_compareString + g_counter);
		
		g_interface.spiMasterSendReceiveData(sendData);
		UI_Counter1.setText("counter: " + g_counter);	
		g_interface.setOutput(1, false);
		
		var receivedData = g_interface.spiMasterReadLastReceivedData();
		if(receivedData.length != 0)
		{
			checkReceivedData(receivedData);
		}
		else
		{
			scriptThread.appendTextToConsole("spiMasterReadLastReceivedData failed");
		}
	}
}

function start()
{
	g_stopSending = false;
	g_receivedData = Array();
	UI_StartButton.setEnabled(false);
	UI_StopButton.setEnabled(true);
	sendData();

}

function stop()
{
	g_stopSending = true;
	UI_StartButton.setEnabled(true);
	UI_StopButton.setEnabled(false);
}


var g_counter = 0;
var g_compareString = "\nTestdata SPI main interface: "
var g_receivedData = Array();
var g_stopSending = true;

scriptThread.appendTextToConsole('script has started');

//Hide the dialog (the tab will be removed from the dialog therefore the dialog is not needed).
UI_Dialog.hide();

//Remove the pages from the dialog and add it to the main window.
if(scriptThread.addToolBoxPagesToMainWindow(UI_ToolBox))
{

	var g_interface = scriptInf.aardvarkI2cSpiCreateInterface();
	var settings = scriptInf.aardvarkI2cSpiGetMainInterfaceSettings();
	if(!g_interface.connectToDevice(settings))
	{
		scriptThread.appendTextToConsole("connectToDevice failed");
		scriptThread.stopScript();
	}
	else
	{
		UI_Dialog.finishedSignal.connect(UI_DialogFinished);
		UI_StartButton.clickedSignal.connect(start);
		UI_StopButton.clickedSignal.connect(stop);
		
		g_interface.changePinConfiguration(0, true, true);
		g_interface.changePinConfiguration(1, false);

		g_interface.setOutput(1, false);
		scriptThread.sleepFromScript(500);
		g_interface.setOutput(1, true);
		
		g_interface.inputStatesChangedSignal.connect(aardvarkI2cSpiInputStatesChangedSlot);
		
		scriptThread.setScriptThreadPriority("HighestPriority");
	}

}
else
{
	scriptThread.messageBox("Critical", "Error", "scriptThread::addToolBoxPagesToMainWindow cannot be called in command-line mode");
	scriptThread.stopScript();
}



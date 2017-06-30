

function stopScript() 
{
	g_interface.changePinConfiguration(0, true);
	g_interface.changePinConfiguration(1, true);
	g_interface.disconnect();
    scriptThread.appendTextToConsole("SPI slave script has been stopped");
}

function aardvarkI2cSpiInputStatesChangedSlot(states)
{
	if(states[1])
	{		
		//Set the slave response.
		g_interface.slaveSetResponse(conv.stringToArray(g_sendData + g_counter));
		g_interface.setOutput(0, true);
	}
	else
	{
		g_interface.setOutput(0, false);
	}
}

function slaveDataSentSlot(data)
{
	scriptThread.addMessageToLogAndConsoles(scriptThread.byteArrayToString(data))
}
function slaveDataReceivedSlot(data)
{
	g_receivedData = g_receivedData.concat(data);
	
	if(g_receivedData.length == (g_sendData + g_counter).length)
	{	
		if(scriptThread.byteArrayToString(g_receivedData) == (g_sendData + g_counter))
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

var g_sendData = "\nTestdata SPI main interface: ";
var g_counter = 0;
var g_receivedData = Array();

var g_interface = scriptInf.aardvarkI2cSpiCreateInterface();
var settings = scriptInf.aardvarkI2cSpiGetMainInterfaceSettings();
if(!g_interface.connectToDevice(settings))
{
	scriptThread.appendTextToConsole("connectToDevice failed");
	scriptThread.stopScript();
}
else
{
	
	scriptThread.appendTextToConsole('SPI slave script has started');
	g_interface.slaveDataReceivedSignal.connect(slaveDataReceivedSlot);
	g_interface.slaveDataSentSignal.connect(slaveDataSentSlot);

	//Set the slave response.
	g_interface.slaveSetResponse(conv.stringToArray(g_sendData + g_counter));

	g_interface.changePinConfiguration(0, false);
	g_interface.changePinConfiguration(1, true, true);
	g_interface.setOutput(0, false);

	scriptThread.sleepFromScript(500);
	g_interface.inputStatesChangedSignal.connect(aardvarkI2cSpiInputStatesChangedSlot);

	scriptThread.setScriptThreadPriority("HighestPriority");
}
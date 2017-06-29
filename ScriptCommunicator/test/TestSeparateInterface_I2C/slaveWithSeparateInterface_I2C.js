

function stopScript() 
{
	g_interface.changePinConfiguration(2, true);
	g_interface.changePinConfiguration(3, true);
	g_interface.disconnect();
    scriptThread.appendTextToConsole("I2C slave script has been stopped");
}

function aardvarkI2cSpiInputStatesChangedSlot(states)
{
	if(states[3])
	{		
		//Set the slave response.
		g_interface.slaveSetResponse(conv.stringToArray(g_sendData + g_counter));
		g_interface.setOutput(2, true);
	}
	else
	{
		g_interface.setOutput(2, false);
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

var g_sendData = "\nTestdata I2C main interface: ";
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
	
	scriptThread.appendTextToConsole('I2C slave script has started');
	g_interface.slaveDataReceivedSignal.connect(slaveDataReceivedSlot);
	g_interface.slaveDataSentSignal.connect(slaveDataSentSlot);

	//Set the slave response.
	g_interface.slaveSetResponse(conv.stringToArray(g_sendData + g_counter));

	g_interface.changePinConfiguration(2, false);
	g_interface.changePinConfiguration(3, true, true);
	g_interface.setOutput(2, false);

	scriptThread.sleepFromScript(500);
	g_interface.inputStatesChangedSignal.connect(aardvarkI2cSpiInputStatesChangedSlot);

	scriptThread.setScriptThreadPriority("HighestPriority");
}
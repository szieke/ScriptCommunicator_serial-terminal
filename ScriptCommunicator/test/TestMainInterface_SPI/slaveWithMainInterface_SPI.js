

function stopScript() 
{
	scriptInf.aardvarkI2cSpiChangePinConfiguration(0, true);
	scriptInf.aardvarkI2cSpiChangePinConfiguration(1, true);
    scriptThread.appendTextToConsole("SPI slave script has been stopped");
}

function aardvarkI2cSpiInputStatesChangedSlot(states)
{
	if(states[1])
	{		
		//Set the slave response.
		scriptInf.sendString(g_sendData + g_counter);
		scriptInf.aardvarkI2cSpiSetOutput(0, true);
	}
	else
	{
		scriptInf.aardvarkI2cSpiSetOutput(0, false);
	}
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


scriptThread.appendTextToConsole('SPI slave script has started');
scriptInf.dataReceivedSignal.connect(slaveDataReceivedSlot);


//Set the slave response.
scriptInf.sendString(g_sendData + g_counter);

scriptInf.aardvarkI2cSpiChangePinConfiguration(0, false);
scriptInf.aardvarkI2cSpiChangePinConfiguration(1, true, true);
scriptInf.aardvarkI2cSpiSetOutput(0, false);

scriptThread.sleepFromScript(500);
scriptInf.aardvarkI2cSpiInputStatesChangedSignal.connect(aardvarkI2cSpiInputStatesChangedSlot);

scriptThread.setScriptThreadPriority("HighestPriority");
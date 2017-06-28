

function stopScript() 
{
    scriptThread.appendTextToConsole("SPI slave script has been stopped");
}

function aardvarkI2cSpiInputStatesChangedSlot(states)
{
	if(states[1])
	{		
		//Set the slave response.
		scriptInf.sendString(g_sendData + g_counter);
		scriptInf.aardvarkI2cSpiSetOutput(0, true);
		g_counter++;
	}
	else
	{
		scriptInf.aardvarkI2cSpiSetOutput(0, false);
	}
}

function slaveDataReceivedSlot(data)
{
	scriptInf.aardvarkI2cSpiSetOutput(0, false);
}

var g_sendData = "\nTestdata SPI main interface: ";
var g_counter = 0;

scriptThread.appendTextToConsole('SPI slave script has started');
scriptInf.slaveDataSentSignal.connect(slaveDataReceivedSlot);
scriptInf.aardvarkI2cSpiInputStatesChangedSignal.connect(aardvarkI2cSpiInputStatesChangedSlot);

//Set the slave response.
scriptInf.sendString(g_sendData + g_counter);

scriptInf.aardvarkI2cSpiChangePinConfiguration(0, false);
scriptInf.aardvarkI2cSpiChangePinConfiguration(1, true);
scriptInf.aardvarkI2cSpiSetOutput(0, true);

scriptThread.setScriptThreadPriority("HighestPriority");
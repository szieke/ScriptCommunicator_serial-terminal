

function stopScript() 
{
    scriptThread.appendTextToConsole("loopback script has been stopped");
}

function canMessageReceivedSlot(types, ids, timeStamps, data)
{
	for(var i = 0; i < types.length; i++)
	{
		g_interface.sendCanMessage(types[i], ids[i], data[i]);
	}
}

scriptThread.appendTextToConsole('loopback script has started');
var g_interface = scriptInf.createPcanInterface();
g_interface.open(2, 500, true, false);
g_interface.canMessagesReceivedSignal.connect(canMessageReceivedSlot);
scriptThread.setScriptThreadPriority("HighestPriority");


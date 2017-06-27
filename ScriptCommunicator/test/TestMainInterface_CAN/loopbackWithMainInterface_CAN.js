

function stopScript() 
{
    scriptThread.appendTextToConsole("loopback script has been stopped");
}

function canMessageReceivedSlot(types, ids, timeStamps, data)
{
	for(var i = 0; i < types.length; i++)
	{
		scriptInf.sendCanMessage(types[i], ids[i], data[i]);
	}
}

scriptThread.appendTextToConsole('loopback script has started');
scriptInf.canMessagesReceivedSignal.connect(canMessageReceivedSlot);
scriptThread.setScriptThreadPriority("HighestPriority");


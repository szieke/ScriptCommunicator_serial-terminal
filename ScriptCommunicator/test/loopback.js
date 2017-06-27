

function stopScript() 
{
    scriptThread.appendTextToConsole("loopback script has been stopped");
}

function dataReceived(data)
{
	scriptThread.sendDataArray(data);
}

scriptThread.appendTextToConsole('loopback script has started');
scriptThread.dataReceivedSignal.connect(dataReceived);


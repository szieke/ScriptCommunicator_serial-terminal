

function stopScript() 
{
    scriptThread.appendTextToConsole("loopback script has been stopped");
}

function dataReceived(data)
{
	scriptInf.sendDataArray(data);
}

scriptThread.appendTextToConsole('loopback script has started');
scriptInf.dataReceivedSignal.connect(dataReceived);


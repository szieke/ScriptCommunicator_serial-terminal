

function stopScript() 
{
    scriptThread.appendTextToConsole("loopback script has been stopped");
}

function dataReceived(data)
{
	g_receivedData = g_receivedData.concat(data);
	
	if((g_receivedData.length > 200) || (g_receivedData.indexOf(10) != -1))
	{
		scriptInf.sendDataArray(g_receivedData);
		g_receivedData = Array();
	}

}

var g_receivedData = Array();
scriptThread.appendTextToConsole('loopback script has started');
scriptInf.dataReceivedSignal.connect(dataReceived);


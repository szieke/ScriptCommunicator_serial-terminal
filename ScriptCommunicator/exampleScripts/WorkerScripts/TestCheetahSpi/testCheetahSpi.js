/*************************************************************************
This worker script (worker scripts can be added in the script window) demonstrates the usage of the 
cheetah SPI (master) interface.
***************************************************************************/

//Is called if this script shall be exited.
function stopScript() 
{
    scriptThread.appendTextToConsole("script test cheetah spi stopped");
}

scriptThread.appendTextToConsole('script test cheetah spi started');

var interface = scriptThread.createCheetahSpiInterface() ;
if(interface.connect(0, 1, 12000))
{	var data = Array(0xD7,0xff,0xff,0xff,0xff);
	if(interface.sendReceiveData(data, 0))
	{
		var receivedData = interface.readAll();
		scriptThread.appendTextToConsole("data received:  "  + receivedData);
	}
	else
	{
		scriptThread.messageBox("Critical", "error", 'sending failed');
	}
	interface.disconnect();
}
else
{
	scriptThread.messageBox("Critical", "error", 'could not connect to interface');
}

scriptThread.stopScript()



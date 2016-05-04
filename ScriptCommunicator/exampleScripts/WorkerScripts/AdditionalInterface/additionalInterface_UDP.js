

//Is called if this script shall be exited.
function stopScript() 
{
	//Remove the entry.
	scriptThread.getGlobalUnsignedNumber("UDP_INTERFACE_INSTANCE_" + instanceNumber, true);
    scriptThread.appendTextToConsole("script has been stopped");
}

//Is called if the dialog is closed.
function UI_DialogFinished(e)
{
	scriptThread.stopScript()
}

function ConnectButtonPressed()
{
	if(!isConnected)
	{	
		if(udpSocket.bind(UI_socketOwnPort.text()))
		{
			if(scriptThread.registerForSendDataFromMainInterface(true))
			{
				isConnected = true;
				UI_connectButton.setText("disconnect");
				UI_socketDestinationPort.setEnabled(false);
				UI_socketDestinationAddress.setEnabled(false);
				UI_socketOwnPort.setEnabled(false);
			}
			else
			{
				udpSocket.close();
				scriptThread.messageBox("Critical", "error", "registerForSendDataFromMainInterface failed"); 
			}
		}
		else
		{
			scriptThread.messageBox("Critical", "error", "could not bind socket to port: " + UI_socketOwnPort.text()); 
		}
	}
	else
	{
		udpSocket.close();
		isConnected = false;
		UI_connectButton.setText("connect");
		UI_socketDestinationPort.setEnabled(true);
		UI_socketDestinationAddress.setEnabled(true);
		UI_socketOwnPort.setEnabled(true);
		scriptThread.registerForSendDataFromMainInterface(false);
	}
}

function dataReceived()
{
	scriptThread.sendReceivedDataToMainInterface(udpSocket.readAll());
}

function sendDataFromMainInterface(data)
{
	var result = false;
	if(udpSocket.write(data, UI_socketDestinationAddress.text() , UI_socketDestinationPort.text()) == data.length)
	{
		result = true;
	}
	else
	{
		ConnectButtonPressed();
	}
	
	return result;
}

//Hide the dialog (the tab will be removed from the dialog therefore the dialog is not needed).
UI_Dialog.hide();

scriptThread.appendTextToConsole('script has started');
UI_Dialog.finishedSignal.connect(UI_DialogFinished);

var instanceNumber = 1;

do
{
	var resultArray = scriptThread.getGlobalUnsignedNumber("UDP_INTERFACE_INSTANCE_" + instanceNumber);
	if(resultArray[0] == 0)
	{
		scriptThread.setGlobalUnsignedNumber("UDP_INTERFACE_INSTANCE_" + instanceNumber , instanceNumber);
	}
	else
	{
		instanceNumber++;
	}
}while(resultArray[0] == 1)



UI_ToolBox.setItemText(0, "UDP " + instanceNumber);
scriptThread.addToolBoxPagesToMainWindow(UI_ToolBox);

var udpSocket = scriptThread.createUdpSocket()
udpSocket.readyReadSignal.connect(dataReceived);

var isConnected = false;
UI_connectButton.clickedSignal.connect(ConnectButtonPressed);





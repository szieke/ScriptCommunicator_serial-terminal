//Is called if the user has pressed the connect button.
function ConnectButtonPressed()
{
	if(!isConnected)
	{	
		if(udpSocket.bind(UI_SocketOwnPort.text()))
		{
			isConnected = true;
			UI_ConnectButton.setText("disconnect");
			UI_SocketDestinationPort.setEnabled(false);
			UI_SocketDestinationAddress.setEnabled(false);
			UI_SocketOwnPort.setEnabled(false);
		}
		else
		{
			scriptThread.messageBox("Critical", "error", "could not bind socket to port: " + UI_SocketOwnPort.text()); 
		}
	}
	else
	{
		udpSocket.close();
		isConnected = false;
		UI_ConnectButton.setText("connect");
		UI_SocketDestinationPort.setEnabled(true);
		UI_SocketDestinationAddress.setEnabled(true);
		UI_SocketOwnPort.setEnabled(true);
	}
}
//Is called if the additional interface has received data.
function dataReceivedAdditionalInterface()
{
	var data  = udpSocket.readAll();
	
	if(isConnected)
	{
		if(UI_ShowAscii.isChecked())
		{
			g_consoleData += UI_TextEdit.replaceNonHtmlChars(scriptThread.byteArrayToString(data).replace("\r\n", "\n"));
		}
		else if(UI_ShowHex.isChecked())
		{
			g_consoleData += scriptThread.byteArrayToHexString(data);
		}
		if(UI_SendToMainInterface.isChecked())
		{
			scriptThread.sendReceivedDataToMainInterface(data);
		}
	}
}

//Is called if the main interface shall send data.
function sendDataFromMainInterface(data)
{
	if(isConnected)
	{
		//Send the data with the additional interface.
		if(udpSocket.write(data, UI_SocketDestinationAddress.text() , UI_SocketDestinationPort.text()) != data.length)
		{
			//Call ConnectButtonPressed to diconnect the additional interface.
			ConnectButtonPressed();
			UI_TextEdit.append("UDP " + instanceNumber, "udpSocket.write failed");
		}
		else
		{
			if(UI_ShowAscii.isChecked())
			{
				//Add the data to the save console data.
				g_consoleData += "<span style=\"color:#ff0000;\">"  + UI_TextEdit.replaceNonHtmlChars(scriptThread.byteArrayToString(data)) + "</span>";
			}
			else if(UI_ShowHex.isChecked())
			{
				//Add the data to the save console data.
				g_consoleData += "<span style=\"color:#ff0000;\">"  + scriptThread.byteArrayToHexString(data) + "</span>";
			}
		}
	}
	else
	{
		UI_TextEdit.append("sending failed: interface is not connected");
	}
}


//Is called if the user has pressed the clear button.
function CleartButtonPressed()
{
	UI_TextEdit.clear();
}

//Hide the dialog (the tab will be removed from the dialog therefore the dialog is not needed).
UI_Dialog.hide();

scriptThread.appendTextToConsole('script has started');

//The instance number of the current script.
var instanceNumber = 1;

//Get the insstance number of the current script.
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


var g_consoleData = "";
var g_instanceName = "UDP " + instanceNumber;
var g_settingsFileName = "uiSettings_" + g_instanceName +".txt";
g_settingsFileName = g_settingsFileName.replace(/ /g, '_');

//Load the script with the helper functions.
scriptThread.loadScript("helper.js");

//Load the saved settings.
loadUiSettings();

//Check the version of ScriptCommunicator.
checkVersion();

UI_Dialog.finishedSignal.connect(UI_DialogFinished);

UI_ToolBox.setItemText(0, g_instanceName);
scriptThread.addToolBoxPagesToMainWindow(UI_ToolBox);

UI_TabWidget.setTabText(0, g_instanceName);
scriptThread.addTabsToMainWindow(UI_TabWidget)
scriptThread.sendDataFromMainInterfaceSignal.connect(sendDataFromMainInterface)

var udpSocket = scriptThread.createUdpSocket()
udpSocket.readyReadSignal.connect(dataReceivedAdditionalInterface);

var isConnected = false;
UI_ConnectButton.clickedSignal.connect(ConnectButtonPressed);
UI_ClearButton.clickedSignal.connect(CleartButtonPressed);

var consoleTimer = scriptThread.createTimer()
consoleTimer.timeout.connect(updateConsole);
consoleTimer.start(200);





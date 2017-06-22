/***************************************************************************************
This script send all data which shall be sent with the main interface to an additional UDP socket.
All data which has been received with the additional UDP socket will be sent to the main interface
(this data will be added to the standard consoles, the logs and worker scripts can received this data 
via scriptInf.dataReceivedSignal) .
****************************************************************************************/


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
		if(UI_ShowAscii.isChecked() || UI_ShowHex.isChecked())
		{
			addConsoleData(data, true);
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
			UI_TextEdit.append("UDP " + instanceNumber + " udpSocket.write failed");
		}
		else
		{
			if(UI_ShowAscii.isChecked() || UI_ShowHex.isChecked())
			{
				addConsoleData(data, false);
			}
		}
	}
	else
	{
		UI_TextEdit.append("sending failed: interface is not connected");
	}
}

//Loads the saved user interface settings.
function loadUiSettings()
{
	if(scriptFile.checkFileExists(g_settingsFileName))
	{
		var settings = scriptFile.readFile(g_settingsFileName);
		var stringArray = settings.split("\r\n");
		
		UI_SocketOwnPort.setText(getValueOfStringArray(stringArray, "UI_SocketOwnPort"));
		UI_SocketDestinationAddress.setText(getValueOfStringArray(stringArray, "UI_SocketDestinationAddress"));
		UI_SocketDestinationPort.setText(getValueOfStringArray(stringArray, "UI_SocketDestinationPort"));
		UI_ShowAscii.setChecked(getValueOfStringArray(stringArray, "UI_ShowAscii") == 'true');
		UI_ShowHex.setChecked(getValueOfStringArray(stringArray, "UI_ShowHex") == 'true');
		UI_ShowNothing.setChecked(getValueOfStringArray(stringArray, "UI_ShowNothing") == 'true');
		UI_SendToMainInterface.setChecked(getValueOfStringArray(stringArray, "UI_SendToMainInterface") == 'true');
	}	
}

//Saves the user interface settings.
function saveUiSettings()
{
	var settings = "";
	
	try
	{
		settings += "UI_SocketOwnPort=" + UI_SocketOwnPort.text() + "\r\n";
		settings += "UI_SocketDestinationAddress=" + UI_SocketDestinationAddress.text() + "\r\n";
		settings += "UI_SocketDestinationPort=" + UI_SocketDestinationPort.text() + "\r\n";

		settings += "UI_ShowAscii=" + UI_ShowAscii.isChecked() + "\r\n";
		settings += "UI_ShowHex=" + UI_ShowHex.isChecked() + "\r\n";
		settings += "UI_ShowNothing=" + UI_ShowNothing.isChecked() + "\r\n";
		settings += "UI_SendToMainInterface=" + UI_SendToMainInterface.isChecked() + "\r\n";
		
		scriptFile.writeFile(g_settingsFileName, true, settings, true);
	}
	catch(e)
	{
		scriptThread.messageBox("Critical", "exception in saveUiSettings", e.toString());
	}
}



//Hide the dialog (the tab will be removed from the dialog therefore the dialog is not needed).
UI_Dialog.hide();

scriptThread.appendTextToConsole('script has started');

//The instance number of the current script.
var instanceNumber = 1;
var g_prefix = "TCP";

//Get the insstance number of the current script.
do
{
	var resultArray = scriptThread.getGlobalUnsignedNumber(g_prefix + "_INTERFACE_INSTANCE_" + instanceNumber);
	if(resultArray[0] == 0)
	{
		scriptThread.setGlobalUnsignedNumber(g_prefix + "_INTERFACE_INSTANCE_" + instanceNumber , instanceNumber);
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
scriptThread.loadScript("../Common/helper.js");

//Load the saved settings.
loadUiSettings();

//Check the version of ScriptCommunicator.
checkVersion();

UI_Dialog.finishedSignal.connect(UI_DialogFinished);

UI_ToolBox.setItemText(0, g_instanceName);
scriptThread.addToolBoxPagesToMainWindow(UI_ToolBox);

UI_TabWidget.setTabText(0, g_instanceName);
scriptThread.addTabsToMainWindow(UI_TabWidget)
scriptInf.sendDataFromMainInterfaceSignal.connect(sendDataFromMainInterface)

var udpSocket = scriptInf.createUdpSocket()
udpSocket.readyReadSignal.connect(dataReceivedAdditionalInterface);

var isConnected = false;
UI_ConnectButton.clickedSignal.connect(ConnectButtonPressed);
UI_ClearButton.clickedSignal.connect(ClearButtonPressed);

var consoleTimer = scriptThread.createTimer()
consoleTimer.timeoutSignal.connect(updateConsole);
consoleTimer.start(200);





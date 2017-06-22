/***************************************************************************************
This script send all data which shall be sent with the main interface to an additional TCP client.
All data which has been received with the additional TCP client will be sent to the main interface
(this data will be added to the standard consoles, the logs and worker scripts can received this data 
via scriptInf.dataReceivedSignal) .
****************************************************************************************/


//Is called if the user has pressed the connect button.
function ConnectButtonPressed()
{
	if(!isConnected)
	{	
			UI_ConnectButton.setText("disconnect");
			UI_SocketDestinationPort.setEnabled(false);
			UI_SocketDestinationAddress.setEnabled(false);
		
			tcpClient = scriptInf.createTcpClient();
			tcpClient.disconnectedSignal.connect(additionalInterfaceDisconnected);
			tcpClient.connectedSignal.connect(additionalInterfaceConnected);
			tcpClient.errorSignal.connect(tcpClientError);
			tcpClient.readyReadSignal.connect(dataReceivedAdditionalInterface);

			UI_TextEdit.insertHtml("<br>connecting to host");
			
			tcpClient.connectToHost(UI_SocketDestinationAddress.text() , UI_SocketDestinationPort.text());
	}
	else
	{
		additionalInterfaceDisconnected();
	}
}

function additionalInterfaceDisconnected()
{
	isConnected = false;
	
	isConnected = false;
	UI_ConnectButton.setText("connect");
	UI_SocketDestinationPort.setEnabled(true);
	UI_SocketDestinationAddress.setEnabled(true);
	
	lastTcpClientErrorString = tcpClient.getErrorString();
	tcpClient.disconnectedSignal.disconnect(additionalInterfaceDisconnected);
	tcpClient.errorSignal.disconnect(tcpClientError);

	tcpClient.close();
	
	//Delete the TCP client.
	tcpClient  = undefined;
}
function additionalInterfaceConnected()
{
	isConnected = true;
	UI_TextEdit.insertHtml("<br>interface is connected");
}

//An error has been ocurred.
function tcpClientError(error)
{
	if(error != 1)
	{//The error is not QAbstractSocket::RemoteHostClosedError.
		additionalInterfaceDisconnected();
	}

	if(error == 0) //QAbstractSocket::ConnectionRefusedError
	{
		scriptThread.messageBox("Critical", "TCP error",
								 "The connection was refused. " +
									"Make sure the server is running, " +
									"and check that the host name and port " +
									"settings are correct.");
	}
	else if(error == 1) //QAbstractSocket::RemoteHostClosedError
	{
		//The connection has been closed. Do nothing.
	}
	else if(error == 2) //QAbstractSocket::HostNotFoundError
	{
		scriptThread.messageBox("Critical", "TCP error",
								 "The server was not found. Please check the " +
									"host name and port settings.");
	}
	else
	{
		scriptThread.messageBox("Critical", "TCP error",
								 "The following error occurred: " +
								 lastTcpClientErrorString);
	}
    
}

//Is called if the additional interface has received data.
function dataReceivedAdditionalInterface()
{
	var data  = tcpClient.readAll();
	
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
		if(tcpClient.write(data, UI_SocketDestinationAddress.text() , UI_SocketDestinationPort.text()) != data.length)
		{
			//Call ConnectButtonPressed to diconnect the additional interface.
			ConnectButtonPressed();
			UI_TextEdit.append("TCP " + instanceNumber + " tcpClient.write failed");
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

var lastTcpClientErrorString = "";
var g_instanceName = "TCP " + instanceNumber;
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

var tcpClient = undefined;

var isConnected = false;
UI_ConnectButton.clickedSignal.connect(ConnectButtonPressed);
UI_ClearButton.clickedSignal.connect(ClearButtonPressed);

var consoleTimer = scriptThread.createTimer()
consoleTimer.timeoutSignal.connect(updateConsole);
consoleTimer.start(200);





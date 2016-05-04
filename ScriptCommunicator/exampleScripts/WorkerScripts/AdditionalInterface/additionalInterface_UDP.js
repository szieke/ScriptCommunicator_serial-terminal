

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
			isConnected = true;
			UI_ConnectButton.setText("disconnect");
			UI_socketDestinationPort.setEnabled(false);
			UI_socketDestinationAddress.setEnabled(false);
			UI_socketOwnPort.setEnabled(false);

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
		UI_ConnectButton.setText("connect");
		UI_socketDestinationPort.setEnabled(true);
		UI_socketDestinationAddress.setEnabled(true);
		UI_socketOwnPort.setEnabled(true);
	}
}

function dataReceived()
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

function sendDataFromMainInterface(data)
{
	if(isConnected)
	{
		if(udpSocket.write(data, UI_socketDestinationAddress.text() , UI_socketDestinationPort.text()) != data.length)
		{
			ConnectButtonPressed();
			UI_TextEdit.append("UDP " + instanceNumber, "udpSocket.write failed");
		}
		else
		{
			if(UI_ShowAscii.isChecked())
			{
				g_consoleData += "<span style=\"color:#ff0000;\">"  + UI_TextEdit.replaceNonHtmlChars(scriptThread.byteArrayToString(data)) + "</span>";
			}
			else if(UI_ShowHex.isChecked())
			{
				g_consoleData += "<span style=\"color:#ff0000;\">"  + scriptThread.byteArrayToHexString(data) + "</span>";
			}
		}
	}
	else
	{
		UI_TextEdit.append("sending failed: interface is not connected");
	}
}

function updateConsole()
{
	if(g_consoleData.length > 0)
	{
		if(g_consoleData.length > 50000)
		{
			g_consoleData = g_consoleData.substring(g_consoleData.length - 30000);
			UI_TextEdit.clear();
		}
			
		var list = g_consoleData.split("<br>")
		if(list.length == 1)
		{
			UI_TextEdit.insertHtml(g_consoleData);
		}
		else
		{
			UI_TextEdit.insertHtml(list[0]);
		
			for(var i = 1; i < list.length; i++)
			{
				UI_TextEdit.append(list[i]);
			}
		}
		g_consoleData = "";
	}

}

function CleartButtonPressed()
{
	UI_TextEdit.clear();
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


var g_consoleData = "";


UI_ToolBox.setItemText(0, "UDP " + instanceNumber);
scriptThread.addToolBoxPagesToMainWindow(UI_ToolBox);

UI_TabWidget.setTabText(0, "UDP " + instanceNumber);
scriptThread.addTabsToMainWindow(UI_TabWidget)
scriptThread.sendDataFromMainInterfaceSignal.connect(sendDataFromMainInterface)

var udpSocket = scriptThread.createUdpSocket()
udpSocket.readyReadSignal.connect(dataReceived);

var isConnected = false;
UI_ConnectButton.clickedSignal.connect(ConnectButtonPressed);
UI_ClearButton.clickedSignal.connect(CleartButtonPressed);

var consoleTimer = scriptThread.createTimer()
consoleTimer.timeout.connect(updateConsole);
consoleTimer.start(200);





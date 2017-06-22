/***************************************************************************************
This script send all data which shall be sent with the main interface to an additional serial port.
All data which has been received with the additional serial port will be sent to the main interface
(this data will be added to the standard consoles, the logs and worker scripts can received this data 
via scriptInf.dataReceivedSignal) .
****************************************************************************************/

//Is called if the user has pressed the connect button.
function ConnectButtonPressed()
{
	if(!isConnected)
	{	
		serialPort.setDTR(true);
		serialPort.setPortName(UI_SerialPortInfoListBox.currentText());
		if (serialPort.open())
		{
			if (serialPort.setBaudRate(Number(UI_BaudRateBox.currentText()))
			 && serialPort.setDataBits(Number(UI_DataBitsBox.currentText()))
			 && serialPort.setParity(UI_ParityBox.currentText())
			 && serialPort.setStopBits(UI_StopBitsBox.currentText())
			 && serialPort.setFlowControl(UI_FlowControlBox.currentText()))
			 {
				isConnected = true;
				UI_ConnectButton.setText("disconnect");

				serialPort.setRTS(true);
	
				UI_BaudRateBox.setEnabled(false);
				UI_DataBitsBox.setEnabled(false);
				UI_ParityBox.setEnabled(false);
				UI_StopBitsBox.setEnabled(false);
				UI_FlowControlBox.setEnabled(false);
				UI_SerialPortInfoListBox.setEnabled(false);
			 }
		}
		else
		{
			serialPort.close();
			isConnectedSecondInterface = false;
			scriptThread.messageBox("Critical", 'serial port open error', 'could not open serial port: ' + UI_SerialPortInfoListBox.currentText())
		}
	}
	else
	{
		serialPort.close();
		isConnected = false;
		UI_ConnectButton.setText("connect");
		UI_BaudRateBox.setEnabled(true);
		UI_DataBitsBox.setEnabled(true);
		UI_ParityBox.setEnabled(true);
		UI_StopBitsBox.setEnabled(true);
		UI_FlowControlBox.setEnabled(true);
		UI_SerialPortInfoListBox.setEnabled(true);
	}
}
//Is called if the additional interface has received data.
function dataReceivedAdditionalInterface()
{
	var data  = serialPort.readAll();
	
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
		if(serialPort.write(data) != data.length)
		{
			//Call ConnectButtonPressed to diconnect the additional interface.
			ConnectButtonPressed();
			UI_TextEdit.append("Serial " + instanceNumber + " serialPort.write failed");
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
		
		UI_SerialPortInfoListBox.setCurrentText(getValueOfStringArray(stringArray, "UI_SerialPortInfoListBox"));
		UI_BaudRateBox.setCurrentText(getValueOfStringArray(stringArray, "UI_BaudRateBox"));
		UI_DataBitsBox.setCurrentText(getValueOfStringArray(stringArray, "UI_DataBitsBox"));
		UI_StopBitsBox.setCurrentText(getValueOfStringArray(stringArray, "UI_StopBitsBox"));
		UI_ParityBox.setCurrentText(getValueOfStringArray(stringArray, "UI_ParityBox"));
		UI_FlowControlBox.setCurrentText(getValueOfStringArray(stringArray, "UI_FlowControlBox"));
	
	}	
}

//Saves the user interface settings.
function saveUiSettings()
{
	var settings = "";
	
	try
	{
		settings += "UI_SerialPortInfoListBox=" + UI_SerialPortInfoListBox.currentText() + "\r\n";
		settings += "UI_BaudRateBox=" + UI_BaudRateBox.currentText() + "\r\n";
		settings += "UI_DataBitsBox=" + UI_DataBitsBox.currentText() + "\r\n";
		settings += "UI_StopBitsBox=" + UI_StopBitsBox.currentText() + "\r\n";
		settings += "UI_ParityBox=" + UI_ParityBox.currentText() + "\r\n";
		settings += "UI_FlowControlBox=" + UI_FlowControlBox.currentText() + "\r\n";

		
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
var g_prefix = "Serial";

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
var g_instanceName = "Serial " + instanceNumber;
var g_settingsFileName = "uiSettings_" + g_instanceName +".txt";
g_settingsFileName = g_settingsFileName.replace(/ /g, '_');

//Load the script with the helper functions.
scriptThread.loadScript("../Common/helper.js");

//Add the available serial ports.
var availablePorts = scriptInf.availableSerialPorts();
for(var i = 0; i < availablePorts.length; i++)
{
	UI_SerialPortInfoListBox.addItem(availablePorts[i]);
}


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

var serialPort = scriptInf.createSerialPort();
serialPort.readyReadSignal.connect(dataReceivedAdditionalInterface);

var isConnected = false;
UI_ConnectButton.clickedSignal.connect(ConnectButtonPressed);
UI_ClearButton.clickedSignal.connect(ClearButtonPressed);

var consoleTimer = scriptThread.createTimer()
consoleTimer.timeoutSignal.connect(updateConsole);
consoleTimer.start(200);





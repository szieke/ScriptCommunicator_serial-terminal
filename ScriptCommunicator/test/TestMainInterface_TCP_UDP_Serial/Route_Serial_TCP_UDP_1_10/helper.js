/*************************************************************************
This script file contains helper function for routeSerialTcpUdp.js
***************************************************************************/

//Is called if the user has pressed the clear button.
function UI_clearConsoleButtonClicked()
{
	UI_scriptConsole.clear();
}
function stopScript() 
{
	saveUiSettings();
	scriptThread.appendTextToConsole("script stopped");
}

//Append data at the end of the console.
function appendDataToConsole(text)
{
	UI_scriptConsole.insertHtml(text);
}

//Is called if the user closes the user interface.
function UI_DialogFinished(e)
{
	//Stop this script.
	scriptThread.stopScript()
}

//Is called if the user clickes a check box.
function checkBoxClicked()
{
	textFromGuiElementChanged("");
}
//Is called if a text from a GUI element has been changed.
function textFromGuiElementChanged(text)
{
	
	UI_noProxyRadioButton.setEnabled(false);
	UI_useSystemProxyRadioButton.setEnabled(false);
	UI_useSpecificProxyRadioButton.setEnabled(false);
	UI_proxyAddressLineEdit.setEnabled(false);
	UI_proxyPortLineEdit.setEnabled(false);
	UI_proxyUserNameLineEdit.setEnabled(false);
	UI_proxyPasswordLineEdit.setEnabled(false);
	UI_disconnectButton.setText("disconnect");
	
	if(UI_connectionTypeComboBox.currentText() == "socket")
	{
		UI_baudRateBox.setEnabled(false);
		UI_dataBitsBox.setEnabled(false);
		UI_parityBox.setEnabled(false);
		UI_stopBitsBox.setEnabled(false);
		UI_flowControlBox.setEnabled(false);
		UI_serialPortInfoListBox.setEnabled(false);

		UI_socketsTypeComboBox.setEnabled(true);

		if(UI_socketsTypeComboBox.currentText() == "UDP socket")
		{
			UI_socketDestinationPort.setEnabled(true);
			UI_socketDestinationAddress.setEnabled(true);
			UI_socketOwnPort.setEnabled(true);
		}
		else if(UI_socketsTypeComboBox.currentText() == "TCP server")
		{
			UI_socketOwnPort.setEnabled(true);
			UI_socketDestinationPort.setEnabled(false);
			UI_socketDestinationAddress.setEnabled(false);
		}
		else
		{//TCP client
			UI_socketOwnPort.setEnabled(false);
			UI_socketDestinationPort.setEnabled(true);
			UI_socketDestinationAddress.setEnabled(true);
			
			UI_noProxyRadioButton.setEnabled(true);
			UI_useSystemProxyRadioButton.setEnabled(true);
			UI_useSpecificProxyRadioButton.setEnabled(true);
			
			if(UI_useSystemProxyRadioButton.isChecked())
			{
				UI_proxyUserNameLineEdit.setEnabled(true);
				UI_proxyPasswordLineEdit.setEnabled(true);
			}
			if(UI_useSpecificProxyRadioButton.isChecked())
			{
				UI_proxyAddressLineEdit.setEnabled(true);
				UI_proxyPortLineEdit.setEnabled(true);
				UI_proxyUserNameLineEdit.setEnabled(true);
				UI_proxyPasswordLineEdit.setEnabled(true);
			}
		}
	}
	else
	{
		UI_baudRateBox.setEnabled(true);
		UI_dataBitsBox.setEnabled(true);
		UI_parityBox.setEnabled(true);
		UI_stopBitsBox.setEnabled(true);
		UI_flowControlBox.setEnabled(true);
		UI_serialPortInfoListBox.setEnabled(true);

		UI_socketsTypeComboBox.setEnabled(false);
		UI_socketDestinationPort.setEnabled(false);
		UI_socketDestinationAddress.setEnabled(false);
		UI_socketOwnPort.setEnabled(false);
		
	}
	
}

//Loads the saved user interface settings.
function loadUiSettings()
{
	if(scriptThread.checkFileExists("uiSettings.txt"))
	{
		var settings = scriptThread.readFile("uiSettings.txt");
		var stringArray = settings.split("\r\n");
		
		UI_dataBitsBox.setCurrentText(getValueOfStringArray(stringArray, "UI_dataBitsBox"));
		UI_parityBox.setCurrentText(getValueOfStringArray(stringArray, "UI_parityBox"));
		UI_baudRateBox.setCurrentText(getValueOfStringArray(stringArray, "UI_baudRateBox"));
		UI_stopBitsBox.setCurrentText(getValueOfStringArray(stringArray, "UI_stopBitsBox"));
		UI_flowControlBox.setCurrentText(getValueOfStringArray(stringArray, "UI_flowControlBox"));
		UI_serialPortInfoListBox.setCurrentText(getValueOfStringArray(stringArray, "UI_serialPortInfoListBox"));
		
		UI_socketsTypeComboBox.setCurrentText(getValueOfStringArray(stringArray, "UI_socketsTypeComboBox"));
		UI_socketOwnPort.setText(getValueOfStringArray(stringArray, "UI_socketOwnPort"));
		UI_socketDestinationPort.setText(getValueOfStringArray(stringArray, "UI_socketDestinationPort"));
		UI_socketDestinationAddress.setText(getValueOfStringArray(stringArray, "UI_socketDestinationAddress"));
		
		UI_connectionTypeComboBox.setCurrentText(getValueOfStringArray(stringArray, "UI_connectionTypeComboBox"));
		
		UI_noProxyRadioButton.setChecked(getValueOfStringArray(stringArray, "UI_noProxyRadioButton") == 'true');
		UI_useSystemProxyRadioButton.setChecked(getValueOfStringArray(stringArray, "UI_useSystemProxyRadioButton") == 'true');
		UI_useSpecificProxyRadioButton.setChecked(getValueOfStringArray(stringArray, "UI_useSpecificProxyRadioButton") == 'true');
		UI_proxyAddressLineEdit.setText(getValueOfStringArray(stringArray, "UI_proxyAddressLineEdit"));
		UI_proxyPortLineEdit.setText(getValueOfStringArray(stringArray, "UI_proxyPortLineEdit"));
		UI_proxyUserNameLineEdit.setText(getValueOfStringArray(stringArray, "UI_proxyUserNameLineEdit"));
		UI_proxyPasswordLineEdit.setText(getValueOfStringArray(stringArray, "UI_proxyPasswordLineEdit"));
		
		
		textFromGuiElementChanged("");
	}	
}

//Saves the user interface settings.
function saveUiSettings()
{
	var settings;
	
	try
	{
		settings = "UI_dataBitsBox=" + UI_dataBitsBox.currentText() + "\r\n";
		
		settings += "UI_parityBox=" + UI_parityBox.currentText() + "\r\n";
		settings += "UI_baudRateBox=" + UI_baudRateBox.currentText() + "\r\n";
		settings += "UI_stopBitsBox=" + UI_stopBitsBox.currentText() + "\r\n";
		settings += "UI_flowControlBox=" + UI_flowControlBox.currentText() + "\r\n";
		settings += "UI_serialPortInfoListBox=" + UI_serialPortInfoListBox.currentText() + "\r\n";


		settings += "UI_socketsTypeComboBox=" + UI_socketsTypeComboBox.currentText() + "\r\n";
		settings += "UI_socketOwnPort=" + UI_socketOwnPort.text() + "\r\n";
		settings += "UI_socketDestinationPort=" + UI_socketDestinationPort.text() + "\r\n";
		settings += "UI_socketDestinationAddress=" + UI_socketDestinationAddress.text() + "\r\n";
		
		settings += "UI_connectionTypeComboBox=" + UI_connectionTypeComboBox.currentText() + "\r\n";
		
		settings += "UI_noProxyRadioButton=" + UI_noProxyRadioButton.isChecked() + "\r\n";
		settings += "UI_useSystemProxyRadioButton=" + UI_useSystemProxyRadioButton.isChecked() + "\r\n";
		settings += "UI_useSpecificProxyRadioButton=" + UI_useSpecificProxyRadioButton.isChecked() + "\r\n";
		settings += "UI_proxyAddressLineEdit=" + UI_proxyAddressLineEdit.text() + "\r\n";
		settings += "UI_proxyPortLineEdit=" + UI_proxyPortLineEdit.text() + "\r\n";
		settings += "UI_proxyUserNameLineEdit=" + UI_proxyUserNameLineEdit.text() + "\r\n";
		settings += "UI_proxyPasswordLineEdit=" + UI_proxyPasswordLineEdit.text() + "\r\n";
		
		scriptThread.writeFile("uiSettings.txt", true, settings, true);
	}
	catch(e)
	{
		scriptThread.messageBox("Critical", "exception in saveUiSettings", e.toString());
	}
}

//Returns a value from stringArray (values are stored in a key/value string,
//eg. 'UI_dataBitsBox=8')
function getValueOfStringArray(stringArray, key)
{
	for (var i=0; i < stringArray.length; i++)
	{
		var subStringArray = stringArray[i].split("=");
		if(subStringArray[0] == key)
		{
			return subStringArray[1];
		}
	}
}

//Checks the version of ScriptCommunicator.
function checkVersion()
{
	var versionIsOk = false;
	try
	{
		versionIsOk = scriptThread.checkScriptCommunicatorVersion("04.11");
	}
	catch(e)
	{
	}
	
	if(!versionIsOk)
	{
		scriptThread.messageBox("Critical", "version is to old",
								 "The current version of ScriptCommunicator is to old to execute this script.\n" +
									"The latest version of ScriptCommunicator can be found here:\n" +
									"http://sourceforge.net/projects/scriptcommunicator/");						
		scriptThread.stopScript();
	}
}




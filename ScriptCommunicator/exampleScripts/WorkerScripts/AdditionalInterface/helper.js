/*************************************************************************
This script file contains helper function for the additionl interface scripts.
***************************************************************************/

//Is called if this script shall be exited.
function stopScript() 
{
	//Remove the entry.
	scriptThread.getGlobalUnsignedNumber("UDP_INTERFACE_INSTANCE_" + instanceNumber, true);
	saveUiSettings();
	scriptThread.appendTextToConsole("script stopped");
}

//Append data at the end of the console.
function updateConsole()
{
	if(g_consoleData.length > 0)
	{
		//Limit the number of characters.
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
		{//If the data contains a new line then one insertHtml and several append are used.
		  //This is done because this is much faster the one insertHtml call.
			
			UI_TextEdit.insertHtml(list[0]);
		
			for(var i = 1; i < list.length; i++)
			{
				UI_TextEdit.append(list[i]);
			}
		}
		g_consoleData = "";
	}

}

//Is called if the user closes the user interface.
function UI_DialogFinished()
{
	//Stop this script.
	scriptThread.stopScript()
}

//Loads the saved user interface settings.
function loadUiSettings()
{
	if(scriptThread.checkFileExists(g_settingsFileName))
	{
		var settings = scriptThread.readFile(g_settingsFileName);
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
		
		scriptThread.writeFile(g_settingsFileName, true, settings, true);
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
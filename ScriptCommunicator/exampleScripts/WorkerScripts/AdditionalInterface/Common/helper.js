/*************************************************************************
This script file contains helper function for the additionl interface scripts.
***************************************************************************/

//Is called if the user has pressed the clear button.
function ClearButtonPressed()
{
	UI_TextEdit.clear();
}

//Is called if this script shall be exited.
function stopScript() 
{
	//Remove the entry.
	scriptThread.getGlobalUnsignedNumber(g_prefix + "_INTERFACE_INSTANCE_" + instanceNumber, true);
	saveUiSettings();
	scriptThread.appendTextToConsole("script stopped");
}


//Is called if the user closes the user interface.
function UI_DialogFinished()
{
	//Stop this script.
	scriptThread.stopScript()
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

function addConsoleData(data, received)
{
	if(g_consoleData.length == 0)
	{
		g_consoleData[0] = Array(data, received);
	}
	else
	{
		if(g_consoleData[g_consoleData.length - 1][1] == received)
		{
			g_consoleData[g_consoleData.length - 1][0]  = g_consoleData[g_consoleData.length - 1] [0].concat(data);
		}
		else
		{
			g_consoleData[g_consoleData.length] = Array(Array(), true);
			g_consoleData[g_consoleData.length - 1][0]  = data;
			g_consoleData[g_consoleData.length - 1][1]  = received;
		}
	}
	
	g_bytesInConsoleData += data.length;
	
	if(g_bytesInConsoleData > 50000)
	{
		g_clearConsole = true;
	}
	
	while(g_bytesInConsoleData > 50000 && g_consoleData.length > 0)
	{
		if(g_consoleData[0][0].length > 50000)
		{
			g_bytesInConsoleData -= g_consoleData[0][0].length - 50000;
			g_consoleData[0][0].splice(0, g_consoleData[0][0].length - 50000);
		}
		else
		{
			g_bytesInConsoleData -= g_consoleData[0][0].length;
			g_consoleData.splice(0, 1);
		}

	}
	
}


//Append data at the end of the console.
function updateConsole()
{
	if(g_bytesInConsoleData > 0)
	{
		var consoleString = "";
		
		for(var i = 0; i < g_consoleData.length; i++)
		{
				if(g_consoleData[i][1] )
				{
					consoleString += "<span style=\"color:#00ff00;\">";
				}
				else
				{
					consoleString += "<span style=\"color:#ff0000;\">";
				}
				if(UI_ShowAscii.isChecked())
				{
					consoleString = UI_TextEdit.replaceNonHtmlChars(conv.byteArrayToString(g_consoleData[i][0]).replace(/\r\n/g, "\n")) ;
				}
				else
				{
					consoleString = scriptThread.byteArrayToHexString(g_consoleData[i][0]);
				}
				
				
				if(g_consoleData[i][1] )
				{
					consoleString = consoleString.replace(/<br>/g, "</span><br><span style=\"color:#00ff00;\">");
				}
				else
				{
					consoleString = consoleString.replace(/<br>/g, "</span><br><span style=\"color:#ff0000;\">");
				}
				consoleString += "</span>";
		}
		
		if(g_clearConsole)
		{
			UI_TextEdit.clear();
			g_clearConsole = false;
		}
		

		var list = consoleString.split("<br>");
		
		UI_TextEdit.insertHtml(list[0]);
		
		for(var i = 1; i < list.length; i++)
		{
			UI_TextEdit.append(list[i]);
		}
	
		g_consoleData  = Array();
		g_bytesInConsoleData = 0;
	}

}

var g_consoleData  = Array();
var g_bytesInConsoleData = 0;
var g_clearConsole = false;

	
	
	
	
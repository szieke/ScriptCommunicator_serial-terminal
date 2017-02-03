/***************************************************************************************
This worker script (worker scripts can be added in the script window) shows 
all recieved data (main interface) in a utf8 console.
****************************************************************************************/

//The Lock button in the main window has been pressed.
function mainWindowLockScrollingClicked(isChecked)
{
	UI_TextEdit1.lockScrolling(isChecked);
}
//The Clear button in the main window has been pressed.
function mainWindowClearConsoleClicked()
{
	UI_TextEdit1.clear();
}
//Reads the global console settings (settings dialog) and adjusts the utf8 console to it.
function readConsoleSetting(isFirstCall)
{

	//get the console settings
	g_settings = scriptThread.getConsoleSettings();

	if(isFirstCall != undefined)
	{//readConcoleSetting is not called by the cyclic timer.
		
		//background color.
		UI_TextEdit1.setPaletteColorRgb(parseInt(g_settings.backgroundColor.substring(0,2), 16), 
		parseInt(g_settings.backgroundColor.substring(2,4), 16), parseInt(g_settings.backgroundColor.substring(4,0), 16), "Base");
		
		//text color.
		UI_TextEdit1.setPaletteColorRgb(parseInt(g_settings.receiveColor.substring(0,2), 16), 
		parseInt(g_settings.receiveColor.substring(2,4), 16), parseInt(g_settings.receiveColor.substring(4,0), 16), "Text");
	}
}


function addDataToConsole(data, fontColor)
{
	var receivedString  = "";
	if(g_settings["generateCyclicTimeStamps"])
	{
		if((Date.now() - g_timeLastTimestamp) >= g_settings["timeStampInterval"])
		{
			g_timeLastTimestamp = Date.now();
			
			receivedString += "<span style=\"font-family:'"+ g_settings["font"];
			receivedString += "';font-size:" + g_settings["fontSize"]+ "pt;color:#" + g_settings["timeStampColor"] + "\">";
			receivedString += UI_TextEdit1.replaceNonHtmlChars(scriptThread.getTimestamp(), false);
			receivedString += "</span>";
		}
	}
	
	receivedString += "<span style=\"font-family:'"+ g_settings["font"];
	receivedString += "';font-size:" + g_settings["fontSize"]+ "pt;color:#" + fontColor + "\">";
	
	var tmpString = UI_TextEdit1.replaceNonHtmlChars(conv.byteArrayToUtf8String(data), false);
	
	
	if(g_settings["createNewLineAtByte"])
	{
		//Replace the new line bytes.
		var newLineAtByte = String.fromCharCode(g_settings["newLineAtByte"]);
		while(tmpString.indexOf(newLineAtByte) != -1)
		{
			tmpString = tmpString.replace(newLineAtByte, "<br>");
		}
	}
	
	receivedString += tmpString;
	receivedString += "</span>";	
	

	
	UI_TextEdit1.insertHtml(receivedString);	
}
//The main interface has sent data.
function dataSendSlot(data)
{
	if(g_settings["showSendData"])
	{
		addDataToConsole(data, g_settings["sendColor"]);
	}
}

//The main interface has received data.
function dataReceivedSlot(data)
{
	if(g_settings["showReceivedData"])
	{
		addDataToConsole(data, g_settings["receiveColor"]);
	}
}

//The console settings.
var g_settings = scriptThread.getConsoleSettings();


//The time at which the the last timestamp has been created.
var g_timeLastTimestamp = Date.now();

//Hide the dialog (the tab will be removed from the dialog therefore the dialog is not needed).
UI_Dialog.hide();

//Remove the tab from the dialog and add it to the main window.
scriptThread.addTabsToMainWindow(UI_TabWidget)

scriptThread.dataReceivedSignal.connect(dataReceivedSlot);
scriptThread.mainWindowLockScrollingClickedSignal.connect(mainWindowLockScrollingClicked);
scriptThread.mainWindowClearConsoleClickedSignal.connect(mainWindowClearConsoleClicked);
scriptThread.sendDataFromMainInterfaceSignal.connect(dataSendSlot);
readConsoleSetting(true);

var settingsTimer = scriptThread.createTimer();
settingsTimer.timeoutSignal.connect(readConsoleSetting);
settingsTimer.start(2000);


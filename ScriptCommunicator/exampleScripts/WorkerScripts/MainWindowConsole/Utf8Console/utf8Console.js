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
function readConsoleSetting()
{

	//get the console settings
	g_settings = scriptThread.getConsoleSettings();

	if(g_saveBackgroundColor != g_settings.backgroundColor)
	{
		g_saveBackgroundColor = g_settings.backgroundColor;
		
		UI_TextEdit1.setPaletteColorRgb(parseInt(g_settings.backgroundColor.slice(0,2), 16), 
		parseInt(g_settings.backgroundColor.slice(2,4), 16), parseInt(g_settings.backgroundColor.slice(4), 16), "Base");

	}
}

function addDataToConsole(data, fontColor)
{
	var stringToAdd = "<span style=\"font-family:'"+ g_settings.font;
	stringToAdd += "';font-size:" + g_settings.fontSize + "pt;color:#" + fontColor + "\">";
	
	if(g_settings.generateCyclicTimeStamps)
	{
		if((Date.now() - g_timeLastTimestamp) >= g_settings.timeStampInterval)
		{
			g_timeLastTimestamp = Date.now();
			stringToAdd += UI_TextEdit1.replaceNonHtmlChars(scriptThread.getTimestamp(), true);
		}
	}
	

	var newLineAtByte = String.fromCharCode(g_settings.newLineAtByte);
	
	//Replace all HTML characters (all but '\n').
	stringToAdd += UI_TextEdit1.replaceNonHtmlChars(conv.byteArrayToUtf8String(data), false);
	
	if(g_settings.createNewLineAtByte)
	{
		
		//Replace the new line bytes.
		stringToAdd = stringToAdd.replace(RegExp(newLineAtByte, 'g'), "<br>" + newLineAtByte)

	}
	
	if(g_settings.ceateTimestampAtByte)
	{
		var timeAtByte = "\n"//String.fromCharCode(g_settings.timestampAtByte);
		
		var list = stringToAdd.split(timeAtByte);
		stringToAdd = "";
		for(var i = 0; i < list.length; i++)
		{
			stringToAdd += list[i] ;
			
			if(i < (list.length - 1))
			{
				stringToAdd += UI_TextEdit1.replaceNonHtmlChars(scriptThread.getTimestamp(), true);
			}
		}
		
	}
	
	if(g_settings.createNewLineAtByte)
	{
		stringToAdd = stringToAdd.replace(RegExp(newLineAtByte, 'g'), "")
	}
	

	
	UI_TextEdit1.insertHtml(stringToAdd);	
	

}
//The main interface has sent data.
function dataSendSlot(data)
{
	if(g_settings.showSendData)
	{
		addDataToConsole(data, g_settings.sendColor);
	}
}

//The main interface has received data.
function dataReceivedSlot(data)
{
	if(g_settings.showReceivedData)
	{
		addDataToConsole(data, g_settings.receiveColor);
	}
}

//The console settings.
var g_settings = scriptThread.getConsoleSettings();

UI_TextEdit1.setMaxChars(1000000);

//The time at which the the last timestamp has been created.
var g_timeLastTimestamp = Date.now();

//Hide the dialog (the tab will be removed from the dialog therefore the dialog is not needed).
UI_Dialog.hide();

//Remove the tab from the dialog and add it to the main window.
scriptThread.addTabsToMainWindow(UI_TabWidget)

scriptInf.dataReceivedSignal.connect(dataReceivedSlot);
scriptThread.mainWindowLockScrollingClickedSignal.connect(mainWindowLockScrollingClicked);
scriptThread.mainWindowClearConsoleClickedSignal.connect(mainWindowClearConsoleClicked);
scriptInf.sendDataFromMainInterfaceSignal.connect(dataSendSlot);

var g_saveBackgroundColor = "";
readConsoleSetting();

var settingsTimer = scriptThread.createTimer();
settingsTimer.timeoutSignal.connect(readConsoleSetting);
settingsTimer.start(2000);


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
	g_storedReceivedData = Array();
	g_storedSendData = Array();
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



function addDataToConsole(data, fontColor, isSentData)
{
	var resultString  = "<span style=\"font-family:'"+ g_settings.font;
	resultString += "';font-size:" + g_settings.fontSize + "pt;color:#" + fontColor + "\">";
	var arrayLength= 0;
	var timeStamp = scriptThread.getTimestamp();
	
	if(isSentData)
	{
		g_storedSendData = g_storedSendData.concat(data);
		arrayLength = g_storedSendData.length;
	}
	else
	{
		g_storedReceivedData = g_storedReceivedData.concat(data);
		arrayLength = g_storedReceivedData.length;
	}
	
	//After 5 bytes have been received or sent the packet table is created.
	while(arrayLength>=5)
	{
		var packet = Array();
		if(isSentData)
		{
			arrayLength = g_storedSendData.length;
			packet = g_storedSendData.splice(0, 5);
		}
		else
		{
			arrayLength = g_storedReceivedData.length;
			packet = g_storedReceivedData.splice(0, 5);
		}
		
		resultString += '<TABLE ALIGN=CENTER WIDTH="50%" BORDER=1 CELLSPACING=10 CELLPADDING=3><CAPTION><p style="color:blue">Five-byte Packet ('+timeStamp+')</p></CAPTION><TR><TD> <p style="color:green">0b'
		+packet[0].toString(2)+'</p></TD> <TD> '
		+packet[1]+'</TD> <TD> '+packet[2]+'</TD><TD> '
		+packet[3]+'</TD> <TD> <p style="color:red"> 0x'
		+packet[4].toString(16).toUpperCase()+'</p></TD> </TR></TABLE> <BR>';
		
		if(isSentData)
		{
			arrayLength = g_storedSendData.length;
		}
		else
		{
			arrayLength = g_storedReceivedData.length;
		}
		
	}
	
	resultString += "</span>";	
	
	UI_TextEdit1.append(resultString);	
}
//The main interface has sent data.
function dataSendSlot(data)
{
	if(g_settings.showSendData)
	{
		addDataToConsole(data, g_settings.sendColor, true);
	}
}

//The main interface has received data.
function dataReceivedSlot(data)
{
	if(g_settings.showReceivedData)
	{
		addDataToConsole(data, g_settings.receiveColor, false);
	}
}

//The console settings.
var g_settings = scriptThread.getConsoleSettings();

var g_storedReceivedData = Array();
var g_storedSendData = Array();

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
UI_TextEdit1.setMaxChars(1000000);

var settingsTimer = scriptThread.createTimer();
settingsTimer.timeoutSignal.connect(readConsoleSetting);
settingsTimer.start(2000);


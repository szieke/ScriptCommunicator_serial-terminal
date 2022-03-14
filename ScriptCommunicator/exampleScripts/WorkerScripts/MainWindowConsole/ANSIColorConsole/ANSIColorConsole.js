/***************************************************************************************
This worker script (worker scripts can be added in the script window) shows 
all recieved data (main interface) in a ANSI console.
****************************************************************************************/

// Color regexes:
const RGX_Color_Default = (/\u001b\[0m/);
const RGX_Color_BLACK 	= (/\u001b\[0;30m/);
const RGX_Color_RED 	= (/\u001b\[0;31m/);
const RGX_Color_GREEN 	= (/\u001b\[0;32m/);
const RGX_Color_YELLOW 	= (/\u001b\[0;33m/);
const RGX_Color_BLUE 	= (/\u001b\[0;34m/);
const RGX_Color_PURPLE 	= (/\u001b\[0;35m/);
const RGX_Color_CYAN 	= (/\u001b\[0;36m/);
const RGX_Color_WHITE 	= (/\u001b\[0;37m/);

var g_textColor = "000000";
var g_tmpBuf = "";


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
		stringToAdd = stringToAdd.replace(newLineAtByte, "<br>" + newLineAtByte)
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
	
	// Apply ANSI Esc Filter. This matches most of the ANSI escape codes, beyond just colors, 
	// 	including the extended VT100 codes, archaic/proprietary printer codes, etc.
	// Regex source: https://stackoverflow.com/questions/25245716/remove-all-ansi-colors-styles-from-strings
	stringToAdd = stringToAdd.replace(/[\u001b\u009b][[()#;?]*(?:[0-9]{1,4}(?:;[0-9]{0,4})*)?[0-9A-ORZcf-nqry=><]/g, "");		
	
	if(g_settings.createNewLineAtByte)
	{
		stringToAdd = stringToAdd.replace(newLineAtByte, "");
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
		// First, convert array to string so we can work with them:
		var stringData = conv.byteArrayToString(data);
		
		// If there are some leftover data from previous run, add them:
		if(g_tmpBuf != "") stringData = g_tmpBuf + stringData;
		g_tmpBuf = ""; 	// Clear the buffer after that, not needed anymore
		
		// Split string to array of strings separated by ECS character (033, u001B) 
		// split() deletes the split character, so it has to be added back later.
		var stringArray = stringData.split(/\u001b/g);		
		//var idx = stringData.search(/[\u001b\u009b][[()#;?]*(?:[0-9]{1,4}(?:;[0-9]{0,4})*)?[0-9A-ORZcf-nqry=><]/g, "");
		
		// Print first element of array (data before first escape char), using stored color from previous run:
		addDataToConsole(conv.stringToArray(stringArray[0]), g_textColor);

		// Process following elements:
		for(var i=1; i<stringArray.length; i++) 
		{
			// Put the ESC character back (if split was done by ESC):
			stringArray[i] = "\033" + stringArray[i];
			
			var len = stringArray[i].length;
			// Check length of last element and if not correct, save it for later and skip.		
			// If last element: 
			if (i == (stringArray.length - 1)) 
			{
				// ESC+[0m -> 4chars (color reset to default); ESC+[0;3xm -> 7 chars (basic color set)	
				
				// If (shorter than 4 chars) OR (longer than 4 but shorter than 7) OR (equals 4 but not ESC+[0m):
				if ( (len < 4) || ((len > 4) && (len < 7)) || ((len == 4) && (stringArray[i] != "\033[0m")) ) 
				{
					// But also skip for ESC+[0m with newline character/s:
					if( (stringArray[i] != "\033[0m\n") || (stringArray[i] != "\033[0m\r\n") ) 
					{		   
						g_tmpBuf = stringArray[i];	// Store current string for next run:
						break;	// Break from for-loop now
					}
				}	
			}

			// Now we can search for possible color setting ANSI sequence.
			// Not the best, but does the job (basic colors only):	
			if (RGX_Color_Default.test(stringArray[i])) 	g_textColor = "000000";
			else if (RGX_Color_BLACK.test(stringArray[i])) 	g_textColor = "000000";
			else if (RGX_Color_RED.test(stringArray[i])) 	g_textColor = "AA0000";
			else if (RGX_Color_GREEN.test(stringArray[i])) 	g_textColor = "00AA00";
			else if (RGX_Color_YELLOW.test(stringArray[i])) g_textColor = "AAAA00";
			else if (RGX_Color_BLUE.test(stringArray[i])) 	g_textColor = "0000AA";
			else if (RGX_Color_PURPLE.test(stringArray[i])) g_textColor = "AA00AA";
			else if (RGX_Color_CYAN.test(stringArray[i])) 	g_textColor = "00AAAA";
			else if (RGX_Color_WHITE.test(stringArray[i])) 	g_textColor = "AAAAAA";
			else g_textColor = "000000"
			
			// Convert and print processed string data back to array so main console can show it:
			addDataToConsole(conv.stringToArray(stringArray[i]), g_textColor);				
		}
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


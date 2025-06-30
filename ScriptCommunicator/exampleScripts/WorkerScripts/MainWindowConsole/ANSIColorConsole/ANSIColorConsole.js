/***************************************************************************************
This worker script (worker scripts can be added in the script window) shows 
all recieved data (main interface) in a ANSI console.
****************************************************************************************/

// Textcolor regexes:
const RGX_Color_Default = (/\u001b\[([0-9]{1,4};)*0m/);
const RGX_Color_BLACK 	= (/\u001b\[([0-9]{1,4};)*30m/);
const RGX_Color_RED 	= (/\u001b\[([0-9]{1,4};)*31m/);
const RGX_Color_GREEN 	= (/\u001b\[([0-9]{1,4};)*32m/);
const RGX_Color_YELLOW 	= (/\u001b\[([0-9]{1,4};)*33m/);
const RGX_Color_BLUE 	= (/\u001b\[([0-9]{1,4};)*34m/);
const RGX_Color_PURPLE 	= (/\u001b\[([0-9]{1,4};)*35m/);
const RGX_Color_CYAN 	= (/\u001b\[([0-9]{1,4};)*36m/);
const RGX_Color_WHITE 	= (/\u001b\[([0-9]{1,4};)*37m/);
const RGX_Color_GREY 	= (/\u001b\[([0-9]{1,4};)*39m/);


//The Lock button in the main window has been pressed.
function mainWindowLockScrollingClicked(isChecked)
{
	UI_TextEdit1.lockScrolling(isChecked);
}

//The Clear button in the main window has been pressed.
function mainWindowClearConsoleClicked()
{
	UI_TextEdit1.blockSignals(true);
	UI_TextEdit1.clear();
	g_currentConsoleConent = "";
	UI_TextEdit1.blockSignals(false);
}


function addDataToConsole(data, fontColor)
{
	
	var stringToAdd = "<span style=\"font-family:'"+ g_font;
	stringToAdd += "';font-size:" + g_fontSize + "pt;color:#" + fontColor + "\">";
	
	if(g_generateCyclicTimeStamps)
	{
		if((Date.now() - g_timeLastTimestamp) >= g_timeStampInterval)
		{
			g_timeLastTimestamp = Date.now();
			stringToAdd += UI_TextEdit1.replaceNonHtmlChars(scriptThread.getTimestamp(), true);
		}
	}

	
	//Replace all HTML characters (all but '\n').
	stringToAdd += UI_TextEdit1.replaceNonHtmlChars(conv.byteArrayToUtf8String(data), false);
	
	//Replace the new line bytes.
	stringToAdd = stringToAdd.replace(RegExp(g_newLineAtByte, 'g'), "<br>")
	
	
	// Apply ANSI Esc Filter. This matches most of the ANSI escape codes, beyond just colors, 
	// 	including the extended VT100 codes, archaic/proprietary printer codes, etc.
	// Regex source: https://stackoverflow.com/questions/25245716/remove-all-ansi-colors-styles-from-strings
	stringToAdd = stringToAdd.replace(/[\u001b\u009b][[()#;?]*(?:[0-9]{1,4}(?:;[0-9]{0,4})*)?[0-9A-ORZcf-nqry=><]/g, "");		
	
	stringToAdd = stringToAdd.replace(RegExp(g_newLineAtByte, 'g'), "")

	UI_TextEdit1.blockSignals(true);
	UI_TextEdit1.insertHtml(stringToAdd);	
	g_currentConsoleConent = UI_TextEdit1.toPlainText();
	UI_TextEdit1.blockSignals(false);
}

function processStringData(data)
{
	
	// First, convert array to string so we can work with them:
	var stringData = conv.byteArrayToString(data);
	
	//Ignore control code
	stringData = stringData.replace(/\u001b\u005b4P/g,"");
	stringData = stringData.replace(/\u001b\u005bK/g,"");
	stringData = stringData.replace(/\u001b\u005bA/g,"");
	
	//Remove terminal bell.
	stringData = stringData.replace(/\u0007/g,"");
	
	
	// If there are some leftover data from previous run, add them:
	if(g_storedRxData != "") stringData = g_storedRxData + stringData;
	g_storedRxData = ""; 	// Clear the buffer after that, not needed anymore
	
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
		stringArray[i] = "\u001b" + stringArray[i];
		
		var len = stringArray[i].length;
		// Check length of last element and if not correct, save it for later and skip.		
		// If last element: 
		if (i == (stringArray.length - 1)) 
		{
			// ESC+[0m -> 4chars (color reset to default); ESC+[0;3xm -> 7 chars (basic color set)	
			
			// If (shorter than 4 chars) OR (longer than 4 but shorter than 7) OR (equals 4 but not ESC+[0m):
			if ( (len < 4) || ((len > 4) && (len < 7)) || ((len == 4) && (stringArray[i] != "\u001b[0m")) ) 
			{
				// But also skip for ESC+[0m with newline character/s:
				if( (stringArray[i] != "\u001b[0m\n") || (stringArray[i] != "\u001b[0m\r\n") ) 
				{		   
					g_storedRxData = stringArray[i];	// Store current string for next run:
					break;	// Break from for-loop now
				}
			}	
		}
		

		// Now we can search for possible color setting ANSI sequence.
		// Not the best, but does the job (basic colors only):	
		if (RGX_Color_Default.test(stringArray[i])) 	g_textColor = "EEEEEE";
		else if (RGX_Color_BLACK.test(stringArray[i])) 	g_textColor = "000000";
		else if (RGX_Color_RED.test(stringArray[i])) 	g_textColor = "AA0000";
		else if (RGX_Color_GREEN.test(stringArray[i])) 	g_textColor = "00AA00";
		else if (RGX_Color_YELLOW.test(stringArray[i])) g_textColor = "AAAA00";
		else if (RGX_Color_BLUE.test(stringArray[i])) 	g_textColor = "0000AA";
		else if (RGX_Color_PURPLE.test(stringArray[i])) g_textColor = "AA00AA";
		else if (RGX_Color_CYAN.test(stringArray[i])) 	g_textColor = "00AAAA";
		else if (RGX_Color_WHITE.test(stringArray[i])) 	g_textColor = "EEEEEE";
		else if (RGX_Color_GREY.test(stringArray[i])) 	g_textColor = "A9A9A9";
		else g_textColor = "EEEEEE"
		

		
		// Convert and print processed string data back to array so main console can show it:
		addDataToConsole(conv.stringToArray(stringArray[i]), g_textColor);				
	}
}
//The main interface has received data.
function dataReceivedSlot(data)
{
	g_receivedData = g_receivedData.concat(data);
}

//Process the received data.
function processTimerSlot()
{
	var pos;
	
	if(g_receivedData.length == 0)
	{
		return;
	}
	

	
	g_processTimer.stop();
	
	var data = g_receivedData;
	g_receivedData = Array();
	
	pos = data.indexOf(0x8);
	while(pos!= -1)
	{
		if(pos != 0)
		{
			processStringData(data.slice(0, pos));
		}
	
		if((data[pos + 1] == 0x20) && (data[pos + 2] == 0x8)) 
		{//Backspace key pressed.
			
			//Remove the last character.
			UI_TextEdit1.blockSignals(true);
			UI_TextEdit1.deleteLastCharacters(1);
			g_currentConsoleConent = UI_TextEdit1.toPlainText();
			UI_TextEdit1.blockSignals(false);
			
			data = data.slice(pos + 3, data.length);
		}
		else
		{//Remove the last character.
			
			UI_TextEdit1.blockSignals(true);
			UI_TextEdit1.deleteLastCharacters(1);
			g_currentConsoleConent = UI_TextEdit1.toPlainText();
			UI_TextEdit1.blockSignals(false);
			
			data = data.slice(pos + 1, data.length);
		}
		
		pos = data.indexOf(0x8);
	}
	
	pos = data.indexOf(13);
	while(pos!= -1)
	{//Carriage return found.
		
		if(pos != 0)
		{
			processStringData(data.slice(0, pos));
		}
		
		data = data.slice(pos + 1, data.length);

		if(data[0] != 10)
		{//No new line after the carriage return.

			if(data.length == 0)
			{//Carriage return is the last character
				
				UI_TextEdit1.blockSignals(true);
				UI_TextEdit1.deleteLastLine()
				g_currentConsoleConent = UI_TextEdit1.toPlainText();
				UI_TextEdit1.blockSignals(false);
			}
		}
	
		pos = data.indexOf(13);
	}
	
	if(data.length == 0)
	{
		g_processTimer.start(g_timerInterval);
		return
	}
			
	processStringData(data);
	g_processTimer.start(g_timerInterval);
}

//Key was pressed while the console had the focus.
function keyPressedSlot(key, ctrlModifier, text)
{
	const ShiftModifier        = 0x02000000;
    const ControlModifier      = 0x04000000;
    const AltModifier          = 0x08000000;
	const Key_Left = 0x01000012;
	const Key_Up = 0x01000013;
	const Key_Right = 0x01000014;
	const Key_Down = 0x01000015;
	
	var data = Array();
	
	if(text == "")
	{
		data.push(0x1b);
        data.push(0x5b);
		
		if(key == Key_Left)
		{//\u001b[D 
			data.push(0x44);
		}
		else if(key== Key_Up)
        {//\u001b[A
            data.push(0x41);
        }
        else if(key== Key_Right)
        {//\u001b[C
            data.push(0x43);
        }
        else if(key== Key_Down)
        {//\u001b[B
            data.push(0x42);
        }
		else
		{
			data = Array();
		}
	}
	else
	{
		data = conv.stringToUtf8Array(text);
	}
	
	
	UI_TextEdit1.moveTextPositionToEnd();
	if(data.length != 0)
	{
		scriptInf.sendDataArray(data);
	}
}

//The text of the console has been changed (only possible with 'paste' via the console context menu). 
function consoleTextChangedSlot()
{
	var currentText = UI_TextEdit1.toPlainText();
	var charCount = currentText.length - g_currentConsoleConent.length;
	var stringToSend = currentText.slice(currentText.length - charCount, currentText.length);
	
	//Remove the pasted characters.
	UI_TextEdit1.blockSignals(true);
	UI_TextEdit1.deleteLastCharacters(charCount);
	UI_TextEdit1.blockSignals(false);
	
	//Send the pasted characters.
	scriptInf.sendString(stringToSend);
	
}

//Hide the dialog (the tab will be removed from the dialog therefore the dialog is not needed).
UI_Dialog.hide();

//Remove the tab from the dialog and add it to the main window.
scriptThread.addTabsToMainWindow(UI_TabWidget)

//Setup the console.
UI_TextEdit1.setMaxChars(50000);
UI_TextEdit1.keyPressedSignal.connect(keyPressedSlot);
UI_TextEdit1.textChangedSignal.connect(consoleTextChangedSlot);
UI_TextEdit1.addKeyFilter();
UI_TextEdit1.setUpdateRate(200);
var g_currentConsoleConent = "";

scriptInf.dataReceivedSignal.connect(dataReceivedSlot);
scriptThread.mainWindowLockScrollingClickedSignal.connect(mainWindowLockScrollingClicked);
scriptThread.mainWindowClearConsoleClickedSignal.connect(mainWindowClearConsoleClicked);

//The time at which the the last timestamp has been created.
var g_timeLastTimestamp = Date.now();

//The time stamp interval.
var g_timeStampInterval = 1000;

//True if a cyclic timestamp shall be generated.
var g_generateCyclicTimeStamps = false;

//The console text color.
var g_textColor = "EEEEEE";

//The stored recieved data.
var g_storedRxData = "";

//A new line is created in the console when this byte is received.
var g_newLineAtByte = "\n";

//The console font.
var g_font = "Courier New";

//The console font size.
var g_fontSize = 12;


var g_timerInterval = 100;
var g_processTimer = scriptThread.createTimer();
g_processTimer.timeoutSignal.connect(processTimerSlot);
g_receivedData = Array();
g_processTimer.start(g_timerInterval);







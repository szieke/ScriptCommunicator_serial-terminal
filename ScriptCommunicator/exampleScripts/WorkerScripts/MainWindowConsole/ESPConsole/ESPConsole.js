/***************************************************************************************
This worker script (worker scripts can be added in the script window) shows 
all recieved data (main interface) in a ESPConsole and can automatically 
decode backtrace if configured correctly. 

See README.md for more details.
Requres ScriptCommunicator v6+ (getUserGenericConfigFolder())
****************************************************************************************/

var VERSION_INFO = "ESP Console v1.1.1 (30.03.2023)";

// Load additional scripts and UI
scriptThread.loadScript("runProcessAsync.js");
scriptThread.loadScript("backtraceSettings.js");

// ESP32 specific color regexes:
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
var g_tmpBuf = "";		// Temporary buffer for unfinished ANSI data
var g_addrChunk = "";	// Temporary buffer for incomplete addreses to decode

//The Lock button in the main window has been pressed.
function mainWindowLockScrollingClicked(isChecked)
{
	UI_TextEdit1.lockScrolling(isChecked);
}

//The Clear button in the main window has been pressed.
function mainWindowClearConsoleClicked()
{
	UI_TextEdit1.clear();
	UI_label_backtrace.setText("Decoding backtrace for ESP Project:");
	UI_label_backtrace.setWindowTextColor("darkGray");	// Reasonable default that works on dark and light themes
	// Also refresh FW info if available
	if( UI_chkBox_backtraceDecode.isChecked() ) {
		checkReadelf(UI_comBox_projecElfFile.currentText());
	}
}

//Reads the global console settings (settings dialog) and adjusts the utf8 console to it.
function readConsoleSetting()
{
	//get the console settings
	g_settings = scriptThread.getConsoleSettings();

	if(g_saveBackgroundColor != g_settings.backgroundColor) {
		g_saveBackgroundColor = g_settings.backgroundColor;
		
		UI_TextEdit1.setPaletteColorRgb(parseInt(g_settings.backgroundColor.slice(0,2), 16), 
		parseInt(g_settings.backgroundColor.slice(2,4), 16), parseInt(g_settings.backgroundColor.slice(4), 16), "Base");
	}
}

function addStrToConsole(strData, fontColor)
{
	var stringToAdd = "<span style=\"font-family:'"+ g_settings.font;
	stringToAdd += "';font-size:" + g_settings.fontSize + "pt;color:#" + fontColor + "\">";
	
	if(g_settings.generateCyclicTimeStamps) {
		if((Date.now() - g_timeLastTimestamp) >= g_settings.timeStampInterval) {
			g_timeLastTimestamp = Date.now();
			stringToAdd += UI_TextEdit1.replaceNonHtmlChars(scriptThread.getTimestamp(), true);
		}
	}
	
	var newLineAtByte = String.fromCharCode(g_settings.newLineAtByte);
	
	//Replace all HTML characters (all but '\n').
	stringToAdd += UI_TextEdit1.replaceNonHtmlChars(/*conv.byteArrayToUtf8String*/(strData), false);
	
	if(g_settings.createNewLineAtByte) {
		// Replace the new line bytes for <br>, but keep newline for timestamp
		stringToAdd = stringToAdd.replace(RegExp(newLineAtByte, 'g'), "<br>" + newLineAtByte)
	}
	
	if(g_settings.ceateTimestampAtByte) {
		var timeAtByte = String.fromCharCode(g_settings.timestampAtByte);
		var list = stringToAdd.split(timeAtByte);
		stringToAdd = "";
		for(var i = 0; i < list.length; i++)
		{
			stringToAdd += list[i] ;
			if(i < (list.length - 1)) {
				stringToAdd += UI_TextEdit1.replaceNonHtmlChars(scriptThread.getTimestamp(), true);
			}
		}	
	}
	
	if(g_settings.createNewLineAtByte) {
		// Clean newline bytes
		stringToAdd = stringToAdd.replace(RegExp(newLineAtByte, 'g'), "")
	}	
	
	// Apply ANSI Esc Filter. This matches most of the ANSI escape codes, beyond just colors, 
	// 	including the extended VT100 codes, archaic/proprietary printer codes, etc.
	// Regex source: https://stackoverflow.com/questions/25245716/remove-all-ansi-colors-styles-from-strings
	stringToAdd = stringToAdd.replace(/[\u001b\u009b][[()#;?]*(?:[0-9]{1,4}(?:;[0-9]{0,4})*)?[0-9A-ORZcf-nqry=><]/g, "");		
	
	UI_TextEdit1.insertHtml(stringToAdd);	
}


/* Find and decode backtrace addresses, print the output using xtensa tools.
 * Example backtrace: 
 *  Backtrace:0x12345678:0x123456780x12345678:0x12345678 0x12345678:0x12345678 0x12345678:0x12345678 0x12345678:0x12345678 0x12345678:0x00000000  |<-CORRUPTED
 */
function findAndDecodeBacktrace(backtraceString)
{
	// Basic workaround to show "backtrace detected" message even if result was not correct
	if( backtraceString.search("Backtrace:") >= 0 ) {
		UI_label_backtrace.setText("Backtrace detected!");
		UI_label_backtrace.setWindowTextColor("red");
	}
	
	// If there is some unfinished addres leftover, add it first:
	if(g_addrChunk != "") backtraceString = g_addrChunk + backtraceString;
	g_addrChunk = ""; 	// Clear the buffer after that, not needed anymore
	// Create arrray of strings for backtrace addresses:
	var addrs = Array();	
	// Safest option would be to search for every '0x' character combination and take folowing 8 characters
	var idx = backtraceString.indexOf("0x", 0);	// from start
	// If 0x was found at all:
	while( idx >= 0 ) 
	{
		// Check how much of string is left and act accordingly
		if( backtraceString.slice(idx, idx+11).length <= 10 ) {	// 10 or less characters. Try to slice 11 characters for length test.
			// If slice is 10 or less characters, just store it and wait for next round (in case it is longer address format)
			g_addrChunk = backtraceString.slice(idx, idx+10);		
			idx = -1;	// Wont loop again
		}
		else {
			// More than 10 charcters
			if( backtraceString[idx+10] == ':' ) 	// Address is followed by comma (11th character = index+10)
			{
				// In this case complete slice must have 21 characters instead of just 10
				if( backtraceString.slice(idx, idx+21).length < 21 ) {
					// Not enough, store it and wait for next round
					g_addrChunk = backtraceString.slice(idx, idx+21);
					idx = -1;	// Wont loop again
				}
				else {
					// Long address, enough characters. Store address and search for next 0x
					addrs.push( backtraceString.slice(idx, idx+21) ); 	// Add address:address to array of strings
					idx = backtraceString.indexOf("0x", idx+21);		// Start next search after previous end
				}
			}
			else {	
				// More than 10 with no comma after addres
				addrs.push( backtraceString.slice(idx, idx+10) ); 	// Add address to array of strings
				idx = backtraceString.indexOf("0x", idx+10);		// Start next search after previous end
			}
		}
	}

	var program = UI_lnEd_pathAddr2Line.text();	
	var elfFile = UI_comBox_projecElfFile.currentText();
	// Run xtensa addr2line command for every address: xtensa-esp32-elf-addr2line -pfiaC -e build/PROJECT.elf ADDRESS
	for(var i = 0; i < addrs.length; i++)
	{
		var arguments = Array("-pfiaC", "-e", ""+elfFile+"", addrs[i]);
		var ret = runProcessAsync(program, arguments, 1000, 1000, "");		
		if(ret.exitCode != 0) { 	// Error
			addStrToConsole(ret.stdErr, "CC0000");	// red
		}
		else {	
			// OK, but also filer out invalid results. Unfortunately no string.includes("") function?
			if(	UI_chkBox_invalidResultsEnable.isChecked() ) {
				// No filter, print everything that was decoded
				addStrToConsole("\r\n" + ret.stdOut, "FF0000");	// red
				UI_label_backtrace.setText("Backtrace detected!");
				UI_label_backtrace.setWindowTextColor("red");
			}
			else {
				if( ((/[?][?][ ][?][?][:][0]/).test(ret.stdOut) || (/[?][?][:][?]/).test(ret.stdOut)) != true ) 
				{
					if( (/call_start_cpu/).test(ret.stdOut) == true ) {	
						addStrToConsole("\r\n" + ret.stdOut, "AAAA00");	// Not an eror, print this one in yellow
					}
					else {	// May be backtrace
						addStrToConsole("\r\n" + ret.stdOut, "FF0000");	// red
						UI_label_backtrace.setText("Backtrace detected!");
						UI_label_backtrace.setWindowTextColor("red");
					}
				}
			}
		}
	}
}

//The main interface has sent data.
function dataSendSlot(data)
{
	if(g_settings.showSendData) {
		addStrToConsole(conv.byteArrayToUtf8String(data), g_settings.sendColor);
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
		
		// Split string to array of strings separated by ESC character (033, u001B) 
		// split() deletes the split character, so it has to be added back later.
		var stringArray = stringData.split(/\u001b/g);		
		
		// Print first element of array (data before first escape char), using stored color from previous run:
		addStrToConsole(stringArray[0], g_textColor);
		
		// Check for Backtrace addresses:
		if(UI_chkBox_backtraceDecode.isChecked()) {
			findAndDecodeBacktrace(stringArray[0]);
		}

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
				if ( (len < 4) || ((len > 4) && (len < 7)) || ((len == 4) && (stringArray[i] != "\u001b[0m")) ) {
					// But also skip for ESC+[0m with newline character/s:
					if( (stringArray[i] != "\u001b[0m\n") || (stringArray[i] != "\u001b[0m\r\n") ) {		   
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
			addStrToConsole(stringArray[i], g_textColor);
			
			// Check for Backtrace addresses:
			if(UI_chkBox_backtraceDecode.isChecked()) {
				findAndDecodeBacktrace(stringArray[i]);
			}		
		}
	}
}

// Is called if this script shall be exited.
function stopScript() 
{
	saveUiSettings();
	scriptThread.stopScript();	// This is the stopper
}

// Connect Signals
UI_btnBacktraceSettings.clickedSignal.connect(showSettingsDialog);

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

// Load settings
loadUiSettings();
UI_label_backtrace.setWindowTextColor("darkGray");	// Reasonable default that works on dark and light themes

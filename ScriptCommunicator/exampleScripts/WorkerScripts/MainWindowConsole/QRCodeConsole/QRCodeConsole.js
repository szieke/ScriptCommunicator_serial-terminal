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

function createQRCode()
{
	var resultString = "<p style='font-size:1px'>";
	var HTMLContent = "";
	
	for(var u = 0; u < QRCodeArray.length-1; u++)
	{
		for(var i = 0; i < QRCodeArray[u].length-1; i++)
		{
			var bitArray = numToBinaryArray( parseInt( QRCodeArray[u][i] ) );
		  
			for(var y = 0; y < bitArray.length; y++)
			{
				if ( bitArray[y] == "0" ) 
				{//The currentbit is 0.
					HTMLContent = "<img src='"+imageFolder+"0.gif'>";
			 
				} 				
				else
				{//The current bit is 1.
					HTMLContent = "<img src='"+imageFolder+"1.gif'>";
				}
				resultString += HTMLContent;
			} 
	   
		}
		resultString += "<BR>";
	}
	
	UI_TextEdit1.append(resultString);	
}


//The console settings.
var g_settings = scriptThread.getConsoleSettings();

scriptThread.loadScript("QRCodeHelper.js");

var imageFolder = scriptFile.getScriptFolder() + "/Media/"
var QRCode = scriptFile.readFile("QRData.txt");
var QRCodeArray =  CSVToArray(QRCode);


//The time at which the the last timestamp has been created.
var g_timeLastTimestamp = Date.now();

//Hide the dialog (the tab will be removed from the dialog therefore the dialog is not needed).
UI_Dialog.hide();

//Remove the tab from the dialog and add it to the main window.
scriptThread.addTabsToMainWindow(UI_TabWidget)

scriptThread.mainWindowLockScrollingClickedSignal.connect(mainWindowLockScrollingClicked);
scriptThread.mainWindowClearConsoleClickedSignal.connect(mainWindowClearConsoleClicked);

var g_saveBackgroundColor = "";
readConsoleSetting();

var settingsTimer = scriptThread.createTimer();
settingsTimer.timeoutSignal.connect(readConsoleSetting);
settingsTimer.start(2000);

UI_TextEdit1.setMaxChars(1000000);
createQRCode();


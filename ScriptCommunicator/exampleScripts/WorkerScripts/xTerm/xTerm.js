/*************************************************************************
This script executes a terminal which can interpret xTterm  control sequences.
Note: https://github.com/xtermjs is used.
***************************************************************************/


//Is called if this script shall be exited.
function stopScript() 
{
    scriptThread.appendTextToConsole("script has been stopped");
}

//Is called if the dialog is closed.
function dialogFinished(e)
{
	scriptThread.stopScript()
}

var g_fileName=""
function savePage()
{
		g_fileName = scriptThread.showFileDialog(true, "save terminal content", "", "");
		if(g_fileName != "")
		{//One file selected.
			if(scriptFile.writeFile(g_fileName, false, UI_WebView.evaluateJavaScript("getContent()"), true))
			{
				scriptThread.messageBox("Critical", "error", "could not write " + g_fileName);
			}
		}
}

function callWorkerScriptWithResult(value, resultObject)
{
	if(value[0] == "readData")
	{
		resultObject.setResult(receivedData);
		receivedData = ""
	}
}

function callWorkerScript(value)
{

	 if(value[0] == "sendData")
	{
		scriptInf.sendString(value[1]);
	}
}

function dataReceivedSlot(data)
{
	receivedData += conv.byteArrayToString(data);
}


scriptThread.appendTextToConsole('script has started');
UI_Dialog.finishedSignal.connect(dialogFinished);

UI_SavePushButton.clickedSignal.connect(savePage)

var receivedData = "";

scriptThread.addTabsToMainWindow(UI_tabWidget);
UI_Dialog.hide();

try
{
	UI_WebView.callWorkerScriptWithResultSignal.connect(callWorkerScriptWithResult);
	UI_WebView.callWorkerScriptSignal.connect(callWorkerScript);
	scriptInf.dataReceivedSignal.connect(dataReceivedSlot);

	//Load the terminal.
	UI_WebView.load("file:///" + scriptThread.getScriptFolder() + "/src/index.html")

	/*Set:
	* - font size to 12
	* - cursor is blinking
	* - max. lines to 10000
	* - the background color to #ffffff
	* - the foreground color to ##000000
	*/
	UI_WebView.evaluateJavaScript("setOptions(12, true, 1000, '#ffffff', '#000000')");
	
}
catch(e)
{
	scriptThread.messageBox("Error", "Missing libraries", "Missing libraries. See " + scriptFile.createAbsolutePath("readme.txt") + " for more informations.");
	scriptThread.stopScript()
}






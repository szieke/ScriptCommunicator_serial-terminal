
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

function printPage()
{
		UI_WebView.print("print page");
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

UI_PrintPushButton.clickedSignal.connect(printPage)

var receivedData = "";

scriptThread.addTabsToMainWindow(UI_tabWidget);
UI_Dialog.hide();

try
{
	UI_WebView.callWorkerScriptWithResultSignal.connect(callWorkerScriptWithResult);
	
}
catch(e)
{
	scriptThread.messageBox("Error", "Missing libraries", "See " + scriptFile.createAbsolutePath("readme.txt") + " for more informations.");
}


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
scriptThread.addMessageToLogAndConsoles(UI_WebView.evaluateJavaScript("setOptions(12, true, 10000, '#ffffff', '#000000')"));






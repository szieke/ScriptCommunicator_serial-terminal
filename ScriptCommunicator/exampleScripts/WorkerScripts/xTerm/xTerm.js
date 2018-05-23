
//Is called if this script shall be exited.
function stopScript() 
{
    scriptThread.appendTextToConsole("script has been stopped");
	//UI_WebView.evaluateJavaScript("unload()")
}

//Is called if the dialog is closed.
function dialogFinished(e)
{
	scriptThread.stopScript()
}

function loadPage()
{
	UI_WebView.load("file:///" + scriptThread.getScriptFolder() + "/src/index.html")
	
}

function loadFinished(ok)
{
	if(ok)
	{
		UI_UrlLabel.setText("url loaded: " + UI_WebView.url());
		UI_UrlLineEdit.setText(UI_WebView.url());
	}
	else
	{
		UI_UrlLabel.setText("error while loading: " + UI_UrlLineEdit.text());
	}
}

function printPage()
{
		UI_WebView.print("print page");
}

scriptThread.appendTextToConsole('script has started');
UI_Dialog.finishedSignal.connect(dialogFinished);

UI_LoadPushButton.clickedSignal.connect(loadPage)
UI_PrintPushButton.clickedSignal.connect(printPage)

var receivedData = "";

function callWorkerScriptWithResult(value, resultObject)
{
	
	
	if(value[0] == "readData")
	{
		receivedData = "ttttt\r\n"
		resultObject.setResult(receivedData);
		receivedData = ""
	}
	
}

function sendData(value)
{

	 if(value[0] == "sendArray")
	{
		scriptInf.sendString(value[1]);
	}
}

function dataReceivedSlot(data)
{
	receivedData += conv.byteArrayToString(data);
}


try
{
	UI_WebView.loadFinishedSignal.connect(loadFinished);
	
}
catch(e)
{
	scriptThread.messageBox("Error", "Missing libraries", "See " + scriptFile.createAbsolutePath("readme.txt") + " for more informations.");
}


UI_WebView.callWorkerScriptWithResultSignal.connect(callWorkerScriptWithResult);
UI_WebView.callWorkerScriptSignal.connect(sendData);
scriptInf.dataReceivedSignal.connect(dataReceivedSlot);



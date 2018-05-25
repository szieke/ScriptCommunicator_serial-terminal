
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

function loadPage()
{
	UI_WebView.load(UI_UrlLineEdit.text())
	
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


try
{
	UI_WebView.loadFinishedSignal.connect(loadFinished);
	
}
catch(e)
{
	scriptThread.messageBox("Error", "Missing libraries", "Missing libraries. See " + scriptFile.createAbsolutePath("readme.txt") + " for more informations.");
	scriptThread.stopScript()
}




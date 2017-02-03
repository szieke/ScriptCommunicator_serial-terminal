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

function dataReceivedSlot(data)
{
	
	var receivedString = conv.byteArrayToUtf8String(data)
	
	//The console is a HTML console therefore following characters must be replaced with their HTML representation.
	receivedString = receivedString.replace(/</gm, "&lt;")//replace < with &lt
	receivedString = receivedString.replace(/>/gm, "&gt;")//replace > with &gt
	receivedString = receivedString.replace(/(\r\n|\n|\r)/gm, "<br>")//replace \r\n, \n and \r with <br>
	receivedString = receivedString.replace(/ /gm, "&nbsp;")//replace space with &nbsp;
	
	UI_TextEdit1.insertHtml(receivedString);
}
scriptThread.appendTextToConsole('script has started');


//Hide the dialog (the tab will be removed from the dialog therefore the dialog is not needed).
UI_Dialog.hide();

//Remove the tab from the dialog and add it to the main window.
scriptThread.addTabsToMainWindow(UI_TabWidget)

scriptThread.dataReceivedSignal.connect(dataReceivedSlot);
scriptThread.mainWindowLockScrollingClickedSignal.connect(mainWindowLockScrollingClicked);
scriptThread.mainWindowClearConsoleClickedSignal.connect(mainWindowClearConsoleClicked);

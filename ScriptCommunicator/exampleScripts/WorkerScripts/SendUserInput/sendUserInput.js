/*This script demonstrates sending of user input*/

//Is called if this script shall be exited.
function stopScript() 
{
	scriptThread.setMainWindowAndTaskBarIcon("default.ico");
    scriptThread.appendTextToConsole("script has been stopped");
}

//The dialog is closed.
function dialogFinished(e)
{
	scriptThread.stopScript()
}

function sendSequence()
{
	var sender = this;
	scriptThread.appendTextToConsole(sender.getObjectName());
	var data = "send data: ";
	data += UI_LineEdit.text() + " ";
	data += UI_ComboBox.currentText() + " ";
	data += UI_SpinBox.value().toString() +"\n";
	
	if(!scriptInf.sendString(data))
	{
		scriptThread.messageBox("Critical", "Error", "Sending failed. Check if ScriptCommunicator is connected.");
	}
}

function testShortcut1()
{
	scriptThread.appendTextToConsole("testShortcut1");
}

function testShortcut2()
{
	scriptThread.appendTextToConsole("testShortcut2");
}

function testShortcut3()
{
	scriptThread.appendTextToConsole("testShortcut3");
}

scriptThread.appendTextToConsole('script has started');
UI_Dialog.finishedSignal.connect(dialogFinished);

UI_SendButton.clickedSignal.connect(UI_SendButton, sendSequence)

scriptThread.appendTextToConsole("serial port signals: " + scriptInf.getSerialPortSignals().toString(16));
scriptThread.setMainWindowAndTaskBarIcon("mainWindowIcon.ico");
UI_Dialog.setWindowIcon("dialogIcon.png");

//Test shortcut 1.
UI_Dialog.createShortCut("Alt+V", testShortcut1)

//Test shortcut 2.
UI_Dialog.createShortCut("F1", testShortcut2)

//To activate test shortcut 3:
//- enter Alt+Ctrl+X, release Alt+Ctrl+X
//- enter Ctrl+S, release Ctrl+S
//- enter Q
UI_Dialog.createShortCut("Alt+Ctrl+X,Ctrl+S,Q", testShortcut3)

var ports = scriptInf.availableSerialPortsExt();
for(var i = 0; i < ports.length; i++)
{
	scriptThread.appendTextToConsole("serial port info name:" + ports[i].portName + "  systemLocation:" + ports[i].systemLocation 
	+ "  description:" + ports[i].description + "  manufacturer:" + ports[i].manufacturer+ "  serialNumber:" + ports[i].serialNumber 
	+ " vendorIdentifier: " + ports[i].vendorIdentifier + "  productIdentifier:" + ports[i].productIdentifier);
}



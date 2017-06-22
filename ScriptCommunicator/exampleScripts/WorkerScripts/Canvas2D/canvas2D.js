/***************************************************************************************
This worker script (worker scripts can be added in the script window) demonstrates the
usage of the ScriptCanvas2D class.
****************************************************************************************/

//Is called if this script shall be exited.
function stopScript() 
{
    scriptThread.appendTextToConsole("script has been stopped");
}

//Is called if the dialog is closed.
function UI_DialogFinished(e)
{
	scriptThread.stopScript()
}
function saveToFile()
{
	var fileName = scriptThread.showFileDialog(true, "save to file", "",
	"BMP (*.bmp);;JPG (*.jpg);;PNG (*.png);;PBM (*.bpm);;PGM (*.pgm);;PPM (*.ppm)");
	
	if(fileName != "")
	{
		ctxClock.saveToFile(fileName);
	}
}
function print()
{
	ctxClock.print("print widget");
}

scriptThread.appendTextToConsole('script has started');
UI_Dialog.finishedSignal.connect(UI_DialogFinished);

scriptThread.loadScript("includedScripts/pacman.js");
scriptThread.loadScript("includedScripts/clock.js");
scriptThread.loadScript("includedScripts/plasma.js");
scriptThread.loadScript("includedScripts/rotate.js");

UI_SavePushButton.clickedSignal.connect(saveToFile);
UI_PrintPushButton.clickedSignal.connect(print);


ctxClock.save();
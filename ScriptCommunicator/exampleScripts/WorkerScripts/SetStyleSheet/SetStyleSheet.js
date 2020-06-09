/*************************************************************************
This worker script (worker scripts can be added in the script window) demonstrates how to set the style
sheet of a script widget.
***************************************************************************/

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


scriptThread.appendTextToConsole('script has started');
UI_Dialog.finishedSignal.connect(UI_DialogFinished);


var styleSheet = "background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:0, stop:0 rgba(131, 16, 0, 255), stop:1 rgba(227, 6, 19, 255));\n"
styleSheet += "image: url(" + scriptFile.getScriptFolder() + "/lock.ico);\n"
styleSheet += "image-position: right center;\n"
styleSheet += "background-repeat: no-repeat\n"

UI_Label.setStyleSheet(styleSheet);




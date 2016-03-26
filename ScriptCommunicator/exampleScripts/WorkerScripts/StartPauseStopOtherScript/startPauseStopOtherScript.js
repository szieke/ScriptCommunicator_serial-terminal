/*
This script demonstrates how to start, pause and stop
other script in the script-table (script-window)
*/

//Is called if this script shall be exited.
function stopScript() 
{
    scriptThread.appendTextToConsole("script has been stopped");
}

scriptThread.appendTextToConsole('script ' + scriptThread.getScriptTableName() + ' has started');

scriptThread.setScriptState(0, "send input example");//Start the script.
scriptThread.sleepFromScript(3000);

scriptThread.setScriptState(1, "send input example");//Pause the script.
scriptThread.sleepFromScript(3000);

scriptThread.setScriptState(2, "send input example");//Stop the script.

scriptThread.stopScript()



/*************************************************************************
This worker script (worker scripts can be added in the script window) demonstrates 
how to write ScriptCommunicator global worker script data.
Note: Worker scripts can exchange data with ScriptCommunicator global variables.
***************************************************************************/

//Is called by ScriptCommunicator if the script shall be stopped.
function stopScript() 
{
	scriptThread.appendTextToConsole("script data exchange receiver stopped");
	
	//Remove all global variables.
	scriptThread.getGlobalString("String", true);
	scriptThread.getGlobalDataArray("DataArray", true);
	scriptThread.getGlobalUnsignedNumber("UnsignedNumber", true);
	scriptThread.getGlobalSignedNumber("SignedNumber", true);
	scriptThread.getGlobalRealNumber("RealNumber", true);
}

function timeOut()
{
	counter++;
	scriptThread.setGlobalString("String", counter.toString());
	var array = Array();
	
	counter++;
	array[0] = counter;
	scriptThread.setGlobalDataArray("DataArray", array);
	
	counter++;
	scriptThread.setGlobalUnsignedNumber("UnsignedNumber", counter);
	
	counter++;
	scriptThread.setGlobalSignedNumber("SignedNumber", counter);
	
	counter++;
	scriptThread.setGlobalRealNumber("RealNumber", counter + 0.1);

}

var counter = 0;
scriptThread.appendTextToConsole("script data exchange receiver started");
var timer = scriptThread.createTimer()
timer.timeoutSignal.connect(timeOut)
timer.start(1000);

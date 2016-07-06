/*************************************************************************
This worker script (worker scripts can be added in the script window) demonstrates how to read 
ScriptCommunicator global worker script data.
Note: Woker scripts can exchange data with ScriptCommunicator global variables.
***************************************************************************/

function timeOut()
{
	var string = scriptThread.getGlobalString("String");
	if(string.length > 0)
	{
		scriptThread.appendTextToConsole("string: " + string);
	}
	
	var array = scriptThread.getGlobalDataArray("DataArray");
	if(array.length > 0)
	{
		scriptThread.appendTextToConsole("data array: " + array[0].toString());
	}
	
	var resultArray = scriptThread.getGlobalUnsignedNumber("UnsignedNumber");
	if(resultArray[0] == 1)
	{
		scriptThread.appendTextToConsole("unsigned number: " + resultArray[1]);
	}
	
	resultArray = scriptThread.getGlobalSignedNumber("SignedNumber");
	if(resultArray[0] == 1)
	{
		scriptThread.appendTextToConsole("signed number: " + resultArray[1]);
	}
	
	resultArray = scriptThread.getGlobalRealNumber("RealNumber");
	if(resultArray[0] == 1.0)
	{
		scriptThread.appendTextToConsole("real number: " + resultArray[1]);
	}
	
}

function globalStringChanged(name, string)
{
	scriptThread.appendTextToConsole("globalStringChangedSignal called: name=" + name + "  value=" + string);
}
function globalDataArrayChanged(name, array)
{
	scriptThread.appendTextToConsole("globalDataArrayChanged called: name=" + name + "  value=" + array.toString());
}
function globalUnsignedChanged(name, number)
{
	scriptThread.appendTextToConsole("globalUnsignedChanged called: name=" + name + "  value=" + number);
}
function globalSignedChanged(name, number)
{
	scriptThread.appendTextToConsole("globalSignedChanged called: name=" + name + "  value=" + number);
}
function globalRealChanged(name, number)
{
	scriptThread.appendTextToConsole("globalRealChanged called: name=" + name + "  value=" + number);
}

scriptThread.appendTextToConsole("script data exchange receiver started");
var timer = scriptThread.createTimer();
timer.timeoutSignal.connect(timeOut);
timer.start(1000);

scriptThread.globalStringChangedSignal.connect(globalStringChanged);
scriptThread.globalDataArrayChangedSignal.connect(globalDataArrayChanged);
scriptThread.globalUnsignedChangedSignal.connect(globalUnsignedChanged);
scriptThread.globalSignedChangedSignal.connect(globalSignedChanged);
scriptThread.globalRealChangedSignal.connect(globalRealChanged);

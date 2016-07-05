/*************************************************************************
This worker script (worker scripts can be added in the script window) generates 5 byte frames and adds 
them with addMessageToLogAndConsoles.
This script belongs to the CustomConsole_Table.js example script.
***************************************************************************/

function stopScript() 
{
    scriptThread.appendTextToConsole("Random packet generator stopped ");
}


function timeout() 
{
	var data = "";
	for (var i = 0; i < 5; i++) 
	{
		data += String.fromCharCode(Math.floor((Math.random() * 255)+ 1));
	}
	scriptThread.addMessageToLogAndConsoles(data,true);

}

//start the periodically timer which calls the function timeout
var timer = scriptThread.createTimer()
timer.timeoutSignal.connect(timeout);
scriptThread.appendTextToConsole('Timer started ');
timer.start(1000);





/*************************************************************************
Template for a custom log and a custom console script.
IMPORTANT!
To reload a changed custom console/log script the corresponding checkbox in the settings dialog must
be unchecked and then checked again or the corresponding search button must be pressed.
***************************************************************************/


/*
  * This function is called if:
  * - data has been sent
  * - data has been received
  * - a user message has been entered (from message dialog or normal script (scriptThread.addMessageToLogAndConsoles))
  * Here the string is created which shall be added to the custom console or to the custom log (argument isLog)
  *
  * Note: The custom console (QTextEdit is used) interprets the returned text as HTML (if a new line shall be created,
  * then a <br> must be returned (and not \n)). 
  * Therefore every created console string can have its own format (text color, text size, font family, ...).->see below
  * If no format information is given then the format settings from the settings dialog are used (text color=receive color).
  *
  * The created log strings are directly (without interpreting the content)  written into the custom log file.
  *
  * If the data is from a CAN interface then the bytes have the following meaning:
  * Byte 0= message type (0=standard, 1=standard remote-transfer-request, 2=extended, 3=extended remote-transfer-request)
  * Byte 1-4= can id 
  * Byte 5-12= the data.  
  *
  * @param data
  *   The data.
  * @param timeStamp
  *      The time stamp (the format is set in the settings dialog).
  * @param type
  *   0=the data has been received from a normal interface (all but CAN)
  *   1=the data has been sent with a normal interface (all but CAN)
  *   2=the data has been received from the CAN interface
  *   3=the data has been sent with CAN the can interface
  *   4=the data is a user message (from message dialog or normal script (scriptThread.addMessageToLogAndConsoles))
  * @param isLog
  *   True if this call is for the custom log (false=custom console)
  */
function createString(data, timeStamp, type, isLog)
{
	
	var resultString = "";
	
	//Convert the recieved bytes (unsigned char) to an ascii string (char)
	for(var i = 0; i < data.length; i++)
	{
		resultString += String.fromCharCode(data[i])
	}
	
	if(!isLog)
	{
		//The console is a HTML console therefore following characters must be replaced with their HTML representation.
		resultString = resultString.replace(/</gm, "&lt;")//replace < with &lt
		resultString = resultString.replace(/>/gm, "&gt;")//replace > with &gt
		resultString = resultString.replace(/(\r\n|\n|\r)/gm, "<br>")//replace \r\n, \n and \r with <br>
		resultString = resultString.replace(/ /gm, "&nbsp;")//replace space with &nbsp;
	}
	return resultString;
}
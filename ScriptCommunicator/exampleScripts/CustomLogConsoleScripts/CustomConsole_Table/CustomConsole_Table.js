/*************************************************************************
This script demonstrates how to create tables in a custom console script (can be added 
in settings dialog)
IMPORTANT!
To reload a custom console script the corresponding checkbox in the settings dialog must
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
	storedData = storedData.concat(data);
	
	//After 5 bytes have been received the packet table is created.
	while(storedData.length >=5)
	{
		resultString += '<TABLE ALIGN=CENTER WIDTH="50%" BORDER=1 CELLSPACING=10 CELLPADDING=3><CAPTION><p style="color:blue">Five-byte Packet ('+timeStamp+')</p></CAPTION><TR><TD> <p style="color:green">0b'+storedData[0].toString(2)+'</p></TD> <TD> '+storedData[1]+'</TD> <TD> '+storedData[2]+'</TD><TD> '+storedData[3]+'</TD> <TD> <p style="color:red"> 0x'+storedData[4].toString(16).toUpperCase()+'</p></TD> </TR></TABLE> <BR>';
		storedData.splice(0, 5);
	}

	return resultString;
}

var storedData = Array()
cust.appendTextToConsole("CustomConsole_Table.js started", true, false);
/*************************************************************************
This script demonstrates how to create a logic analyzer with custom console script (can be added 
in settings dialog).
IMPORTANT!
To reload a changed custom console script the corresponding checkbox in the settings dialog must
be unchecked and then checked again or the corresponding search button must be pressed.
***************************************************************************/

//Load the helper script.
cust.loadScript("CustomConsole_Helper.js")

//Set the image folder.
var imageFolder = scriptFile.getScriptFolder() + "/Media"


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
  * If the data is send with a CAN interface then the first bytes are:
  * Byte 0= message type (0=standard, 1=standard remote-transfer-request, 2=extended, 3=extended remote-transfer-request)
  * Byte 1-4 (MSB)= can id 
  * Byte 5-12= the data.  
  *
  * If the data is received with a CAN interface then the first bytes are:
  * Byte 0= message type (0=standard, 1=standard remote-transfer-request, 2=extended, 3=extended remote-transfer-request)
  * Byte 1-4 (MSB)= can id 
  * Byte 5-8 (MSB)=timestamp (difference between the first received CAN message (after the last connect) and the current)
  * Byte 9-16= the data.  
  *
  * If the data is send or received with a I2C interface then the first bytes are:
  * Byte 0= flags bits (1=10 bit address, 2=combined FMT, 4=no stop condition)
  * Byte 1-2 (MSB)= I2C address
  * Byte 3-n= the data.  
  *
  * @param data
  *   The data.
  * @param timeStamp
  *      The time stamp (the format is set in the settings dialog).
  * @param type
  *   0=the data has been received from a normal interface (all but CAN or I2C)
  *   1=the data has been sent with a normal interface (all but CAN or I2C)
  *   2=the data has been received from the CAN interface
  *   3=the data has been sent with a CAN interface
  *   4=the data is a user message (from message dialog or normal script (scriptThread.addMessageToLogAndConsoles))
  *   5=the data has been received from the I2C interface
  *   6=the data has been sent with a I2C interface
  * @param isLog
  *   True if this call is for the custom log (false=custom console)
  */
function createString(data, timeStamp, type, isLog)
{
	
	var resultString = "";
	var HTMLContent = "";
	var lastBit = " ";
	var bitArray = Array();
	
	for(var i = 0; i < data.length; i++)
	{
		//Convert the current byte to a bit array.
		bitArray = numToBinaryArray( data[i] );

		if (i==0) 
		{
			lastBit = bitArray[0]; 
		}

		for(var u = 0; u < bitArray.length; u++)
		{
			if ( bitArray[u] == "0" ) 
			{//The currentbit is 0.
			
			    if (bitArray[u] == lastBit) 
			    {//The current and the last are equal.
				
					HTMLContent += "<img src='" + imageFolder + "/0.gif'>";
			    }
			    else
				{
					HTMLContent += "<img src='" + imageFolder + "/1to0.gif'>";
					lastBit = bitArray[u];
				}
			} 				
			else
			{//The current bit is 1.
			
				if (bitArray[u] == lastBit)
				{//The current and the last are equal.
				
					HTMLContent += "<img src='" + imageFolder + "/1.gif'>";
				}
				else
				{
					HTMLContent += "<img src='" + imageFolder + "/0to1.gif'>";
				    lastBit = bitArray[u];
				}
			} 
		}
		
	}
	
	resultString = HTMLContent + data +"<br>"

	return resultString;
}

cust.appendTextToConsole("CustomConsole_LogicAnalyzer.js started", true, false);
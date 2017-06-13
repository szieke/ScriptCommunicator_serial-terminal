/*************************************************************************
This script demonstrates how to create a ascii custom log/console script.
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

	var resultString = outString;
	outString = "";
	
	//Convert the received bytes (unsigned char) to an ascii string (char)
	for(var i = 0; i < data.length; i++)
	{
		resultString += String.fromCharCode(data[i])
	}
	return resultString;
}

cust.appendTextToConsole("CustomLogConsole_ReadXml.js started", true, false);

var outString  = "";
var xmlReader = cust.createXmlReader();
if(xmlReader.readFile("testRead.xml") == 0)
{

	var itemList = xmlReader.elementsByTagName("ItemList");
	var sequenceItems = itemList[0].childElements();
	var sequences = sequenceItems[0].childElements();

	var attributes = sequences[0].attributes();
	var attributeByName = sequences[0].attributeValue("format");

	var text = sequenceItems[0].childTextElements();
	var cdata = sequenceItems[0].childCDataElements();
	var comment = sequenceItems[0].childCommentElements();

	var root = xmlReader.getRootElement();

	outString +="<br>sequenceItems: " + sequenceItems.length;
	outString +="<br>first attribute, name: " + attributes[0].name() + "  value: " + attributes[0].value();
	outString +="<br>sequence attribute format: " + attributeByName;
	outString +="<br>text2: " + text[1];
	outString +="<br>cdata2: " + cdata[1];
	outString +="<br>comment2: " + comment[1];
	outString +="<br>root: " + root.elementName();
	outString +="<br>";
	
}
else
{
	outString = "<br><br><br>readFile failed<br><br><br>";
}

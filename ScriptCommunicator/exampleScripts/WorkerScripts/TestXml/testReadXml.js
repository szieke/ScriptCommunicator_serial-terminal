/********************************************************************************************************
* The script demonstrates the usage of the ScriptCommunicator XML API.
**********************************************************************************************/

//Is called if this script shall be exited.
function stopScript() 
{
    scriptThread.appendTextToConsole("script has been stopped");
}

scriptThread.appendTextToConsole('script has started');

var xmlReader = scriptThread.createXmlReader();
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

	scriptThread.appendTextToConsole("sequenceItems: " + sequenceItems.length);
	scriptThread.appendTextToConsole("first attribute, name: " + attributes[0].name() + "  value: " + attributes[0].value());
	scriptThread.appendTextToConsole("sequence attribute format: " + attributeByName);
	scriptThread.appendTextToConsole("text2: " + text[1]);
	scriptThread.appendTextToConsole("cdata2: " + cdata[1]);
	scriptThread.appendTextToConsole("comment2: " + comment[1]);
	scriptThread.appendTextToConsole("root: " + root.elementName());
	
}
else
{
	scriptThread.messageBox("Critical", "Error", "readFile failed");
}

scriptThread.stopScript();
